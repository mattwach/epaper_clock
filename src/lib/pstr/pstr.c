#include "pstr.h"

#include <inttypes.h>

uint8_t pstr_buffer[12];

// C template.  The code between each conversion is identical
// but the value type is different, thus the final code will
// compile somewhat differently between the cases on an 8-bit
// AVR.
//
// Note that max_length does not need to account for the length
// prefix because "buff + 1" actually makes two slots
// available.
//
// How the function below works:
//   - It's easiest to build the string backwards
//   - We simply % 10 to get the lowerst digit then /= 10 
//     for the next one.
#define UINT_TO_BUFF(val, buff, max_length) \
    buff += max_length; \
    uint8_t* ret = buff; \
    if (val == 0) { \
        *(ret--) = '0'; \
    } else { \
        for (; val != 0; val /= 10) { \
            *(ret--) = '0' + (val % 10); \
        } \
    } \
    *ret = buff - ret; \
    return ret;

// How the function below works:
//   - It's easiest to build the string backwards
//   - We simply % 10 to get the lowerst digit then /= 10 
//     for the next one.
//   - negatives need to be taken into account and the
//     division needs to be unsigned to properly handle
//     the extreme negative signed case (in which there is
//     to signed equivilent.  e.g. int8_t is -128 to 127)
#define INT_TO_BUFF(utype, val, buff, max_length) \
    uint8_t is_negative = val < 0; \
    utype uval = val; \
    if (is_negative) { \
        uval = -val; \
    } \
    buff += max_length; \
    uint8_t* ret = buff; \
    if (uval == 0) { \
        *(ret--) = '0'; \
    } else { \
        for (; uval != 0; uval /= 10) { \
            *(ret--) = '0' + (uval % 10); \
        } \
    } \
    if (is_negative) { \
        *(ret--) = '-'; \
    } \
    *ret = buff - ret; \
    return ret;

// Converts a unsigned to hex value
//
// How it works:
//  - Because the hex is prefixed with zeros, the length is always
//    max_length
//  - Then we proceed to the end of the string and work backwards
//    each digit low digit is calculated with & 0xF plus some
//    character addition to get it to ASCII
//  - When done, buff will be back where it started, so it's just
//    returned.
#define HEX_TO_BUFF(val, buff, max_length) \
    uint8_t len = max_length; \
    *buff = max_length; \
    buff += max_length; \
    for (; len > 0; --len, val >>= 4) { \
        uint8_t digit = (uint8_t)(val & 0xf); \
        *(buff--) = digit < 10 ? '0' + digit : '7' + digit; \
    } \
    return buff;

const uint8_t* u8_to_ps_buff(uint8_t val, uint8_t* buff) {
    // Max buffer is 3 (255)
    UINT_TO_BUFF(val, buff, 3)
}

const uint8_t* u16_to_ps_buff(uint16_t val, uint8_t* buff) {
    // Max buffer is 5 (65536)
    UINT_TO_BUFF(val, buff, 5)
}

const uint8_t* u32_to_ps_buff(uint32_t val, uint8_t* buff) {
    // Max buffer is 10 (4294967295)
    UINT_TO_BUFF(val, buff, 10)
}

const uint8_t* i8_to_ps_buff(int8_t val, uint8_t* buff) {
    // Max buffer is 4 (-128)
    INT_TO_BUFF(uint8_t, val, buff, 4)
}

const uint8_t* i16_to_ps_buff(int16_t val, uint8_t* buff) {
    // Max buffer is 6 (-32768)
    INT_TO_BUFF(uint16_t, val, buff, 6)
}

const uint8_t* i32_to_ps_buff(int32_t val, uint8_t* buff) {
    // Max buffer is 11 (-2147483648)
    INT_TO_BUFF(uint32_t, val, buff, 11)
}

const uint8_t* u8_to_pshex_buff(uint8_t val, uint8_t* buff) {
    HEX_TO_BUFF(val, buff, 2)
}

const uint8_t* u16_to_pshex_buff(uint16_t val, uint8_t* buff) {
    HEX_TO_BUFF(val, buff, 4)
}

const uint8_t* u32_to_pshex_buff(uint32_t val, uint8_t* buff) {
    HEX_TO_BUFF(val, buff, 8)
}

const uint8_t* u8_to_psbinary_buff(uint8_t val, uint8_t gap, uint8_t* buff) {
    *buff = gap ? 9 : 8;
    buff += *buff;
    uint8_t idx = 0;
    for (; idx < 4; ++idx, val >>= 1) {
        *(buff--) = '0' + (val & 1);
    }
    if (gap) {
        *(buff--) = ' ';
    }
    for (idx = 0; idx < 4; ++idx, val >>= 1) {
        *(buff--) = '0' + (val & 1);
    }
    return buff;
}

uint8_t* cs_to_ps_cpy(uint8_t* ps, const char* cs) {
    uint8_t* ps_out = ps;
    while (*cs) {
        *(++ps_out) = (uint8_t)(*(cs++));
    }
    *ps = ps_out - ps; // calc length
    return ps;
}

uint8_t* cs_to_ps_cat(uint8_t* ps, const char* cs) {
    uint8_t* ps_end = ps + *ps;
    while (*cs) {
        *(++ps_end) = (uint8_t)(*(cs++));
    }
    *ps = ps_end - ps; // calc length
    return ps;
}

uint8_t* cs_to_ps(char* cs) {
    uint8_t* end = (uint8_t*)cs;
    for (; *end; ++end);
    const uint8_t length = end - (uint8_t*)cs;
    for (; end > (uint8_t*)cs; --end) {
        *end = *(end - 1);
    }
    *end = length;
    return end;
}

uint8_t pstrequal(const uint8_t* a, const uint8_t* b) {
    if (*a != *b) { return 0; }
    uint8_t len = *a;
    ++a;
    ++b;
    for (; len > 0; --len, ++a, ++b) {
        if (*a != *b) {
            return 0;
        }
    }
    return 1;
}

int8_t pstrcmp(const uint8_t* a, const uint8_t* b) {
    const uint8_t alen = *a;
    const uint8_t blen = *b;
    uint8_t cmplen = alen < blen ? alen : blen;
    ++a;
    ++b;
    for (; cmplen > 0; --cmplen, ++a, ++b) {
        if (*a < *b) {
            return -1;
        }
        if (*a > *b) {
            return 1;
        }
    }
    if (alen < blen) {
        return -1; 
    }
    if (blen < alen) {
        return 1;
    }
    return 0;
}

uint8_t* pstrcpy(uint8_t* dest, const uint8_t* src) {
    uint8_t* ret = dest;
    uint8_t len = *src;
    *(dest++) = *(src++);
    for (; len > 0; --len, ++dest, ++src) {
        *dest = *src;
    }
    return ret;
}

uint8_t* pstrcat(uint8_t* dest, const uint8_t* src) {
    uint8_t* ret = dest;
    const uint8_t destlen = *dest;
    uint8_t len = *src;
    *dest += *src;
    dest += destlen + 1;
    ++src;
    for (; len > 0; --len, ++dest, ++src) {
        *dest = *src;
    }
    return ret;
}

uint8_t* pstrappend(uint8_t* dest, char c) {
    uint8_t* ret = dest;
    ++(*dest);
    *(dest + *dest) = c;
    return ret; 
}
