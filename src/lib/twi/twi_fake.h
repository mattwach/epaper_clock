#ifndef LIB_TWI_FAKE_H
#define LIB_TWI_FAKE_H

#include "twi.h"
#include <inttypes.h>

// Fake suppliment for TWI unit tests.
// The basic setup:
//
// The unit test includes this file
// You compile and link twi_fake.o into the test binary.
// Now the code under test will call the fake methods which
// will populate the log.

// Log codes
#define TWI_INIT           0xA0
#define TWI_REINIT         0xA1
#define TWI_STOP           0xA2
#define TWI_START_WRITE    0xA3
#define TWI_WRITE_NO_STOP  0xA4
#define TWI_READ_NO_STOP   0xA5

extern uint8_t twi_log[];
extern uint16_t twi_logidx;
extern error_t twi_err[];
extern uint16_t twi_erridx;
extern uint16_t twi_errcnt;
extern uint8_t* twi_read_data;
extern uint16_t twi_bytes_read;

static inline void twi_log_reset() {
    twi_logidx = 0;
    twi_erridx = 0;
    twi_errcnt = 0;
    twi_read_data = 0;
    twi_bytes_read = 0;
}

static inline void twi_queue_err(error_t err) {
    twi_err[twi_errcnt++] = err;
}

static inline void twi_set_read_data(uint8_t* read_data) {
  twi_read_data = read_data;
  twi_bytes_read = 0;
}



#endif
