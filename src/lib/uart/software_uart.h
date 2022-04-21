#ifndef SOFTWARE_UART_H
#define SOFTWARE_UART_H
// Software uart
//
// Modest requirements for the MVP version
//
// - 9600 baud only
// - Assume 8 or 16Mhz CPU clock
// - Assume 8N1
// - Assume Idle HIGH
// - Recv Only
// - INT0 Only
//
// These can be expanded one at a time after some reliability/confidence
// is established but these constraints will initially help simplify
// issue invedstigation with fewer free variables to contend with.
//
// Usage:
//
// void main(void) {
//  software_uart_init(0);
//  sei();  // enable interrupts
//  ...
// }

#include <inttypes.h>

// This determines how long you can wait without polling.
// if the buffer fills up then characters will be lost
// must be a power of 2
#ifndef SOFTWARE_UART_BUFFER_SIZE
#define SOFTWARE_UART_BUFFER_SIZE 8
#endif

#if SOFTWARE_UART_BUFFER_SIZE > 128
#error maximum SOFTWARE_UART_BUFFER_SIZE is 128
#endif

typedef enum {
  SERIAL_OK = 0,
  SERIAL_ERROR_BAD_START_BIT = 1,
  SERIAL_ERROR_BIT_TOO_NARROW = 2,
  SERIAL_ERROR_TOO_MANY_BITS = 3,
  SERIAL_ERROR_BAD_STOP_BIT = 4,
  SERIAL_ERROR_BUFFER_FULL = 5,
} SerialError;

void software_uart_init(void (*byte_received)(void));

// returns last error.  The error is not necessarly the latest byte as sucessful
// transfers do not touch this. Calling this clears it.
//
// Optional: byte_received can be set to a callback that will be invoked in the interrupt
// handler when a byte is received.  Note that such a callback should act like it's in an
// interrupt handler - e.g. do what needs to be done quickly and return.
SerialError software_uart_last_error(void);

// non blocking read.  Returns 1 if c was filled in.
uint8_t software_uart_read(uint8_t* c);

// turn on and off serail uart reception.  Note that enable is only needed
// if disabled was called since init also enables.
void software_uart_disable(void);
void software_uart_enable(void);

#endif
