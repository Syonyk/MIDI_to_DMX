#ifndef __FIXED_CHANNELS_H__
#define __FIXED_CHANNELS_H__

/**
 * Fixed channels.  If certain DMX channels need to be set to certain
 * values regardless of the lighting scenes, do it here.  This would be things
 * like setting the mode on RGB LED cans, or setting brightness masters on
 * certain lights (if you don't wish to change them).
 */

typedef struct {
  byte channel; // DMX channel (1-127)
  byte value; // Value (0-255))
} fixed_channel;

/**
 * Dynamically sized array of fixed channels.  To add a new line, simply copy an
 * existing line, append it to the end, and change the values.  The first value
 * is the DMX channel, and the second value is the value for that channel.
 */
const PROGMEM fixed_channel fixed_channels[] = {
  //{32, 255}, // Channel 32 - RGB LED mode // TEST TEST TEST
  //{33, 255}, // Channel 33 - RGB brightness // TEST TEST TEST
  {64, 255}, // Channel 64 - RGB LED mode
  {65, 255}, // Channel 65 - RGB brightness
};

#define NUMBER_OF_FIXED_CHANNELS (sizeof(fixed_channels) /                     \
sizeof(fixed_channel))

#endif // __FIXED_CHANNELS_H__