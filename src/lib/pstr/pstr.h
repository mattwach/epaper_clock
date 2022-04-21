#ifndef PSTR_H
#define PSTR_H
// Pascal string library
//
// C Strings end with a zero: "Hello" -> 0x48 0x65 0x6C 0x6C 0x6F 0x00
// Pascal strings start with length: "Hello" -> 0x05 0x48 0x65 0x6C 0x6C 0x6F
//
// C strings are nice becaue they can be long
// Pascal strings are nice because you don't keep buring cycles figuring
//   out the length.
//
// For slow, memory constrained MCU, Pascal can be a good fit.

#include <inttypes.h>

extern uint8_t pstr_buffer[];

// Note on conversions: They can be bad for performance so try to minimize
// their use.

// Convience macro to create a pstring from a raw "quoted" string.
// Usage: pstr(hi, "Hello")
// Hi now contains the pascal string "hello"
struct pstr_struct {
    uint8_t length;
    char data[];
};
#define pstr(var, str) \
  static const struct pstr_struct __ ## var ## __pstr__ = \
    {sizeof(str) - 1, str}; \
  const uint8_t* var = (const uint8_t*)(&__ ## var ## __pstr__)

// Very fast pstr -> c string conversion.  Needs one spare byte in the
// pstr buffer to make room for the null char
static inline char* ps_to_cs(uint8_t* ps) {
    ps[*ps + 1] = '\0';
    return (char*)(ps + 1);
}

// c string -> pstring conversion.  This is slower because it
// needs to find the end of the c string and copy every character over
uint8_t* cs_to_ps_cpy(uint8_t* ps, const char* cs);
uint8_t* cs_to_ps_cat(uint8_t* ps, const char* cs);
// in place
uint8_t* cs_to_ps(char* cs);

// Returns 1 if two pstrings are equal, 0 otherwise.  More efficienct
// than pstrcmp
uint8_t pstrequal(const uint8_t* a, const uint8_t* b);

// Returns -1 if a < b, 0 if a == b, 1 if a > 1.  Less efficient
// than pstrequal
int8_t pstrcmp(const uint8_t* a, const uint8_t* b);

// Copies a pstring to a new buffer, returns dest
uint8_t* pstrcpy(uint8_t* dest, const uint8_t* src);

// Concats a pstring to a another, returns dest
uint8_t* pstrcat(uint8_t* dest, const uint8_t* src);

// Concats a single character to a pstring
uint8_t* pstrappend(uint8_t* dest, char c);

// Common docs for conversion functions
//
// _buff versions:
//   You pass the value-to-convert and a buffer that
//   is big enough to hold the largest value and a length
//   character.  The function will return a buffer somewhere
//   inside of buff, often different from buff itself (to
//   reduce cycles).
//
// non _buff versions.  A global buffer is passed for you.
// This pointer should be considered very temporary - only
// good until the next call.  Also, don't use this version
// in an interrupt handler

// uint8_t -> pstr.  buff needs 4 bytes minimum
const uint8_t* u8_to_ps_buff(uint8_t val, uint8_t* buff);
#define u8_to_ps(val) u8_to_ps_buff(val, pstr_buffer)

// uint16_t -> pstr.  buff needs 6 bytes minimum
const uint8_t* u16_to_ps_buff(uint16_t val, uint8_t* buff);
#define u16_to_ps(val) u16_to_ps_buff(val, pstr_buffer)

// uint32_t -> pstr.  buff needs 11 bytes minimum
const uint8_t* u32_to_ps_buff(uint32_t val, uint8_t* buff);
#define u32_to_ps(val) u32_to_ps_buff(val, pstr_buffer)

// int8_t -> pstr.  buff needs 5 bytes minimum
const uint8_t* i8_to_ps_buff(int8_t val, uint8_t* buff);
#define i8_to_ps(val) i8_to_ps_buff(val, pstr_buffer)

// int16_t -> pstr.  buff needs 7 bytes minimum
const uint8_t* i16_to_ps_buff(int16_t val, uint8_t* buff);
#define i16_to_ps(val) i16_to_ps_buff(val, pstr_buffer)

// int32_t -> pstr.  buff needs 12 bytes minimum
const uint8_t* i32_to_ps_buff(int32_t val, uint8_t* buff);
#define i32_to_ps(val) i32_to_ps_buff(val, pstr_buffer)

// uint8_t -> pstr hexidecimal.  buff needs 3 bytes minimum
const uint8_t* u8_to_pshex_buff(uint8_t val, uint8_t* buff);
#define u8_to_pshex(val) u8_to_pshex_buff(val, pstr_buffer)

// uint16_t -> pstr hexidecimal.  buff needs 5 bytes minimum
const uint8_t* u16_to_pshex_buff(uint16_t val, uint8_t* buff);
#define u16_to_pshex(val) u16_to_pshex_buff(val, pstr_buffer)

// uint32_t -> pstr hexidecimal.  buff needs 9 bytes minimum
const uint8_t* u32_to_pshex_buff(uint32_t val, uint8_t* buff);
#define u32_to_pshex(val) u32_to_pshex_buff(val, pstr_buffer)

// uint8_t -> pstr binary.  buff needs 10 bytes minimum
// If gap is != 0, then a space will be put in the middle
// e.g. gap=0: FF -> 11111111,  gap=1: FF -> 1111 1111
const uint8_t* u8_to_psbinary_buff(uint8_t val, uint8_t gap, uint8_t* buff);
#define u8_to_psbinary(val, gap) u8_to_psbinary_buff(val, gap, pstr_buffer)


#endif