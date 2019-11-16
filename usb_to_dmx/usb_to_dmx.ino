// If this is defined, will build as USB MIDI for the 32U4 based units.
// Else, will build as serial MIDI (for prototype units)
#define USE_USB_MIDI

// Define this to print out state to serial.  Useful for debugging.
// Leave this off in production.  This ONLY WORKS ON THE PRO MINI.
//#define PRINT_STATE

#include <EEPROM.h>

// REQUIRES LIBRARY:
// https://github.com/TinkerKit/DmxMaster
#include <DmxMaster.h>

#ifdef USE_USB_MIDI
// REQUIRES LIBRARY:
// https://github.com/arduino-libraries/MIDIUSB
#include <MIDIUSB.h>
#endif

#include "defines.h"
#include "fixed_channels.h"
#include "scenes.h"
#include "fixtures.h"

// DmxMaster doesn't support more channels on "small memory" devices.
// If you have more than 128 active channels, you probably need a lightboard.
#define MAX_DMX_CHANNELS 128

// Midi can send roughly 1 command per millisecond.  Wait up to this time after
// sending a command for more commands before resuming main flow.
// Generally, multiple commands per ms come in.
#define COMMAND_CHAIN_DELAY_MS 3

/**
 * Extern memory buffer defined in DmxMaster.cpp.  To avoid any blips in the
 * lighting output on a restart, the stored values are put into this buffer
 * directly before starting the DMX output signal.  Lights will hold onto the
 * old value if they do not receive an update, but DmxMaster initializes the
 * buffer to all 0s, so updating with the normal writes, while the output is
 * alive, could cause a visible glitch in the lighting.  By writing the values
 * here, the lighting will always stay where it was before (unless power is
 * interrupted in the middle of a fade - then the old scene will load).
 * 
 * DMX_SIZE is defined in DmxMaster.h
 */
extern volatile uint8_t dmxBuffer[DMX_SIZE];

/**
 * For fades, the fader needs the start state, the end state, and the progress
 * through the fade.
 * 
 * fade_start_values is the start state, and will be updated to match the target
 * of the fade at the conclusion of the fade.
 * fade_target_values is the desired end state of the fade.
 * 
 * The actual state only lives in the DmxMaster class - no need to duplicate it.
 * 
 * fade_start_millis and fade_end_millis are the values of millis() at the start
 * and target end of the fade.  The fade will be calculated based on these.  If
 * a fade is not in progress, reset these both to 0.
 * 
 * After a fade, the current values are written to the EEPROM to be restored on
 * power-on.
 */
byte fade_start_values[MAX_DMX_CHANNELS] = {0};
byte fade_target_values[MAX_DMX_CHANNELS] = {0};
byte fade_current_values[MAX_DMX_CHANNELS] = {0};
uint32_t fade_start_millis = 0, fade_end_millis = 0;

// True if the scene is stored to EEPROM.
bool scene_is_stored_to_eeprom = 0;

#ifdef PRINT_STATE
uint32_t last_print_time = 0;
#endif

/**
 * One problem encountered during development: Power blips.  If the presentation
 * machine USB bus is reset, the converter resets, which means that the start
 * scene (originally all black) is set.  This is no good, since it means someone
 * messing with a USB mic will black out the whole auditorium.  However, if the
 * program code is changed, and new scenes are added, it is reasonable to reset
 * the DMX channels to some safe default values (all black, except for the fixed
 * channels).
 * 
 * Identifying when the code has changed on Arduino is a bit tricky (there are
 * no build numbers), but a quick hash of the compile date/time stamp is good
 * enough for most uses (theoretically, once every 256 builds, there will be a
 * collision, but this is unlikely enough for common uses).
 * 
 * The compile date/time are hashed into a byte that is used as a magic at the
 * start of the EEPROM (DMX channel 0 is not a valid channel, so the magic goes
 * into this channel, effectively).  If the magic matches, then the stored
 * channel data is restored, as this was a power cycle.  If the magic does not
 * match, then this must be a new build, and the defaults are used.  It's not
 * the cleanest approach, but it's good enough for casual use.  One could
 * increase the size of the hash if one wanted better collision resistance.
 */

// Static value based on compile time.  As the headers are included in this
// file, any change to the headers will rebuild this file.
// Stored in PROGMEM, as is good and proper for constants.
const char PROGMEM compile_date[] = __DATE__ " " __TIME__;

