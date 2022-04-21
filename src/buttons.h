#ifndef BUTTONS_H
#define BUTTONS_H

#include <inttypes.h>

#define SELECT_WAS_PRESSED 0x01
#define OPTION_WAS_PRESSED 0x02

// Called during init
// Timer ticks returns a value that ranges from approx 0-255 in approx 1 second.
// It's used for debounce so these values can be approximate.
void buttons_init(uint8_t (*timer_ticks)(void));

// called during init to see if the GPS should be disabled
uint8_t select_button_is_pressed(void);

// called in the main loop to see if a button was pressed
uint8_t button_was_pressed(void);

// called by the button handling code to clear the button
// press state
void clear_button_press_state(void);

#endif

