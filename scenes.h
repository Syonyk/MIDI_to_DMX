/**
 * Scene Definitions.  This mode operates with the MIDI Note On command, and
 * specifies a full scene for each "Note" value sent.  The programming is,
 * sadly, a bit complex, due to limitations in the C++ standard the Arduino
 * environment uses.
 * 
 * The maximum scene number is 127, and it's wise to leave Scene 0 as a full
 * black scene (not required, but it's a good convention).
 * 
 * Programming is described in each section, but if this makes no sense at all,
 * it's probably better to use the fixture mode or the raw DMX mode to program
 * your lights.  You can contact the author for assistance if needed, though
 * free assistance is somewhat limited due to lack of free time.
 * 
 * If you'd like to define your own scenes, you'll need to start by laying out
 * all the DMX channels you use that need to be set (you can ignore channels set
 * in the fixed channel header as they will never change).  This example list
 * will be referenced in several places.
 * 
 * For the example, the following channels/lights are in use:
 * 4: White stage spotlights
 * 6: White congregation overhead lights
 * 66, 67, 68: RGB "side wash" lights
 * 1, 2, 3: RGB "stage edge" lights
 * 8, 9, 10: RGB "stage center" lights
 */

// Set this to however many scenes you have defined below.
#define MAX_SCENE_COUNT 128

// This must be set to the number of unique channels being controlled.
// In the example/my setup, this is 11 channels.
#define MAX_UNIQUE_CHANNELS 11

/**
 * The definitions here are the tricky part, and are tricky mostly because of
 * the C++ standards for sparse array are worse than in C, especially GNU C.
 * 
 * What's happening is that the scene array is being initialized with a series
 * of values in sequential order, but the illusion is given of specifying
 * brightness or color values for each fixture.  A set of defines and a
 * conversion array accomplish this slightly easier to read format.  But,
 * realistically, someone who knows C is probably going to have to set this up
 * the first time.
 * 
 * While going through here, remember that the colors are defined as a comma
 * separated list of the three RGB values:
 * #define COLOR_MAGENTA 255, 0, 255
 * 
 * And the brightness defines are a single value:
 * #define BRIGHTNESS_HIGH 192
 * 
 * The scene_slot_to_channel_mapping translates from the position of the value
 * in each scene to the DMX channel controlled by that position - so, for my
 * example, position 0 (stage spots) is actually controlling DMX channel 4.
 * 
 * For single channel fixtures, you only need one entry.  For RGB fixtures, you
 * need three (one for each channel).  The defines are in R, G, B order, so if
 * you have some bizarre fixture that doesn't have them aligned the same way,
 * this allows you to twiddle the values around and line things up so they work.
 * 
 * Once each of these entries is defined, create a define (below) to indicate
 * the position that each field starts in.  This allows you to define scene
 * arrays that are reasonably readable.
 * 
 * What actually happens, if you take the example scene 1, is this:
 * [1] = {
 *   [0] = 64,
 *   [1] = 255,
 *   [2] = 255, 128, 0,
 *   [5] = 0, 0, 255,
 *   [8] = 0, 0, 255,
 * },
 * 
 * Since there are no gaps, the compiler is happy.  If there is a gap, you get
 * an error:
 * sorry, unimplemented: non-trivial designated initializers not supported
 * 
 * That means that you need to go through and check your mappings again, because
 * there's a gap somewhere.
 * 
 * Alternately, if you wanted, you could just define scene 1 like this:
 * 
 * [1] = {64, 255, 255, 128, 0, 0, 0, 255, 0, 0, 255},
 * 
 * Or, like this:
 * [1] = {
 *   BRIGHTNESS_LOW,
 *   BRIGHTNESS_FULL,
 *   COLOR_BLUE,
 *   COLOR_ORANGE,
 *   COLOR_ORANGE
 * },
 * 
 * It's up to you - I feel my method is the most readable, but does require a
 * bit more upfront work.
 * 
 * Regardless of what method you use, go about setting up your scenes as
 * desired, and things should work!
 */

const PROGMEM byte scene_slot_to_channel_mapping[MAX_UNIQUE_CHANNELS] = {
  4,          // STAGE_SPOTS
  6,          // AUDIENCE_LIGHTS
  66, 67, 68, // WASH_LIGHTS
  1, 2, 3,    // STAGE_EDGE_BARS
  8, 9, 10,   // STAGE_CENTER_BARS
};

// White: Single channel
#define STAGE_SPOTS 0