/**
 * Basic hash function that chews the above compile_date string into a one byte
 * hash.  Read out of program memory, xor the byte in, move on.  This is in NO
 * way cryptographically secure or anything else, it's just unlikely to be the
 * same value for two builds, with a useful enough probability.
 * 
 * @return A byte that is a basic hash of the compile date.
 */
byte get_build_hash() {
  byte xor_stamp = 0;

  for (byte i = 0; i < strlen_P(compile_date); i++) {
    byte val = pgm_read_byte_near(compile_date + i);
    xor_stamp ^= val;
  }

  return xor_stamp;
}

/**
 * Print the state, if enabled.  This is useful for debugging without a full
 * set of lights.
 */
#ifdef PRINT_STATE
void print_state() {
  Serial.println(F("====Current Channel State===="));
  for (uint8_t i = 0; i < MAX_UNIQUE_CHANNELS; i++) {
    Serial.print(F("["));
    Serial.print(pgm_read_byte_near(&scene_slot_to_channel_mapping[i]));
    Serial.print(F("]: "));
    Serial.println(dmxBuffer[pgm_read_byte_near(&scene_slot_to_channel_mapping[i]) - 1]);
  }
}
#endif

/**
 * On power on, attempt to restore the settings from EEPROM.  This only happens
 * if the compile magic matches.  This will write the current state array, the
 * future state field, and the DMX array (extern hack).  Call this BEFORE
 * calling any of the DMX functions that will start sending out the data to the
 * lights.
 * 
 * Byte 0 of the EEPROM stores the build magic, and bytes 1-127 store the DMX
 * channel values at the end of each fade.
 * 
 * If the data does not match, this zeroes the EEPROM state and returns false.
 * 
 * @return True if the data was restored, false if the magic does not match.
 */
bool restore_from_eeprom() {
  byte build_hash = get_build_hash();
  
  if (build_hash == EEPROM[0]) {
    // Build hash matches - restore the state to all the various fields.
    for (byte i = 1; i < MAX_DMX_CHANNELS; i++) {
      byte val = EEPROM[i];
      fade_start_values[i] = val;
      fade_target_values[i] = val;
      dmxBuffer[i] = val;
    }
  } else {
    // Hashes do not match.  Store the new build hash and zero all channels.
    EEPROM[0] = build_hash;
    for (byte i = 1; i < MAX_DMX_CHANNELS; i++) {
      EEPROM[i] = 0;
      fade_start_values[i] = 0;
      fade_target_values[i] = 0;
      dmxBuffer[i] = 0;
    }
    
    // Ensure this gets written out when the fader next runs.
    fade_start_millis = fade_end_millis = millis();
  }
}

/**
 * The only substantial difference between the USB MIDI endpoint code and the
 * serial MIDI code is this function.  This reads the MIDI command and packs it
 * into a midi_command structure for return and use.
 */
#ifdef USE_USB_MIDI
/**
 * The USB MIDI code is a bit simpler, as a full command will come as a block
 * from the PluggableUSB library.  We receive a midiEventPacket_t with the full
 * command, and can simply parse this - there's no possibility of a midstream
 * serial sync issue, and the command is either read or not.
 */
midi_command get_midi_command() {
  midi_command ret = {0, 0, 0, 0};
  byte command;
  midiEventPacket_t rx;

  // Read a packet from the USB interface.  If there is no packet to be read,
  // all the values of the return structure are zeroed.
  rx = MidiUSB.read();
  
  // If there is no MIDI data, there can be no MIDI command.
  if (!rx.header) {
    return ret;
  }
 
  // A MIDI command code has the hight bit set, 3 bits of command code, and the
  // lower 4 are the channel.  Mask this out to see what the command is.
  command = rx.byte1 & MIDI_COMMAND_MASK;
  
  if (command == MIDI_NOTE_ON) {
    ret.command = COMMAND_SCENE;
  } else if (command == MIDI_NOTE_OFF) {
    ret.command = COMMAND_FIXTURE;
  } else if (command == MIDI_CONTROL_CHANGE) {
    ret.command = COMMAND_CHANNEL;
  } else {
    // Not a valid command, leave command as null to indicate nothing.
    // However, read out the data bytes and the command value.
  }
  
  // MIDI channels are 1-16, not 0-15.  However, everything is 0 indexed in C.
  ret.channel = (rx.byte1 & 0xf);
  
  // Copy the two data bytes: Note and Velocity.
  ret.data0 = rx.byte2;
  ret.data1 = rx.byte3;

  return ret;
}

#else
/**
 * The serial MIDI reader is a bit more complex, as it has to handle the
 * possibility of mid-stream sync and has to wait for the data byes to be ready
 * to read.
 */
