#include "uart.h"

void uart_init(uint32_t baud, UART_flags flags) {
  // Set baud rate
  UCSR0A &= ~(1 << U2X0);
  const uint16_t ubbr = F_CPU / (16 * baud) - 1;
  UBRR0H = ubbr >> 8;
  UBRR0L = ubbr;

  // 8 bits, no parity, one stop bit
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

  if (flags & ENABLE_RX) {
    UCSR0B |= (1 << RXEN0);
  }

  if (flags & ENABLE_RX_INTERRUPT) {
    UCSR0B |= (1 << RXCIE0);
  }

  if (flags & ENABLE_TX) {
    UCSR0B |= (1 << TXEN0);
  }

  if (flags & ENABLE_TX_INTERRUPT) {
    UCSR0B |= (1 << TXCIE0);
  }
}

void uart_disable(void) {
  UCSR0B = 0x00;  // disable everything
  // Reset Baud to default (not sure if it matters)
  UBRR0H = 0x00;
  UBRR0L = 0x00;
}

void uart_byte(uint8_t byte) {
  uart_wait();
  // load data into transmit register
  UDR0 = byte;
}

void uart_bytes(const uint8_t* data, uint8_t length) {
  for (; length > 0; --length, ++data) {
    uart_byte(*data);
  }
}

void uart_str(const char* str) {
  for (; *str; ++str) {
    uart_byte(*str);
  }
}
