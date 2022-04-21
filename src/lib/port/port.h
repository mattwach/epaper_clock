#ifndef PORT_H
#define PORT_H

#include <avr/io.h>

// This is used make it convienent to set every unused port as
// an input pullup (to avoid CMOS power drain issues).
#ifdef __AVR_MEGA__
static inline void port_set_all_input_pullup(void) {
  DDRB = 0x00;
  PORTB = 0xFF;
  DDRC = 0x00;
  PORTC = 0x7F;  // there is no PC7
  DDRD = 0x00;
  PORTD = 0xFF;
}

// Careful with this one anytin
static inline void port_set_output_low(
    uint8_t portb, uint8_t portc, uint8_t portd) {
  // avoid touching these pins:
  // PB6, PB7 (oscillator)
  portb &= ~((1 << 6) | (1 << 7));
  // PC6 (reset)
  portc &= ~(1 << 6);
  // PD0 (RXI)
  portd &= ~(1 << 0);

  DDRB |= portb;
  PORTB &= ~portb;
  DDRC |= portc;
  PORTC &= ~portc;
  DDRD |= portd;
  PORTD &= ~portd;
}
#else
// ATTiny
static inline void port_set_all_input_pullup(void) {
  DDRB = 0x00;
  PORTB = 0x3F;  // There are only 6 pins
}
#endif

#endif