midi_command get_midi_command() {
  midi_command ret = {0, 0, 0, 0};
  byte val, command, data;
  
  // If there is no serial data, there can be no MIDI command - return the null
  // struct (valid not set).
  if (!Serial.available()) {
    return ret;
  }

  // Wait for a command - this is a byte with the high bit set.  If this is NOT
  // set, we're mid-stream - just return and try again next time.
  val = Serial.read();
  if (!(val & 0x80)) {
    return ret;
  }
  
  // Determine if this is a command of interest.  Mask off the upper bit and the
  // lower 8, then compare with the valid command values.
  command = val & MIDI_COMMAND_MASK;
  
  if (command == MIDI_NOTE_ON) {
    ret.command = COMMAND_SCENE;
  } else if (command == MIDI_NOTE_OFF) {
    ret.command = COMMAND_FIXTURE;
  } else if (command == MIDI_CONTROL_CHANGE) {
    ret.command = COMMAND_CHANNEL;
  } else {
    // Not a valid command, leave command as null to indicate nothing.
    // However, read out the data bytes and the command value.
  }
  
  // MIDI channels are 1-16, not 0-15.  However, everything is 0 indexed in C.
  ret.channel = (val & 0xf);
  
  // Read the two data bytes: Note and Velocity.
  while (!Serial.available());
  ret.data0 = Serial.read(); // Note
  while (!Serial.available());
  ret.data1 = Serial.read(); // Velocity

  return ret;
}
#endif

/**
 * Data bytes in MIDI can only be 0-127.  However, DMX brightness values are
 * 0-255.  This function scales the 0-127 value to a full range 0-255 value.
 * 
 * Really, it doubles it and adds 1 if it's 127.
 */
uint8_t scale_brightness(const uint8_t brightness) {
  if (brightness == 127) {
    return 255;
  }
  return brightness * 2;
}

/**
 * Set the fade parameters.  This notes the start time and desired end time, and
 * non-zero values in these indicate that a fade needs to run.  If both the
 * start and end values are identical, the new value is set without any fade.
 * 
 * If a fade is already running, this will copy the current fade values to the
 * start array, and extend the fade.  This should work reasonably well for most
 * use cases.
 * 
 * This also marks that the scene needs to be stored to EEPROM after completion
 * for restoration on powerup.
 */
void set_fade(const uint8_t fade_seconds) {
  // This needs to be reset for the new fade.
  fade_start_millis = millis();  
  
  // Case: Fade running.
  if (fade_end_millis && fade_start_millis) {
    // Copy current state to the start.
    for (int i = 0; i < MAX_DMX_CHANNELS; i++) {
      fade_start_values[i] = fade_current_values[i];
    }

    // Just stretch the fade.
    fade_end_millis = fade_end_millis + (fade_seconds * MS_PER_SECOND);
  } 
  // Case: No fade running, start a new one.
  else {
    fade_end_millis = fade_start_millis + (fade_seconds * MS_PER_SECOND);
  }
  
  scene_is_stored_to_eeprom = 0;
}

/**
 * Set a new scene with a fade time.  This pulls a scene out of scene.h, sets it
 * in the fade_target_values array, sets the fade timers properly, and then
 * returns to let the fader run through things.  It will write all values in the
 * scene to the target_values array, as the fader only updates values that don't
 * match already for performance reasons.
 * 
 * However, it now won't call for a fade if the same scene is called for.
 * 
 * @param scene The scene index from scene.h
 */
void set_scene_with_fade_time(const byte scene, const byte fade_seconds) {
  byte channel, value;
  byte fade_required = false;
  
  // Don't read invalid scenes in.
  if (scene >= MAX_SCENE_COUNT) return;
  
  for (byte i = 0; i < MAX_UNIQUE_CHANNELS; i++) {
    channel = pgm_read_byte_near(&scene_slot_to_channel_mapping[i]);
    value = pgm_read_byte_near(&scenes[scene][i]);
    if (value != fade_target_values[channel]) {
      fade_required = true;
    }
    fade_target_values[channel] = value;
  }

  // If anything has changed in the targets, run the fade.
  if (fade_required) {
    set_fade(fade_seconds);
  }
}

/**
 * Set a fixture to a new value.  The fade time may be reset by subsequent calls
 * but they should be close enough together to avoid any visual artifacts with
 * the transitions.  Hopefully.
 * 
 * @param fixture The fixture (0-15) to use - sent in as the channel value.
 * @param color_brightness The color value to use, as defined in fixtures.h - 
 *   sent as the note.  For a white fixture, this sets the brightness directly
 *   with a reasonable scale factor.
 * @param fade_seconds The fade time in seconds - sent in as velocity.
 */
