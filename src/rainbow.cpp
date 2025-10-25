#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "Adafruit_NeoPixel.hpp"

// Copied from pico-examples
#include "nec_receive_library/nec_receive.c"

#include "colours.h"

# define IR_RX_GPIO_PIN 29

// See: https://lectronz.com/products/rp2040-stamp-round-carrier
#define INNER_RING_PIN 24
#define INNER_RING_PIXELS 16

#define OUTER_RING_PIN 26
#define OUTER_RING_PIXELS 60

Adafruit_NeoPixel inner_pixels = Adafruit_NeoPixel(INNER_RING_PIXELS, INNER_RING_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel outer_pixels = Adafruit_NeoPixel(OUTER_RING_PIXELS, OUTER_RING_PIN, NEO_GRB + NEO_KHZ800);

bool keep_running = true;

int ticks = 0;

PIO pio = pio0;
int rx_sm;

bool isLit = true;

// TODO: This should be a more defined structure
int colourMode = 2; // 0: Solid, 1: Discrete Rainbow, 2: Continuous Rainbow, 999: Error

TARGET_COLOUR current_colour = TC_BLUE;

#define MS_PER_TICK 10

#define MAX_BRIGHTNESS 176
#define MIN_BRIGHTNESS 16
#define BRIGHTNESS_DELTA 32

int combined_brightness = 160;

#define INNER_DIRECTION 1
#define INNER_BAND_WIDTH 11

#define OUTER_DIRECTION -1
#define OUTER_BAND_WIDTH 45

#define R_TICKS 25
#define G_TICKS 20
#define B_TICKS 20

// Oscillation for solid colours and colour wheel

// Oscillate above and below the current brightness
#define COLOUR_PULSE_RANGE 16

// How often to update the brightness offset
#define COLOUR_PULSE_TICKS 10

int colour_pulse_brightness_offset = 0;
int colour_pulse_direction = 1;

int inner_r_index = 0;
int inner_g_index = 6;
int inner_b_index = 11;

int outer_r_index = 0;
int outer_g_index = 20;
int outer_b_index = 40;

#define TICKS_PER_CONTINUOUS_COLOUR_CHANGE 200

#define TICKS_PER_ERROR_TOGGLE 100

int continuous_colour_change_index = 0;

void setup () {
    // NeoPixel Init
    inner_pixels.begin();
    inner_pixels.clear();
    inner_pixels.show();

    outer_pixels.begin();
    outer_pixels.clear();
    outer_pixels.show();

    stdio_init_all();

    rx_sm = nec_rx_init(pio, IR_RX_GPIO_PIN);

    if (rx_sm == -1) {
        colourMode = 999;
    }
}

// This is currently hard-coded to be quite low as the performance of the outer
// ring of pixels is inconsistent when the power demand is too high.
void fill_band_values (int numPixels, int band_width, int index, int values[]) {
    int dot_intensity = combined_brightness / 3;
    values[index] = dot_intensity;

    int bands = ((band_width -1) / 2);

    for (int a = 1; a <= bands; a++) {
        int band_intensity = (dot_intensity / (a + 1));
        int leading_band_index = (index + a) % numPixels;
        values[leading_band_index] = band_intensity;

        int trailing_band_index = (index - a + numPixels) % numPixels;
        values[trailing_band_index] = band_intensity;
    }
}

int next_index (int current_index, int direction, int numPixels) {
    int raw_index = current_index + direction;
    return (raw_index + numPixels) % numPixels;
}

void redraw_rainbow_colours () {
    bool isDirty = false;

    if ((ticks % R_TICKS) == 0 ) {
        isDirty = true;
        inner_r_index = next_index(inner_r_index, INNER_DIRECTION, INNER_RING_PIXELS);
        outer_r_index = next_index(outer_r_index, OUTER_DIRECTION, OUTER_RING_PIXELS);
    }

    if ((ticks % G_TICKS) == 0) {
        isDirty = true;
        inner_g_index = next_index(inner_g_index, INNER_DIRECTION, INNER_RING_PIXELS);
        outer_g_index = next_index(outer_g_index, OUTER_DIRECTION, OUTER_RING_PIXELS);
    }

    if ((ticks % B_TICKS) == 0) {
        isDirty = true;
        inner_b_index = next_index(inner_b_index, INNER_DIRECTION, INNER_RING_PIXELS);
        outer_b_index = next_index(outer_b_index, OUTER_DIRECTION, OUTER_RING_PIXELS);
    }

    if (isDirty) {
        int inner_r_values[INNER_RING_PIXELS] = {0};
        fill_band_values (INNER_RING_PIXELS, INNER_BAND_WIDTH, inner_r_index, inner_r_values);

        int inner_g_values[INNER_RING_PIXELS] = {0};
        fill_band_values (INNER_RING_PIXELS, INNER_BAND_WIDTH, inner_g_index, inner_g_values);

        int inner_b_values[INNER_RING_PIXELS] = {0};
        fill_band_values (INNER_RING_PIXELS, INNER_BAND_WIDTH, inner_b_index, inner_b_values);

        inner_pixels.clear();
        for (int i = 0; i < INNER_RING_PIXELS; i++) {
            int r = inner_r_values[i];
            int g = inner_g_values[i];
            int b = inner_b_values[i];

            if (r + g + b > 0) {
                inner_pixels.setPixelColor(i, inner_pixels.Color(r, g, b));
            }
        }
        inner_pixels.show();

        int outer_r_values[OUTER_RING_PIXELS] = {0};
        fill_band_values (OUTER_RING_PIXELS, OUTER_BAND_WIDTH, outer_r_index, outer_r_values);

        int outer_g_values[OUTER_RING_PIXELS] = {0};
        fill_band_values (OUTER_RING_PIXELS, OUTER_BAND_WIDTH, outer_g_index, outer_g_values);

        int outer_b_values[OUTER_RING_PIXELS] = {0};
        fill_band_values (OUTER_RING_PIXELS, OUTER_BAND_WIDTH, outer_b_index, outer_b_values);

        outer_pixels.clear();
        for (int i = 0; i < OUTER_RING_PIXELS; i++) {
            int r = outer_r_values[i];
            int g = outer_g_values[i];
            int b = outer_b_values[i];

            if (r + g + b > 0) {
                outer_pixels.setPixelColor(i, outer_pixels.Color(r, g, b));
            }
        }
        outer_pixels.show();
    }
}

void redraw_pixels () {
    // Give visible feedback if the IR system doesn't initialise correctly.
    if (colourMode == 999) {
        bool lightInner = ((ticks % TICKS_PER_ERROR_TOGGLE) < (TICKS_PER_ERROR_TOGGLE / 2));
        bool lightOuter = !lightInner;

        if (lightInner) {
            inner_pixels.fill(inner_pixels.Color(64, 0, 0), 0, INNER_RING_PIXELS);
        }
        else {
            inner_pixels.clear();
        }
        inner_pixels.show();

        if (lightOuter) {
            outer_pixels.fill(outer_pixels.Color(32, 0, 0), 0, OUTER_RING_PIXELS);
        }
        else {
            outer_pixels.clear();
        }
        outer_pixels.show();        
    } 
    else if (colourMode == 2) {
        redraw_rainbow_colours();
    }
    else {
        TARGET_COLOUR colour_to_paint =  colourMode == 0 ? current_colour : COLOUR_WHEEL[continuous_colour_change_index];

        double inner_brightness_percentage_per_colour = (combined_brightness + colour_pulse_brightness_offset) / 255.0;
        uint32_t inner_colour = inner_pixels.Color(
            colour_to_paint.r * inner_brightness_percentage_per_colour,
            colour_to_paint.g * inner_brightness_percentage_per_colour,
            colour_to_paint.b * inner_brightness_percentage_per_colour
        );

        inner_pixels.fill(inner_colour, 0, INNER_RING_PIXELS);
        inner_pixels.show();

        // The rings should pulse in opposition to each other.
        double outer_brightness_percentage_per_colour = (combined_brightness - colour_pulse_brightness_offset) / 255.0;
        uint32_t outer_colour = outer_pixels.Color(
            colour_to_paint.r * outer_brightness_percentage_per_colour,
            colour_to_paint.g * outer_brightness_percentage_per_colour,
            colour_to_paint.b * outer_brightness_percentage_per_colour
        );

        outer_pixels.fill(outer_colour, 0, OUTER_RING_PIXELS);
        outer_pixels.show();
    }
}

/*

    This code supports most of the IR signals used by the 'practical series ii'
    remote that controls various cheap party lights:

    00:45 - On button (at last used brightness)
    00:46 - White + Dimmed brightness
    00:47 - Off button
    00:44 - 'Stepped' colour cycling
    00:40 - Turn on timer?
    00:43 - Add one hour to timer
    00:07 - Continuous transitions between colours
    00:15 - Increase brightness
    00:09 - Decrease brightness
    00:16 - Red
    00:19 - Green
    00:0d - Blue
    00:0c - Orange
    00:18 - Light Green
    00:5e - Light Blue
    00:08 - Blueish Purple
    00:1c - Light Orange
    00:5a - Lightest Blue
    00:42 - Reddish Purple
    00:52 - Yellow
    00:4a - White + Full Brightness

*/

uint8_t rx_address, rx_data, previous_rx_data;

void poll_infrared () {
    // Adapted from pico-examples
    while (!pio_sm_is_rx_fifo_empty(pio, rx_sm)) {
        uint32_t rx_frame = pio_sm_get(pio, rx_sm);

        if (nec_decode_frame(rx_frame, &rx_address, &rx_data)) {
            // Brief flicker to give the impression that something was received.
            inner_pixels.clear();
            inner_pixels.show();
            outer_pixels.clear();
            outer_pixels.show();

            if (rx_data != previous_rx_data) {
                int old_brightness = combined_brightness;
                bool wasLit = isLit;

                if (isLit) {
                        switch(rx_data) {
                        // 00:46 - White + Dimmed brightness
                        case 0x46:
                            colourMode = 0;
                            combined_brightness = MIN_BRIGHTNESS;
                            current_colour = TC_WHITE;
                            break;
                        // 00:47 - Off button
                        case 0x47:
                            isLit = false;
                            break;
                        // 00:44 - 'Stepped' colour cycling
                        case 0x44:
                            colourMode = 1;
                            break;
                        // 00:40 - Turn on timer (ignored)
                        // 00:43 - Add one hour to timer (ignored)
                        case 0x40:
                        case 0x43:
                            break;
                        // 00:07 - Continuous transitions between colours
                        case 0x07:
                            colourMode = 2;
                        break;
                        // 00:15 - Increase brightness
                        case 0x15:
                            if (combined_brightness <= (MAX_BRIGHTNESS - BRIGHTNESS_DELTA)) {
                                combined_brightness += BRIGHTNESS_DELTA;
                            }
                            break;
                        // 00:09 - Decrease brightness
                        case 0x09:
                            if (combined_brightness >= (MIN_BRIGHTNESS + BRIGHTNESS_DELTA)) {
                                combined_brightness -= BRIGHTNESS_DELTA;
                            }
                            break;
                        // 00:16 - Red
                        case 0x16:
                            colourMode = 0;
                            current_colour = TC_RED;
                            break;
                        // 00:19 - Green
                        case 0x19:
                            colourMode = 0;
                            current_colour = TC_GREEN;
                            break;
                        // 00:0d - Blue
                        case 0x0d:
                            colourMode = 0;
                            current_colour = TC_BLUE;
                            break;
                        // 00:0c - Orange
                        case 0x0c:
                            colourMode = 0;
                            current_colour = TC_ORANGE;
                            break;
                        // 00:18 - Light Green
                        case 0x18:
                            colourMode = 0;
                            current_colour = TC_LIGHTER_GREEN;
                            break;
                        // 00:5e - Light Blue
                        case 0x5e:
                            colourMode = 0;
                            current_colour = TC_LIGHTER_BLUE;
                            break;
                        // 00:08 - Blueish Purple
                        case 0x08:
                            colourMode = 0;
                            current_colour = TC_INDIGO;
                            break;
                        // 00:1c - Light Orange
                        case 0x1c:
                            colourMode = 0;
                            current_colour = TC_LIGHTER_ORANGE;
                            break;
                        // 00:5a - Lightest Blue
                        case 0x5a:
                            colourMode = 0;
                            current_colour = TC_LIGHTEST_BLUE;
                            break;
                        // 00:42 - Reddish Purple
                        case 0x42:
                            colourMode = 0;
                            current_colour = TC_VIOLET;
                            break;
                        // 00:52 - Yellow
                        case 0x52:
                            colourMode = 0;
                            current_colour = TC_YELLOW;
                            break;
                        // 00:4a - White + Full Brightness
                        case 0x4a:
                            colourMode = 0;
                            current_colour = TC_WHITE;
                            combined_brightness = MAX_BRIGHTNESS;
                            break;
                        default:
                            break;
                    }
                }
                // 00:45 - On button (at last used brightness)
                else if (rx_data == 0x45) {
                    isLit = true;
                }

                if (wasLit) {
                    // Turn off all pixels.
                    inner_pixels.fill(inner_pixels.Color(0,0,0));
                    inner_pixels.show();
                    outer_pixels.fill(outer_pixels.Color(0,0,0));
                    outer_pixels.show();
                }                    
            }

            previous_rx_data = rx_data;
        }
        else {
            // Unsupported...
        }
    }
}

int main() {
    setup();

    while (keep_running) {
        // Cycle the colours whether or not we're currently running.
        if ((ticks % TICKS_PER_CONTINUOUS_COLOUR_CHANGE) == 0) {
            continuous_colour_change_index = (continuous_colour_change_index + 1) % 6;
        }

        // Adjust solid colour brightness automatically every few ticks
        if ((ticks % COLOUR_PULSE_TICKS) == 0) {
            colour_pulse_brightness_offset += colour_pulse_direction;

            if (colour_pulse_brightness_offset == COLOUR_PULSE_RANGE) {
                colour_pulse_direction *= -1;
            }
            else if (colour_pulse_brightness_offset == (COLOUR_PULSE_RANGE * -1)) {
                colour_pulse_direction *= -1;
            }
        }

        if (rx_sm != -1) {
            poll_infrared();
        }

        if (isLit) {
            redraw_pixels();
        }

        ticks++;
        sleep_ms(MS_PER_TICK);
    }
}