#include "spi.h"

// Pin layouts.  You might need to access these directly in special cases.
// For example, if using a chip that uses a single pin for MISO/MOSI, you'll
// need to change the output state of the PIN yourself between reads and writes.
#define MOSI_DDR DDRB
#define MOSI_PORT PORTB
#define MOSI_PIN 3  // Nano pin 11

#define MISO_DDR DDRB
#define MISO_PORT PORTB
#define MISO_PIN 4  // Nano pin 12

#define SCK_DDR DDRB
#define SCK_PORT PORTB
#define SCK_PIN 5  // Nano pin 13

void spi_initMaster(uint8_t flags) {
  SPCR = (1 << SPE) | (1 << MSTR);

  if (flags & SPI_LSB) {
    SPCR |= (1 << DORD);
  }

  if (flags & SPI_INVERT_CLOCK) {
    SPCR |= (1 << CPOL);
  }

  if (flags & SPI_PHASE) {
    SPCR |= (1 << CPHA);
  }

  if (flags & SPI_USE_MOSI) {
    MOSI_DDR |= (1 << MOSI_PIN);
  }

  if (flags & SPI_USE_MISO) {
    MISO_DDR &= ~(1 << MISO_PIN);
  }

  SCK_DDR |= (1 << SCK_PIN);
}

void spi_initMasterFreq(uint8_t flags, uint32_t freq) {

  spi_initMaster(flags);
  // Find the bit that corresponds to the requested frequency.
  // Start with the highest, which uses a clock divider of 2
  uint32_t real_freq = F_CPU >> 1;
  uint8_t divider = 0;
  while ((real_freq > freq) && (divider < 6)) {
    real_freq >>= 1;  // halve the cpu freq
    ++divider;   // next divider
  }

  uint8_t spcr = 0;
  // set the SPI2X, SPR1 and SPR2 bits according to the divider value
  switch (divider) {
    case 0:  // 2
      SPSR = (1 << SPI2X);
      break;
    case 1:  // 4
      SPSR = 0;
      break;
    case 2:  // 8
      SPSR = (1 << SPI2X);
      spcr |= (1 << SPR0);
      break;
    case 3:  // 16
      SPSR = 0;
      spcr |= (1 << SPR0);
      break;
    case 4:  // 32
      SPSR = (1 << SPI2X);
      spcr |= (1 << SPR1);
      break;
    case 5:  // 64
      SPSR = 0;
      spcr |= (1 << SPR1);
      break;
    default:  // 128
      SPSR = 0;
      spcr |= (1 << SPR1) | (1 << SPR0);
      break;
  }

  SPCR |= spcr;
}


void spi_syncWrite(uint8_t data) {
  SPDR = data;
  while (!(SPSR & (1 << SPIF)));
}

uint8_t spi_syncTransact(uint8_t data) {
  SPDR = data;
  while (!(SPSR & (1 << SPIF)));
  return SPDR;
}

uint8_t spi_syncRead() {
  while (!(SPSR & (1 << SPIF)));
  return SPDR;
}
