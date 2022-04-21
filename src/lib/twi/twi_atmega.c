#include <avr/io.h>
#include <avr/interrupt.h>
#include "twi.h"

#define WAIT_FOR_TWI() while (!(TWCR & (1 << TWINT)))

#define TWSR_MASK 0xF8      // Mask away prescaler bits
#define TWSR_START 0x08     // Start was transmitted
#define TWSR_RESTART 0x10     // Start was transmitted (bus already active)
#define TWSR_SLAW_ACK 0x18  // SLA+W transmitted, ACK received
#define TWSR_SLAW_NACK 0x20 // SLA+W transmitted, NACK received
#define TWSR_WRITE_DATA_ACK 0x28  // Data transmitted, ACK received
#define TWSR_WRITE_DATA_NACK 0x30 // Data transmitted, NACK received
#define TWSR_ARB_LOST 0x38  // Master lost bus arbitration
#define TWSR_SLAR_ACK 0x40  // SLA+R transmitted, ACK received
#define TWSR_SLAR_NACK 0x48 // SLA+R transmitted, NACK received
#define TWSR_READ_DATA_ACK 0x50  // Data transmitted, ACK received
#define TWSR_READ_DATA_NACK 0x58 // Data transmitted, NACK received


#define SDA 4
#define SCL 5
// TODO: make this a runtime option
#ifndef TWI_FREQ
#define TWI_FREQ 500000L
#endif

#ifndef SET_TWDR
#define SET_TWDR(v) TWDR = (v)
#endif

#ifndef GET_TWDR
#define GET_TWDR() TWDR
#endif

void twi_init(void) {
  static uint8_t already_initialized = 0;
  if (!already_initialized) {
    already_initialized = 1;
    twi_reinit();
  }
}

void twi_reinit(void) {
  // activate internal pullups for twi.
  DDRC &= ~((1 << SDA) | (1 << SCL));
  PORTC |= (1 << SDA) | (1 << SCL);

  // initialize twi prescaler and bit rate
  TWSR &= ~TWPS0;
  TWSR &= ~TWPS1;
  TWBR = ((F_CPU / TWI_FREQ) - 16) / 2;
}

// Instructs the TWI hardware to send a start and wait
static error_t start(void) {
  // Tell the hardware to send a start bit
  TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

  WAIT_FOR_TWI();

  // Check if the send was successful
  switch (TWSR & TWSR_MASK) {
      case TWSR_START:
      case TWSR_RESTART:
        return 0;
      default:
        return TWI_MISSING_START_CON_ERROR;
  }
}

// Instructs the TWI hardware to send a stop
void twi_stop(error_t* err) {
  if (*err) {
    return;
  }
  // Tell the hardware to send a stop bit
  TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

// Tell the hardware to send a byte of data and wait
static inline void sendAndWait(uint8_t byte) {
  // Tell the hardware to send an address byte for writing data
  SET_TWDR(byte);
  TWCR = (1 << TWINT) | (1 << TWEN);
  WAIT_FOR_TWI();
}

void twi_startWrite(uint8_t address, error_t* err) {
  if (*err) {
    return;
  }

  *err = start();

  if (!(*err)) {
    sendAndWait(address << 1);  // Shift is via TWI protocol (bit zero indicates a write)
    // Check if the send was successful
    switch (TWSR & TWSR_MASK) {
        case TWSR_SLAW_ACK:
            break;
        case TWSR_SLAW_NACK:
            *err = TWI_NO_ACK_ERROR;
            break;
        case TWSR_ARB_LOST:
            *err = TWI_ARB_LOST_ERROR;
            break;
        default:
            *err = TWI_INTERNAL_ERROR;
            break;
    }
  }
}

// Instructs the TWI hardware to send a data byte and
// wait for an ACK
static error_t writeCommon(uint8_t byte) {
  sendAndWait(byte);

  // Check if the send was successful
  switch (TWSR & TWSR_MASK) {
    case TWSR_WRITE_DATA_ACK:
      return 0;
    case TWSR_WRITE_DATA_NACK:
      return TWI_NO_ACK_ERROR;
    default:
      return TWI_INTERNAL_ERROR;
  }
}

void twi_writeNoStop(uint8_t byte, error_t* err) {
  if (*err) {
    return;
  }

  *err = writeCommon(byte);
}


void twi_readNoStop(uint8_t address, uint8_t* data, uint8_t length, error_t* err) {
  if (*err) {
    return;
  }

  if (((*err) = start()) != 0) {
    return;
  }

  sendAndWait((address << 1) | 1);  // Shift is via TWI protocol (bit zero indicates a read)
  // Check if the send was successful
  switch (TWSR & TWSR_MASK) {
	case TWSR_SLAR_ACK:
	  break;
	case TWSR_SLAR_NACK:
	  *err = TWI_NO_ACK_ERROR;
	  return;
	case TWSR_ARB_LOST:
	  *err = TWI_ARB_LOST_ERROR;
	  return;
	default:
	  *err = TWI_INTERNAL_ERROR;
	  return;
  }

  uint8_t i = 0;
  for (; i < length; ++i) {
	const uint8_t expected_status = i < (length - 1) ?
	  TWSR_READ_DATA_ACK : TWSR_READ_DATA_NACK;

	TWCR = (expected_status == TWSR_READ_DATA_ACK) ?
	  (1 << TWINT) | (1 << TWEN) | (1 << TWEA) : // ACK the byte
	  (1 << TWINT) | (1 << TWEN); // NACK the final byte

	WAIT_FOR_TWI();

	// Confirm the receive status
	if ((TWSR & TWSR_MASK) != expected_status) {
	  *err = TWI_INTERNAL_ERROR;
	  return;
	}

	data[i] = GET_TWDR(); // grab the byte
  }
}
