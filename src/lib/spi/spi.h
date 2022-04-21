#ifndef APILIB_SPI
#define APILIB_SPI

#include <avr/io.h>

// Hardware connections
//
//       ATMEGA328P   ATTINY85
// MOSI  D11          PB1
// MISO  D12          PB0
// SCK   D13          PB2
// CS    UserDef      UserDef
// D/C   UserDef      UserDef  (if needed)

// SPI Modes
// From the hardware spec:
//  Mode 0: SPI_INVERT_CLOCK=0 SPI_PHASE=0
//  Mode 1: SPI_INVERT_CLOCK=0 SPI_PHASE=1
//  Mode 2: SPI_INVERT_CLOCK=1 SPI_PHASE=0
//  Mode 3: SPI_INVERT_CLOCK=1 SPI_PHASE=1

// Bit mask flags
#define SPI_USE_MOSI 0x01     // bit 0
#define SPI_USE_MISO 0x02     // bit 1
#define SPI_INVERT_CLOCK 0x04 // Bit 2
// Note: SPI_PHASE is a NOOP on ATTiny
#define SPI_PHASE 0x08       // Bit 4
// Note: SPI_LSB is a NOOP on ATTiny
#define SPI_LSB   0x10       // Bit 5

// Initialize SPI
//
// flags: is a bit field.  You can OR parameters together
//   USE_MOSI: If set, configure MOSI reg as an output
//   USE_MISO: If set, configure MISO reg as an input
//   INVERT_CLOCK: If set, invert the clock polarity
//
//   freq: SPR0, SPR1, SPI2X all combine with CPU_FREQ to get the highest
//     value < freq.  It will always pick at least f/128 (the lowest setting).
//     Note that on a tiny device, freq is ignored.
void spi_initMaster(uint8_t flags);
void spi_initMasterFreq(uint8_t flags, uint32_t freq);

// Blocking write or read a SPI byte.
// 
// The caller needs to
//   1) Drop the SS pin to begin a transaction
//   2) call spi_sync_read and spi_sync_write as many times as needed to read and write bytes
//   3) reraise the SS pin
void spi_syncWrite(uint8_t data);
uint8_t spi_syncTransact(uint8_t data);  // write and read
uint8_t spi_syncRead();

#endif  // APILIB_SPI
