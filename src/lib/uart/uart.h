#ifndef UART_H
#define UART_H

#include <inttypes.h>
#include <avr/io.h>

// Simple UART communications.
//
// Mainly designed for sending debug information back to the computer
// and not as USART receiver.  
// 
// The tx functions block until all data is sent.

// Initialize the UART
typedef enum {
  ENABLE_RX = 0x01,
  ENABLE_TX = 0x02,
  ENABLE_RX_INTERRUPT = 0x04,
  ENABLE_TX_INTERRUPT = 0x08,
} UART_flags;

void uart_init(uint32_t baud, UART_flags flags);

// disables all UART functions
void uart_disable(void);

// Writes a buffer of 8-bit data
void uart_bytes(const uint8_t* data, uint8_t len);

// Writes a single byte
void uart_byte(uint8_t byte);

// Reads a single byte, if available.  Returns 1 if something
// was read, 0 otherwise
static inline uint8_t read_no_block(uint8_t* byte) {
  if (!(UCSR0A & (1<<RXC0))) {
    return 0;
  }
  *byte = UDR0;
  return 1;
}

// Waits for UART byte to be sent.  Call this before going into
// low power mode
static inline void uart_wait() {
  while (!(UCSR0A & (1 << UDRE0)));
}

// Writes a null-terminated string
void uart_str(const char* str);

// Writes a pascal string
static inline void uart_pstr(const uint8_t* pstr) {
  uart_bytes((uint8_t*)(pstr + 1), *pstr);
}

// convienence functions
static inline void uart_newln() {
  uart_str("\r\n");
}

static inline void uart_strln(const char* str) {
  uart_str(str);
  uart_newln();
}

static inline void uart_pstrln(const uint8_t* pstr) {
  uart_pstr(pstr);
  uart_newln();
}

#endif