void set_fixture_with_fade_time(const byte fixture, const byte color_brightness, 
        const byte fade_seconds) {
  byte channel, value;
  fixture_data fixture_values;
  
  // Shouldn't happen, but there's no data up this high...
  if (fixture >= MAX_FIXTURE_COUNT) return;

  // Copy the fixture values from program memory into SRAM for use.
  memcpy_P(&fixture_values, &fixtures[fixture], sizeof(fixture_data));

  if (fixture_values.fixture_type == FIXTURE_UNUSED) return;
  
  // Valid fixture - set the values.
  if (fixture_values.fixture_type == FIXTURE_RGB) {
    // RGB fixture - set the RGB channels to the desired color.
    if (color_brightness >= MAX_FIXTURE_COLOR) return;
    
    // Read the color value out of program memory and set it.
    for (channel = 0; channel < 3; channel++) {
      value = pgm_read_byte_near(&colors[color_brightness][channel]);
      fade_target_values[fixture_values.fixture_base_address + channel] = value;
    }
  } else if (fixture_values.fixture_type == FIXTURE_WHITE) {
    // This is a white light - simply set the brightness.
    fade_target_values[fixture_values.fixture_base_address] = 
            scale_brightness(color_brightness);
  }
  
  // For both types, request a fade of the desired length.
  set_fade(fade_seconds);
}

/**
 * At the end of a fade, store the updated scene values.  This writes out the
 * current values to EEPROM for restoration on the next powerup, but it only
 * writes the values that have actually changed.  EEPROM writes are slow (about
 * 3.5ms per write), so writing all 128 values takes half a second, for no good
 * reason except wearing out the EEPROM.  It is well worth the read access to
 * skip unneeded writes.
 */
void store_current_to_eeprom() {
  if (!scene_is_stored_to_eeprom) {
    for (byte i = 1; i < MAX_DMX_CHANNELS; i++) {
      if (fade_start_values[i] != EEPROM[i]) {
        EEPROM[i] = fade_start_values[i];
      }
    }
    scene_is_stored_to_eeprom = 1;
  }
}

/**
 * Fun with faders...
 * 
 * If the fade_start_millis/fade_end_millis values are not zero, this means a
 * fade is in progress.  This function will, without using floating point math,
 * calculate the value for all the channels that differ between the start and
 * end values, and write the fade values.  Once the fade is over, this will
 * update the start scene with the current scene, null out the fader values, and
 * then write the scene to the EEPROM for restoration on powerup.
 * 
 * There is simply no good reason to use floating point math for an Arduino
 * sketch that is writing integer values out, and adding the floating point
 * emulation libraries adds an awful lot of code bulk (a few kb).
 */
void run_fader() {
  // If neither value is set, there's no fade in progress.  Return.
  if (!(fade_start_millis || fade_end_millis)) {
    return;
  }
  
  // If the fade is done, set all values to their target values, terminate the
  // fade, and write the state to eeprom.  This also handles zero second fades
  // where start and end time are the same (it will simply write out all the
  // new values as this will happen after the zero second fade request).
  if (millis() >= fade_end_millis) {
    
    // Write out the full target value and copy it into the start array.
    for (byte i = 0; i < MAX_DMX_CHANNELS; i++) {
      DmxMaster.write(i, fade_target_values[i]);
      fade_start_values[i] = fade_target_values[i];
    }
    
    // With the fade completed, indicate that no fade is in progress.
    fade_start_millis = 0;
    fade_end_millis = 0;

    store_current_to_eeprom();

    return;
  }
  
  // Fade is needed!  Calculate how far through the fade we are in millis, and
  // do the correct math to determine the correct value for each DMX channel.
  uint32_t total_fade_time = fade_end_millis - fade_start_millis;
  
  // This /should/ be wraparound safe...
  uint32_t fade_time_elapsed = millis() - fade_start_millis;

  // Percent through the fade ranges from 0-256 (scaled).
  uint32_t fade_percent = (256 * fade_time_elapsed) / total_fade_time;

  for (byte i = 0; i < MAX_DMX_CHANNELS; i++) {
    byte old_value, new_value, value;
    int32_t temp; // INT32 - not UINT.  This needs to handle negative values!
    old_value = fade_start_values[i];
    new_value = fade_target_values[i];

    // No point in doing expensive math on things that aren't changing...
    if (old_value == new_value) {
      fade_current_values[i] = new_value;
      continue;
    }
    
    // Calculate the offset (positive or negative) between old and new.
    temp = (int32_t)new_value - (int32_t)old_value;
    
    // Convert the percent through the fade into the offset from the old value.
    temp *= fade_percent;
    temp /= 256;
    
    // Apply the offset to the old value to get the midpoint channel value.
    value = old_value + temp;

    // Store the current value to the in-process structure in case the fade
    // switches mid-fade.
    fade_current_values[i] = value;
    
    DmxMaster.write(i, value);
  }
}

