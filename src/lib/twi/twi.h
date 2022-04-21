#ifndef LIB_TWI_H
#define LIB_TWI_H
// Tiny TWI that only support master mode.
// The code started as TinyTwiM and was heavily modified to look like
// the default arduino Wire interface.
//
// The idea is to allow an easy switch between the AtMega and ATTiny chipsets
//
// Hardware Connections:
//
// ATMEGA328P:
//  SDA <- A4
//  SCL <- A5
// ATTINY85:
//  SDA <- PB0
//  SCL <- PB2
//
// How to use
//
// void main() {
//   twi_init();
// }
//
// error_t write_something() {
//   error_t err = 0;
//   twi_startWrite(address, &err);
//   twi_writeNoStop(0x00, &err);
//   twi_writeAndStop(0x01, &err);
//   return err;
// }
//
// void read_something() {
//   unint8_t data[4];
//   error_t err = 0;
//   twi_readWithStop(address, data, sizeof(data), &err);
//   return err;
// }
//
// void write_and_read() {
//   unint8_t data[7];
//   error_t err = 0;
//   twi_startWrite(address, &err);
//   twi_writeNoStop(0x00, &err);
//   twi_readWithStop(address, data, sizeof(data), &err);
//   return err;
// }


#include <error_codes.h>

// Initialize TWI (USI mode for ATTiny85).  This function remembers when you call interface
// and is a NOOP after the first call
void twi_init(void);
// Like twi_init, but always reinitializes
void twi_reinit(void);

// Used to stop a transaction.
// This is an alternative to twi_writeWithStop and twi_readWithStop for
// the case where the code will be cleaner to do the stop explicitly
void twi_stop(error_t* err);

// Used to start a series of writes.
// Args:
//  address: Slave address of device
//  err: If non-zero, the command is skipped.  Sets on error
void twi_startWrite(uint8_t address, error_t* err);

// Used to write a single byte with or without a stop bit.
// Args:
//  byte: The byte to write.
//  err: If non-zero, the command is skipped.  Sets on error
void twi_writeNoStop(uint8_t byte, error_t* err);
static inline void twi_writeWithStop(uint8_t byte, error_t* err) {
  twi_writeNoStop(byte, err);
  twi_stop(err);
}

// Read from a slave device
// Args:
//  byte: The slave address to read from
//  data: Where to put the data
//  length: The number of bytes to read
//  err: If non-zero, the command is skipped.  Sets on error
void twi_readNoStop(uint8_t address, uint8_t* data, uint8_t length, error_t* err);
static inline void twi_readWithStop(
    uint8_t address,
    uint8_t* data,
    uint8_t length,
    error_t* err) {
  twi_readNoStop(address, data, length, err);
  twi_stop(err);
}

#endif
