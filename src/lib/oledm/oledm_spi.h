#ifndef OLEDM_SPI_H
#define OLEDM_SPI_H

// Default address that has be changed by defining this constant when compiling
// this object.
#ifdef __AVR_MEGA__
  #ifndef CS_PIN
  #define CS_DDR DDRB
  #define CS_PORT PORTB
  #define CS_PIN 2  // D10
  #endif

  #ifndef DC_PIN
  #define DC_DDR DDRB
  #define DC_PORT PORTB
  #define DC_PIN 1  // D9
  #endif

  #ifndef RES_PIN
  #define RES_DDR DDRB
  #define RES_PORT PORTB
  #define RES_PIN 0  // D8
  #endif
#else
  #ifndef CS_PIN
  #define CS_DDR DDRB
  #define CS_PORT PORTB
  #define CS_PIN 3  // PB3
  #endif

  #ifndef DC_PIN
  #define DC_DDR DDRB
  #define DC_PORT PORTB
  #define DC_PIN 4  // PB4
  #endif
#endif

#ifndef SPI_FREQUENCY
// Probably higher than the value we'll actually get.
#define SPI_FREQUENCY 10000000
#endif

#endif

