#ifndef EPAPER_H
#define EPAPER_H
// API for the epaper.
// To mimimize the amount of duplicate code, this interface leans on the oled
// API.  That said, there are a few concepts that are unique to the e-paper,
// cluding full-vs-partial updates and double-buffering

#include "oledm.h"

// Enter and exit sleep modes.  It is recommended to enter a sleep mode
// after a short period to avoid consuming power and keeping the chip
// at a high voltage level.  Once entered, only a HWRESET can exit.
typedef enum {
  SLEEP_MODE_0 = 0,  // Unclear from datasheet if this is a mode or not
  SLEEP_MODE_1 = 1,  // Retain RAM
  SLEEP_MODE_2 = 3,  // Discard RAM for slightly lower power
  SLEEP_MODE_OFF = 4,  // Do nothing
} EpaperSleepMode;

void epaper_sleep_mode(struct OLEDM* display, EpaperSleepMode sleep_mode);

// Pulse the reset line.  It is unclear whether it's ever a good idea to do this
// without following up with setup parameters.
void epaper_reset(void);
// use this with the *_no_wait functions to check for unbusy.  This is
// useful when you want to do a cheaper wait.
uint8_t epaper_is_busy(void);
// Only waits a few seconds, then asserts NO_ACK_ERROR
void epaper_wait(struct OLEDM* display);

// The SD1680 epaper display is buffered.  Thus everything you write
// to it is not shown to the user unless oyu do a "full" update
// using (epaper_swap_buffers) or a partial update as described below.
//
// Because it's bad for the hardware to forget about sleep mode, it's included as
// a required parameter.  Pass SLEEP_MODE_OFF if you want to skip sleeping.
void epaper_swap_buffers_no_wait(struct OLEDM* display);
void epaper_swap_buffers(struct OLEDM* display, EpaperSleepMode sleep_mode);

// Used to write to the color (e.g. red, yellow) pixels of the display
void epaper_start_color_pixels(struct OLEDM* display);


// Partial updates.
//
// A full update helps improve contrast and eliminate ghosting effects
// but has the downside of flashing the display black to white.
// If eliminating that effect is a priority, you can experiment with
// partial updates.
void epaper_set_partial_mode(struct OLEDM* display);
void epaper_set_full_mode(struct OLEDM* display);
// This is called instead of epaper_swap_buffers when partial mode is used
//
// Because it's bad for the hardware to forget about sleep mode, it's included as
// a required parameter.  Pass SLEEP_MODE_OFF if you want to skip sleeping.
void epaper_update_partial_no_wait(struct OLEDM* display);
void epaper_update_partial(struct OLEDM* display, EpaperSleepMode sleep_mode);

#endif
