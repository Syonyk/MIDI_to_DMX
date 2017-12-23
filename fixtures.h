/**
 * Fixture mode.  This allows control of individual fixtures by color when
 * properly defined.  This is a more flexible mode than scene mode, but requires
 * a bit more in the way of programming on the Proclaim side of things.
 * 
 * However, you can create brand new color arrangements without having to
 * reprogram the converter, which is quite useful.
 * 
 * To send fixture commands from Proclaim, create a new lighting command with
 * the "Note Off" type.  The "Channel" selects the fixture, the "Note" selects
 * either the light color index or sets the brightness for a single channel
 * fixture, and the "Velocity" sets the fade time (in seconds).
 */

#define FIXTURE_UNUSED 0x00
#define FIXTURE_RGB 0x10
#define FIXTURE_WHITE 0x20

// As MIDI channels are used to select fixtures, only 16 fixtures are allowed.
#define MAX_FIXTURE_COUNT 16

/**
 * RGB colors.  This array is a list of RGB colors that can be selected for each
 * RGB fixture from Proclaim.  This list is from defines.h
 * 
 * If you'd like to add a new color, feel free to do so.  You can add them at
 * the bottom in the same style as the existing colors, and they should be
 * usable after a recompile/flash of the code.
 * 
 * You can either add a new color in defines.h in the same format, or,
 * alternately, add a color in {RRR, GGG, BBB} format - that's what the macros
 * expand to.
 */

const PROGMEM uint8_t colors[][3] = {
// RED  GRN  BLUE
  {COLOR_OFF},     //  0
  {COLOR_RED},     //  1
  {COLOR_ORANGE},  //  2
  {COLOR_YELLOW},  //  3
  {COLOR_LIME},    //  4
  {COLOR_GREEN},   //  5
  {COLOR_CYAN},    //  6
  {COLOR_LT_BLUE}, //  7
  {COLOR_BLUE},    //  8
  {COLOR_PURPLE},  //  9
  {COLOR_MAGENTA}, // 10
  {COLOR_WHITE},   // 11
};

#define MAX_FIXTURE_COLOR (sizeof(colors) / 3)


typedef struct {
  uint8_t fixture_type;
  uint8_t fixture_base_address;
} fixture_data;

/**
 * Up to 16 fixtures can be defined - if you need more, you should probably
 * consider scene mode, or a proper light board.  Or use raw DMX mode.
 * 
 * A "fixture" is a set of lights operating on a single channel.  If you have 4
 * different lights listening on channels 32, 33, 34, that's still a single
 * fixture from this perspective.
 * 
 * Each fixture is defined as a type (RGB or white) and a base address.  This is
 * the base for the RED channel, on a RGB fixture, or the brightness channel for
 * white fixtures.  If you have other channels that need to be fixed to certain
 * values (mode or grand master channels, for instance), set those in
 * fixed_channels.h - this doesn't handle setting goofy side values on some of
 * the RGB fixtures out there.
 * 
 * Note that MIDI channels are from 1-16.  These are labeled as such.  Really
 * the offset is a normal zero indexed array.
 */

const PROGMEM fixture_data fixtures[MAX_FIXTURE_COUNT] = {
  {FIXTURE_RGB,      1}, //  1: Wash lights
  {FIXTURE_RGB,     66}, //  2: Stage Side Bars
  {FIXTURE_RGB,      8}, //  3: Stage Center Bars
  {FIXTURE_WHITE,    4}, //  4: Spots
  {FIXTURE_WHITE,    6}, //  5: Audience Overhead
  {FIXTURE_RGB,     34}, //  6: Stage Lights (TEST)
  {FIXTURE_UNUSED,   0}, //  7: Unused
  {FIXTURE_UNUSED,   0}, //  8: Unused
  {FIXTURE_UNUSED,   0}, //  9: Unused
  {FIXTURE_UNUSED,   0}, // 10: Unused
  {FIXTURE_UNUSED,   0}, // 11: Unused
  {FIXTURE_UNUSED,   0}, // 12: Unused
  {FIXTURE_UNUSED,   0}, // 13: Unused
  {FIXTURE_UNUSED,   0}, // 14: Unused
  {FIXTURE_UNUSED,   0}, // 15: Unused
  {FIXTURE_UNUSED,   0}, // 16: Unused
};
