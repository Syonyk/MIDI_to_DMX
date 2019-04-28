#ifndef __DEFINES_H__
#define __DEFINES_H__

/**
 * Assorted defines for the Midi to DMX project.
 * 
 * Feel free to add more if you want them!
 */

// Color declarations for RGB fixtures.
// All values in R, G, B order.
#define COLOR_OFF 0, 0, 0
#define COLOR_RED 255, 0, 0
#define COLOR_ORANGE 255, 128, 0
#define COLOR_YELLOW 255, 255, 0
#define COLOR_LIME 128, 255, 0
#define COLOR_GREEN 0, 255, 0
#define COLOR_CYAN 0, 255, 255
#define COLOR_LT_BLUE 0, 128, 255
#define COLOR_BLUE 0, 0, 255
#define COLOR_PURPLE 128, 0, 255
#define COLOR_MAGENTA 255, 0, 255
#define COLOR_WHITE 255, 255, 255
// TODO: Add more of these.



/**
 * Brightness values for a single fixture.  These are intended for spotlights,
 * overhead lights, and other single-channel fixtures.
 */
#define BRIGHTNESS_OFF 0
#define BRIGHTNESS_LOW 64
#define BRIGHTNESS_MED 128
#define BRIGHTNESS_HIGH 192
#define BRIGHTNESS_FULL 255

/**
 * MIDI command masks (to get the command out of the first byte) and what the
 * corresponding byte values for each command type will be.
 */
#define MIDI_COMMAND_MASK 0x70
#define MIDI_NOTE_OFF 0x00
#define MIDI_NOTE_ON 0x10
#define MIDI_CONTROL_CHANGE 0x30

// Set these in the valid bit - need to be non-zero.
#define COMMAND_SCENE 0x1
#define COMMAND_FIXTURE 0x2
#define COMMAND_CHANNEL 0x3

#define MS_PER_SECOND 1000

/*
 * Dim a given value to a given percent (0-100).
 * 
 * This is only valid on DMX 0-255 values with a uint16_t as the type.
 */

#define DIM(value, percent) (((uint16_t)value * percent) / 100)

/**
 * 4-byte MIDI command structure - the three data bytes are broken apart into
 * the desired command, and the data values.
 */
typedef struct {
  byte command; // Zero, or the command code.
  byte channel;
  byte data0; // Note or Controller
  byte data1; // Velocity or Value
} midi_command;

#endif // __DEFINES_H__
