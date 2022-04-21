#ifndef DEBUG_DEBUG_H
#define DEBUG_DEBUG_H

#ifdef DEBUG
#include <uart/uart.h>
#include <pstr/pstr.h>

#define DEBUG_INIT() uart_init(9600, ENABLE_TX)

#define DEBUG_STR(str) uart_strln(str)

#define DEBUG_U8(name, val) \
  uart_str(name); \
  uart_str(" 0x"); \
  uart_pstr(u8_to_pshex(val)); \
  uart_newln()

#define DEBUG_U16(name, val) \
  uart_str(name); \
  uart_str(" 0x"); \
  uart_pstr(u16_to_pshex(val)); \
  uart_newln()

#define DEBUG_U32(name, val) \
  uart_str(name); \
  uart_str(" 0x"); \
  uart_pstr(u32_to_pshex(val)); \
  uart_newln()

#define DEBUG_I32(name, val) \
  uart_str(name); \
  uart_str(" 0x"); \
  uart_pstr(i32_to_ps(val)); \
  uart_newln()

#else

#define DEBUG_INIT()
#define DEBUG_STR(str)
#define DEBUG_U8(name, val)
#define DEBUG_U16(name, val)
#define DEBUG_I32(name, val)
#define DEBUG_U32(name, val)

#endif

#endif

