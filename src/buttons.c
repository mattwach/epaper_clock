// Manages button presses

#include "buttons.h"

#include <avr/interrupt.h>
#include <avr/io.h>

// Define the actual buttons
// Note: The enable_buttons code may need changes if these are modified,
// specifically the PCIFR register.
#define SELECT_BUTTON_PORT PORTD
#define SELECT_BUTTON_INPUT PIND
#define SELECT_BUTTON_PIN 3  // PCINT19

#define OPTION_BUTTON_PORT PORTD
#define OPTION_BUTTON_INPUT PIND
#define OPTION_BUTTON_PIN 4  // PCINT20

// ~1/10 of a second
#define BUTTON_DEBOUNCE_TICKS 25
volatile uint16_t last_button_press_tick;
// The main loop needs to know if a button has been pressed so it can update
// in the next second instead of waiting for the minute mark

// bit field.  Bits are defined in display.h
volatile uint8_t button_was_pressed_bits;

// callback for timer ticks
uint8_t (*get_timer_ticks)(void);

//
// public interface
//

void buttons_init(uint8_t (*timer_ticks)(void)) {
  get_timer_ticks = timer_ticks;

  last_button_press_tick = 0;  // For debounce
  button_was_pressed_bits = 0x00;
  SELECT_BUTTON_PORT |= (1 << SELECT_BUTTON_PIN);  // enable the pullup
  OPTION_BUTTON_PORT |= (1 << OPTION_BUTTON_PIN);  // enable the pullup

  PCICR = 1 << 2;  // Enable pin change interrupts for the buttons

  // Allow these two pins to trigger the interrupt
  PCMSK2 = (1 << SELECT_BUTTON_PIN) | (1 << OPTION_BUTTON_PIN);
}

// Gives the current state of the select button
uint8_t select_button_is_pressed(void) {
  return (SELECT_BUTTON_INPUT & (1 << SELECT_BUTTON_PIN)) == 0;
}

uint8_t button_was_pressed(void) {
  return button_was_pressed_bits;
}

// acknowledges the button press state so that it can be set
// again.
void clear_button_press_state(void) {
  button_was_pressed_bits = 0x00;
}

// This is called whenever either button is pressed
ISR(PCINT2_vect) {
  const uint16_t button_tick = get_timer_ticks();
  // The debounce zone is last_button_press_tick <= button_tick <= last_button_press_tick + BUTTON_DEBOUNCE_TICKS 
  if ((button_tick >= last_button_press_tick) &&
      (button_tick <= (last_button_press_tick + BUTTON_DEBOUNCE_TICKS))) {
    // debounce
    return;
  }
  last_button_press_tick = button_tick;

  if ((SELECT_BUTTON_INPUT & (1 << SELECT_BUTTON_PIN)) == 0) {
    button_was_pressed_bits |= SELECT_WAS_PRESSED;
  }

  if ((OPTION_BUTTON_INPUT & (1 << OPTION_BUTTON_PIN)) == 0) {
    button_was_pressed_bits |= OPTION_WAS_PRESSED;
  }

  if (button_was_pressed_bits) {
    last_button_press_tick &= 0xFF;
  }
}