void setup() {
  // Avoid the "double start" issues on older boards.
  // Probably not needed in production.
  delay(2000);
  
  // Hardware serial is used for the MIDI side - set MIDI baud rate 31.25kHz
#ifndef USE_USB_MIDI
  Serial.begin(31250);
#endif

#ifdef PRINT_STATE
  Serial.begin(115200);
#endif
  
  // Check to see if we are restoring from old state, or creating new state.
  if (!restore_from_eeprom()) {
    // State does not match, all channels set to 0.  We need to set the fixed
    // channel values.
    // Directly write to the DMX Master array so these are in place before the
    // DMX signals start going out (it shouldn't matter, but why not?)
    for (int i = 0; i < NUMBER_OF_FIXED_CHANNELS; i++) {
      fixed_channel fixed;
      
      memcpy_P(&fixed, &fixed_channels[i], sizeof(fixed_channel));
      
      fade_start_values[fixed.channel] = fixed.value;
      fade_target_values[fixed.channel] = fixed.value;
      dmxBuffer[fixed.channel] = fixed.value;
    }
  }

  // All the buffers hold the proper values now.  Start the DMX output.
  // Any of the DmxMaster commands will start output, and this code supports
  // up to 128 channels, so transmit all of them.
  DmxMaster.maxChannel(128);
}

/**
 * The main command loop simply looks for a command, executes it, and calls the
 * fader (which will run as fast as possible) to set the actual light values.
 * 
 * The DMX signal is emitted by the DmxMaster library using interrupts and
 * timers and is independent from this code (but does take a good chunk of the
 * CPU time).
 * 
 * The code supports a basic concept of "command chaining" - if a command has
 * been sent, it will wait a few milliseconds for another command before
 * executing the fader.  Proclaim will send commands at roughly one per
 * millisecond (or a bit faster), so waiting for a few ms lets the multiple
 * command sequences all get applied before the fader begins running.
 * 
 * Scene mode and fixture mode call the proper function for the update, but the
 * raw DMX channel mode simply sets the values.
 */
void loop() {
  midi_command command;
  uint32_t start = millis();

  // Turn off the blinding red LEDs on the Pro Micro platform.
#ifdef USE_USB_MIDI
  TXLED1;
  RXLED1;
#endif
  
  // Read and process any updates.  Wait for the timeout to let commands chain.
  while ((millis() - start) < COMMAND_CHAIN_DELAY_MS) {
    command = get_midi_command();
    if (command.command) {
      // Reset the timeout if this is a valid command.
      start = millis();
      
      // Data 0 (Note) is the scene ID, Data 1 (Velocity) is the fade time.
      if (command.command == COMMAND_SCENE) {
        set_scene_with_fade_time(command.data0, command.data1);
        #ifdef PRINT_STATE
        Serial.print(F("Setting scene: "));
        Serial.println(command.data0);
        #endif
      } 
      
      // Channel is the fixture #, data0 (note) is color or brightness, and
      // data1 (velocity) is the fade time.  The last fade time sent is the one
      // that will drive the fade.
      else if (command.command == COMMAND_FIXTURE) {
        set_fixture_with_fade_time(command.channel, command.data0,
                command.data1);
      } 
      
      // DMX channel mode takes "Number" as the channel and "Value" as the
      // brightness for that channel (scaled).  To actually set the new values
      // into motion, send a message with "Number" set to 0 and "Value" set to
      // the desired fade time.
      else if (command.command == COMMAND_CHANNEL) {
        if (command.data0 == 0) {
          set_fade(command.data1);
        } else {
          fade_target_values[command.data0] = scale_brightness(command.data1);
        }
      }
    }
  }

  // Run the fader to update values as needed.
  run_fader();

  // Print state every ~8s if needed.
#ifdef PRINT_STATE
  if ((millis() >> 13) > last_print_time) {
    print_state();
    last_print_time = millis() >> 13;
  }
#endif
}
