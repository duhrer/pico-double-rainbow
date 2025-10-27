// Timing Defaults

// How long to wait between "ticks"
#define MS_PER_TICK 10

// How often to update the brightness offset for solid colours and the colour
// wheel ("pulsing")
#define COLOUR_PULSE_TICKS 10

// How often to change colours when in "wheel" mode.
#define TICKS_PER_CONTINUOUS_COLOUR_CHANGE 200

// If the IR circuit is not working, the lights will flash in a particular
// pattern.  This controls how fast the pattern flashes.
#define TICKS_PER_ERROR_TOGGLE 100

// How often to change the position of each "band" of colours in "rainbow" mode.
#define R_TICKS 25
#define G_TICKS 20
#define B_TICKS 15

// The direction in which the bands move in the inner ring in "rainbow" mode.
// This will be determined in part by the rings you have and whether you want
// the rings to move together or in opposition.
#define INNER_DIRECTION 1
#define INNER_BAND_WIDTH 11

// The direction in which the bands move in outer ring in "rainbow" mode. See
// above.
#define OUTER_DIRECTION -1
#define OUTER_BAND_WIDTH 45

#define COLOUR_PULSE_PERCENTAGE_INCREMENT 5

# define IR_RX_GPIO_PIN 29