// White: Single channel
#define AUDIENCE_LIGHTS 1

// RGB: 3 channels - slots 2, 3, 4
#define WASH_LIGHTS 2

// RGB: 3 channels, slots 5, 6, 7
#define STAGE_EDGE_BARS 5

// RGB: 3 channels, slots 8, 9, 10
#define STAGE_CENTER_BARS 8

/**
 * Involve Church setup:
 * Scene 0 is always black.
 * Scenes 1-16 are preservice scenes.
 * Scenes 17-32 are music scenes.
 * Scenes 33-48 are sermon scenes.
 * 
 * For no particular reason, the following color combos are used (wash/stage):
 * 1: Orange/Blue
 * 2: Blue/Orange
 * 3: Blue/Green
 * 4: Red/Blue
 * 5: White/Green
 * 6: Magenta/Cyan
 * 7: Yellow/Green
 * 8: Cyan/Red
 * 9: Green/Blue
 * 10: Green/Red
 * 11: Red/Purple
 * 12: Purple/Red
 * 13: White/Red
 * 14: Green/Purple
 * 15: Blue/Blue
 * 16: Red/Red
 * 
 * Sermon scenes turn off the stage lights, so only have one for each color.
 */


// Scenes!  Program at will.
const PROGMEM byte scenes[MAX_SCENE_COUNT][MAX_UNIQUE_CHANNELS] = {
  // Scene 0: Lights off.  Scene 0 is always everything off.
  [0] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 1: Preservice 1
  [1] = {
    [STAGE_SPOTS] = BRIGHTNESS_LOW,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_FULL,
    [WASH_LIGHTS] = COLOR_ORANGE,
    [STAGE_EDGE_BARS] = COLOR_BLUE,
    [STAGE_CENTER_BARS] = COLOR_BLUE,
  },
  // Scene 2: Preservice 2
  [2] = {
    [STAGE_SPOTS] = BRIGHTNESS_LOW,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_FULL,
    [WASH_LIGHTS] = COLOR_BLUE,
    [STAGE_EDGE_BARS] = COLOR_ORANGE,
    [STAGE_CENTER_BARS] = COLOR_ORANGE,
  },
  // Scene 3: Preservice 3
  [3] = {
    [STAGE_SPOTS] = BRIGHTNESS_LOW,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_FULL,
    [WASH_LIGHTS] = COLOR_BLUE,
    [STAGE_EDGE_BARS] = COLOR_GREEN,
    [STAGE_CENTER_BARS] = COLOR_GREEN,
  },
  // Scene 4: Preservice 4
  [4] = {
    [STAGE_SPOTS] = BRIGHTNESS_LOW,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_FULL,
    [WASH_LIGHTS] = COLOR_RED,
    [STAGE_EDGE_BARS] = COLOR_BLUE,
    [STAGE_CENTER_BARS] = COLOR_BLUE,
  },
  // Scene 5: Preservice 5
  [5] = {
    [STAGE_SPOTS] = BRIGHTNESS_LOW,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_FULL,
    [WASH_LIGHTS] = COLOR_WHITE,
    [STAGE_EDGE_BARS] = COLOR_GREEN,
    [STAGE_CENTER_BARS] = COLOR_GREEN,
  },
  // Scene 6: Preservice 6
  [6] = {
    [STAGE_SPOTS] = BRIGHTNESS_LOW,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_FULL,
    [WASH_LIGHTS] = COLOR_MAGENTA,
    [STAGE_EDGE_BARS] = COLOR_CYAN,
    [STAGE_CENTER_BARS] = COLOR_CYAN,
  },
  // Scene 7: Preservice 7
  [7] = {
    [STAGE_SPOTS] = BRIGHTNESS_LOW,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_FULL,
    [WASH_LIGHTS] = COLOR_YELLOW,
    [STAGE_EDGE_BARS] = COLOR_GREEN,
    [STAGE_CENTER_BARS] = COLOR_GREEN,
  },
  // Scene 8: Preservice 8
  [8] = {
    [STAGE_SPOTS] = BRIGHTNESS_LOW,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_FULL,
    [WASH_LIGHTS] = COLOR_CYAN,
    [STAGE_EDGE_BARS] = COLOR_RED,
    [STAGE_CENTER_BARS] = COLOR_RED,
  },
  // Scene 9: Preservice 9
  [9] = {
    [STAGE_SPOTS] = BRIGHTNESS_LOW,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_FULL,
    [WASH_LIGHTS] = COLOR_GREEN,
    [STAGE_EDGE_BARS] = COLOR_BLUE,
    [STAGE_CENTER_BARS] = COLOR_BLUE,
  },
  // Scene 10: Preservice 10
  [10] = {
    [STAGE_SPOTS] = BRIGHTNESS_LOW,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_FULL,
    [WASH_LIGHTS] = COLOR_GREEN,
    [STAGE_EDGE_BARS] = COLOR_RED,
    [STAGE_CENTER_BARS] = COLOR_RED,
  },
  // Scene 11: Preservice 11
  [11] = {
    [STAGE_SPOTS] = BRIGHTNESS_LOW,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_FULL,
    [WASH_LIGHTS] = COLOR_RED,
    [STAGE_EDGE_BARS] = COLOR_PURPLE,
    [STAGE_CENTER_BARS] = COLOR_PURPLE,
  },
  // Scene 12: Preservice 12
  [12] = {
    [STAGE_SPOTS] = BRIGHTNESS_LOW,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_FULL,
    [WASH_LIGHTS] = COLOR_PURPLE,
    [STAGE_EDGE_BARS] = COLOR_RED,
    [STAGE_CENTER_BARS] = COLOR_RED,
  },
  // Scene 13: Preservice 13
  [13] = {
    [STAGE_SPOTS] = BRIGHTNESS_LOW,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_FULL,
    [WASH_LIGHTS] = COLOR_WHITE,
    [STAGE_EDGE_BARS] = COLOR_RED,
    [STAGE_CENTER_BARS] = COLOR_RED,
  },
  // Scene 14: Preservice 14
  [14] = {
    [STAGE_SPOTS] = BRIGHTNESS_LOW,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_FULL,
    [WASH_LIGHTS] = COLOR_GREEN,
    [STAGE_EDGE_BARS] = COLOR_PURPLE,
    [STAGE_CENTER_BARS] = COLOR_PURPLE,
  },
  // Scene 15: Preservice 15
  [15] = {
    [STAGE_SPOTS] = BRIGHTNESS_LOW,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_FULL,
    [WASH_LIGHTS] = COLOR_BLUE,
    [STAGE_EDGE_BARS] = COLOR_BLUE,
    [STAGE_CENTER_BARS] = COLOR_BLUE,
  },
  // Scene 16: Preservice 16
  [16] = {
    [STAGE_SPOTS] = BRIGHTNESS_LOW,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_FULL,
    [WASH_LIGHTS] = COLOR_RED,
    [STAGE_EDGE_BARS] = COLOR_RED,
    [STAGE_CENTER_BARS] = COLOR_RED,
  },
  // Scene 17: Music 1
  [17] = {
    [STAGE_SPOTS] = BRIGHTNESS_HIGH,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_MED,
    [WASH_LIGHTS] = COLOR_ORANGE,
    [STAGE_EDGE_BARS] = COLOR_BLUE,
    [STAGE_CENTER_BARS] = COLOR_BLUE,
  },
  // Scene 18: Music 2
  [18] = {
    [STAGE_SPOTS] = BRIGHTNESS_HIGH,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_MED,
    [WASH_LIGHTS] = COLOR_BLUE,
    [STAGE_EDGE_BARS] = COLOR_ORANGE,
    [STAGE_CENTER_BARS] = COLOR_ORANGE,
  },
  // Scene 19: Music 3
  [19] = {
    [STAGE_SPOTS] = BRIGHTNESS_HIGH,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_MED,
    [WASH_LIGHTS] = COLOR_BLUE,
    [STAGE_EDGE_BARS] = COLOR_GREEN,
    [STAGE_CENTER_BARS] = COLOR_GREEN,
  },
  // Scene 20: Music 4
  [20] = {
    [STAGE_SPOTS] = BRIGHTNESS_HIGH,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_MED,
    [WASH_LIGHTS] = COLOR_RED,
    [STAGE_EDGE_BARS] = COLOR_BLUE,
    [STAGE_CENTER_BARS] = COLOR_BLUE,
  },
  // Scene 21: Music 5
  [21] = {
    [STAGE_SPOTS] = BRIGHTNESS_HIGH,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_MED,
    [WASH_LIGHTS] = COLOR_WHITE,
    [STAGE_EDGE_BARS] = COLOR_GREEN,
    [STAGE_CENTER_BARS] = COLOR_GREEN,
  },
  // Scene 22: Music 6
  [22] = {
    [STAGE_SPOTS] = BRIGHTNESS_HIGH,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_MED,
    [WASH_LIGHTS] = COLOR_MAGENTA,
    [STAGE_EDGE_BARS] = COLOR_CYAN,
    [STAGE_CENTER_BARS] = COLOR_CYAN,
  },
  // Scene 23: Music 7
  [23] = {
    [STAGE_SPOTS] = BRIGHTNESS_HIGH,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_MED,
    [WASH_LIGHTS] = COLOR_YELLOW,
    [STAGE_EDGE_BARS] = COLOR_GREEN,
    [STAGE_CENTER_BARS] = COLOR_GREEN,
  },
  // Scene 24: Music 8
  [24] = {
    [STAGE_SPOTS] = BRIGHTNESS_HIGH,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_MED,
    [WASH_LIGHTS] = COLOR_CYAN,
    [STAGE_EDGE_BARS] = COLOR_RED,
    [STAGE_CENTER_BARS] = COLOR_RED,
  },
  // Scene 25: Music 9
  [25] = {
    [STAGE_SPOTS] = BRIGHTNESS_HIGH,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_MED,
    [WASH_LIGHTS] = COLOR_GREEN,
    [STAGE_EDGE_BARS] = COLOR_BLUE,
    [STAGE_CENTER_BARS] = COLOR_BLUE,
  },
  // Scene 26: Music 10
  [26] = {
    [STAGE_SPOTS] = BRIGHTNESS_HIGH,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_MED,
    [WASH_LIGHTS] = COLOR_GREEN,
    [STAGE_EDGE_BARS] = COLOR_RED,
    [STAGE_CENTER_BARS] = COLOR_RED,
  },
  // Scene 27: Music 11
  [27] = {
    [STAGE_SPOTS] = BRIGHTNESS_HIGH,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_MED,
    [WASH_LIGHTS] = COLOR_RED,
    [STAGE_EDGE_BARS] = COLOR_PURPLE,
    [STAGE_CENTER_BARS] = COLOR_PURPLE,
  },
  // Scene 28: Music 12
  [28] = {
    [STAGE_SPOTS] = BRIGHTNESS_HIGH,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_MED,
    [WASH_LIGHTS] = COLOR_PURPLE,
    [STAGE_EDGE_BARS] = COLOR_RED,
    [STAGE_CENTER_BARS] = COLOR_RED,
  },
  // Scene 29: Music 13
  [29] = {
    [STAGE_SPOTS] = BRIGHTNESS_HIGH,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_MED,
    [WASH_LIGHTS] = COLOR_WHITE,
    [STAGE_EDGE_BARS] = COLOR_RED,
    [STAGE_CENTER_BARS] = COLOR_RED,
  },
  // Scene 30: Music 14
  [30] = {
    [STAGE_SPOTS] = BRIGHTNESS_HIGH,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_MED,
    [WASH_LIGHTS] = COLOR_GREEN,
    [STAGE_EDGE_BARS] = COLOR_PURPLE,
    [STAGE_CENTER_BARS] = COLOR_PURPLE,
  },
  // Scene 31: Music 15
  [31] = {
    [STAGE_SPOTS] = BRIGHTNESS_HIGH,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_MED,
    [WASH_LIGHTS] = COLOR_BLUE,
    [STAGE_EDGE_BARS] = COLOR_BLUE,
    [STAGE_CENTER_BARS] = COLOR_BLUE,
  },
  // Scene 32: Music 16
  [32] = {
    [STAGE_SPOTS] = BRIGHTNESS_HIGH,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_MED,
    [WASH_LIGHTS] = COLOR_RED,
    [STAGE_EDGE_BARS] = COLOR_RED,
    [STAGE_CENTER_BARS] = COLOR_RED,
  },
  // Scene 33: Sermon 1
  [33] = {
    [STAGE_SPOTS] = BRIGHTNESS_HIGH,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_FULL,
    [WASH_LIGHTS] = COLOR_ORANGE,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 34: Sermon 2
  [34] = {
    [STAGE_SPOTS] = BRIGHTNESS_HIGH,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_FULL,
    [WASH_LIGHTS] = COLOR_BLUE,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 35: Sermon 3
  [35] = {
    [STAGE_SPOTS] = BRIGHTNESS_HIGH,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_FULL,
    [WASH_LIGHTS] = COLOR_RED,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 36: Sermon 4
  [36] = {
    [STAGE_SPOTS] = BRIGHTNESS_HIGH,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_FULL,
    [WASH_LIGHTS] = COLOR_WHITE,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 37: Sermon 5
  [37] = {
    [STAGE_SPOTS] = BRIGHTNESS_HIGH,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_FULL,
    [WASH_LIGHTS] = COLOR_MAGENTA,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 38: Sermon 6
  [38] = {
    [STAGE_SPOTS] = BRIGHTNESS_HIGH,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_FULL,
    [WASH_LIGHTS] = COLOR_YELLOW,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 39: Sermon 7
  [39] = {
    [STAGE_SPOTS] = BRIGHTNESS_HIGH,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_FULL,
    [WASH_LIGHTS] = COLOR_CYAN,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 40: Sermon 8
  [40] = {
    [STAGE_SPOTS] = BRIGHTNESS_HIGH,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_FULL,
    [WASH_LIGHTS] = COLOR_GREEN,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 41: Sermon 9
  [41] = {
    [STAGE_SPOTS] = BRIGHTNESS_HIGH,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_FULL,
    [WASH_LIGHTS] = COLOR_PURPLE,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 42: Sermon 10
  [42] = {
    [STAGE_SPOTS] = BRIGHTNESS_HIGH,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_FULL,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 43: Sermon 11
  [43] = {
    [STAGE_SPOTS] = BRIGHTNESS_HIGH,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_FULL,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 44: Sermon 12
  [44] = {
    [STAGE_SPOTS] = BRIGHTNESS_HIGH,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_FULL,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 45: Sermon 13
  [45] = {
    [STAGE_SPOTS] = BRIGHTNESS_HIGH,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_FULL,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 46: Sermon 14
  [46] = {
    [STAGE_SPOTS] = BRIGHTNESS_HIGH,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_FULL,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 47: Sermon 15
  [47] = {
    [STAGE_SPOTS] = BRIGHTNESS_HIGH,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_FULL,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 48: Sermon 16
  [48] = {
    [STAGE_SPOTS] = BRIGHTNESS_HIGH,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_FULL,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 49:
  [49] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 50:
  [50] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 51:
  [51] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 52:
  [52] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 53:
  [53] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 54:
  [54] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 55:
  [55] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 56:
  [56] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 57:
  [57] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 58:
  [58] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 59:
  [59] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 60:
  [60] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 61:
  [61] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 62:
  [62] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 63:
  [63] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 64:
  [64] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 65:
  [65] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 66:
  [66] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 67:
  [67] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 68:
  [68] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 69:
  [69] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 70:
  [70] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 71:
  [71] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 72:
  [72] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 73:
  [73] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 74:
  [74] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 75:
  [75] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 76:
  [76] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 77:
  [77] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 78:
  [78] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 79:
  [79] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 80:
  [80] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 81:
  [81] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 82:
  [82] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 83:
  [83] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 84:
  [84] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 85:
  [85] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 86:
  [86] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 87:
  [87] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 88:
  [88] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 89:
  [89] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 90:
  [90] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 91:
  [91] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 92:
  [92] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 93:
  [93] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 94:
  [94] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 95:
  [95] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 96:
  [96] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 97:
  [97] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 98:
  [98] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 99:
  [99] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  /*
   * Scenes 100-110 are Good Friday scenes.
   * 
   * Red side lighting, red stage lighting, fading overheads/spots.
   * 
   * Scene 100 is 0%, scene 110 is 100%.
   * 
   * Red is on full until 0%, at which point it drops to black.
   * 
   * Overheads start with the music scene layout and fade.
   */
  // Scene 100:
  [100] = {
    [STAGE_SPOTS] = DIM(BRIGHTNESS_HIGH, 0),
    [AUDIENCE_LIGHTS] = DIM(BRIGHTNESS_MED, 0),
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 101:
  [101] = {
    [STAGE_SPOTS] = DIM(BRIGHTNESS_HIGH, 10),
    [AUDIENCE_LIGHTS] = DIM(BRIGHTNESS_MED, 10),
    [WASH_LIGHTS] = COLOR_RED,
    [STAGE_EDGE_BARS] = COLOR_RED,
    [STAGE_CENTER_BARS] = COLOR_RED,
  },
  // Scene 102:
  [102] = {
    [STAGE_SPOTS] = DIM(BRIGHTNESS_HIGH, 20),
    [AUDIENCE_LIGHTS] = DIM(BRIGHTNESS_MED, 20),
    [WASH_LIGHTS] = COLOR_RED,
    [STAGE_EDGE_BARS] = COLOR_RED,
    [STAGE_CENTER_BARS] = COLOR_RED,
  },
  // Scene 103:
  [103] = {
    [STAGE_SPOTS] = DIM(BRIGHTNESS_HIGH, 30),
    [AUDIENCE_LIGHTS] = DIM(BRIGHTNESS_MED, 30),
    [WASH_LIGHTS] = COLOR_RED,
    [STAGE_EDGE_BARS] = COLOR_RED,
    [STAGE_CENTER_BARS] = COLOR_RED,
  },
  // Scene 104:
  [104] = {
    [STAGE_SPOTS] = DIM(BRIGHTNESS_HIGH, 40),
    [AUDIENCE_LIGHTS] = DIM(BRIGHTNESS_MED, 40),
    [WASH_LIGHTS] = COLOR_RED,
    [STAGE_EDGE_BARS] = COLOR_RED,
    [STAGE_CENTER_BARS] = COLOR_RED,
  },
  // Scene 105:
  [105] = {
    [STAGE_SPOTS] = DIM(BRIGHTNESS_HIGH, 50),
    [AUDIENCE_LIGHTS] = DIM(BRIGHTNESS_MED, 50),
    [WASH_LIGHTS] = COLOR_RED,
    [STAGE_EDGE_BARS] = COLOR_RED,
    [STAGE_CENTER_BARS] = COLOR_RED,
  },
  // Scene 106:
  [106] = {
    [STAGE_SPOTS] = DIM(BRIGHTNESS_HIGH, 60),
    [AUDIENCE_LIGHTS] = DIM(BRIGHTNESS_MED, 60),
    [WASH_LIGHTS] = COLOR_RED,
    [STAGE_EDGE_BARS] = COLOR_RED,
    [STAGE_CENTER_BARS] = COLOR_RED,
  },
  // Scene 107:
  [107] = {
    [STAGE_SPOTS] = DIM(BRIGHTNESS_HIGH, 70),
    [AUDIENCE_LIGHTS] = DIM(BRIGHTNESS_MED, 70),
    [WASH_LIGHTS] = COLOR_RED,
    [STAGE_EDGE_BARS] = COLOR_RED,
    [STAGE_CENTER_BARS] = COLOR_RED,
  },
  // Scene 108:
  [108] = {
    [STAGE_SPOTS] = DIM(BRIGHTNESS_HIGH, 80),
    [AUDIENCE_LIGHTS] = DIM(BRIGHTNESS_MED, 80),
    [WASH_LIGHTS] = COLOR_RED,
    [STAGE_EDGE_BARS] = COLOR_RED,
    [STAGE_CENTER_BARS] = COLOR_RED,
  },
  // Scene 109:
  [109] = {
    [STAGE_SPOTS] = 100, //DIM(BRIGHTNESS_HIGH, 90),
    [AUDIENCE_LIGHTS] = 100, //DIM(BRIGHTNESS_MED, 90),
    [WASH_LIGHTS] = COLOR_RED,
    [STAGE_EDGE_BARS] = COLOR_RED,
    [STAGE_CENTER_BARS] = COLOR_RED,
  },
  // Scene 110:
  [110] = {
    [STAGE_SPOTS] = DIM(BRIGHTNESS_HIGH, 100),
    [AUDIENCE_LIGHTS] = DIM(BRIGHTNESS_MED, 100),
    [WASH_LIGHTS] = COLOR_RED,
    [STAGE_EDGE_BARS] = COLOR_RED,
    [STAGE_CENTER_BARS] = COLOR_RED,
  },
  // Scene 111:
  [111] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 112:
  [112] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 113:
  [113] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 114:
  [114] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 115:
  [115] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 116:
  [116] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 117:
  [117] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 118:
  [118] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 119:
  [119] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 120:
  [120] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 121:
  [121] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 122:
  [122] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 123:
  [123] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 124:
  [124] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 125:
  [125] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 126:
  [126] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
  // Scene 127:
  [127] = {
    [STAGE_SPOTS] = BRIGHTNESS_OFF,
    [AUDIENCE_LIGHTS] = BRIGHTNESS_OFF,
    [WASH_LIGHTS] = COLOR_OFF,
    [STAGE_EDGE_BARS] = COLOR_OFF,
    [STAGE_CENTER_BARS] = COLOR_OFF,
  },
};
