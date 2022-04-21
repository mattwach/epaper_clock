
#include "pstr.h"

#include <test/unit_test.h>
#include <string.h>

#define TO_PS_HELPER(fn, val, conv, max_length) \
    uint8_t length = strlen(conv); \
    const uint8_t* ptr = fn(val); \
    assert_int_equal(length, *ptr); \
    assert_buff_equal((uint8_t*)conv, ptr + 1, length); \
    assert_int_equal(max_length - length, ptr - pstr_buffer); \
    assert_int_lt(-1, ptr - pstr_buffer);

void test_pstr(void) {
  pstr(empty, "");
  assert_buff_equal(((uint8_t[]){0}), empty, 1);

  pstr(x, "X");
  assert_buff_equal(((uint8_t[]){1, 'X'}), x, 2);

  pstr(hello, "Hello");
  assert_buff_equal(
      ((uint8_t[]){5, 'H', 'e', 'l', 'l', 'o'}),
      hello,
      6);
}

void u8_to_ps_helper(uint8_t val, const char* conv) {
    TO_PS_HELPER(u8_to_ps, val, conv, 3);
}

void test_u8_to_ps(void) {
    u8_to_ps_helper(0, "0");
    u8_to_ps_helper(1, "1");
    u8_to_ps_helper(9, "9");
    u8_to_ps_helper(10, "10");
    u8_to_ps_helper(99, "99");
    u8_to_ps_helper(100, "100");
    u8_to_ps_helper(254, "254");
}

void u16_to_ps_helper(uint16_t val, const char* conv) {
    TO_PS_HELPER(u16_to_ps, val, conv, 5);
}

void test_u16_to_ps(void) {
    u16_to_ps_helper(0, "0");
    u16_to_ps_helper(1, "1");
    u16_to_ps_helper(9, "9");
    u16_to_ps_helper(10, "10");
    u16_to_ps_helper(99, "99");
    u16_to_ps_helper(100, "100");
    u16_to_ps_helper(999, "999");
    u16_to_ps_helper(1000, "1000");
    u16_to_ps_helper(9999, "9999");
    u16_to_ps_helper(10000, "10000");
    u16_to_ps_helper(65535, "65535");
}

void u32_to_ps_helper(uint32_t val, const char* conv) {
    TO_PS_HELPER(u32_to_ps, val, conv, 10);
}

void test_u32_to_ps(void) {
    u32_to_ps_helper(0, "0");
    u32_to_ps_helper(1, "1");
    u32_to_ps_helper(9, "9");
    u32_to_ps_helper(10, "10");
    u32_to_ps_helper(99, "99");
    u32_to_ps_helper(100, "100");
    u32_to_ps_helper(999, "999");
    u32_to_ps_helper(1000, "1000");
    u32_to_ps_helper(9999, "9999");
    u32_to_ps_helper(10000, "10000");
    u32_to_ps_helper(99999, "99999");
    u32_to_ps_helper(100000, "100000");
    u32_to_ps_helper(999999, "999999");
    u32_to_ps_helper(1000000, "1000000");
    u32_to_ps_helper(9999999, "9999999");
    u32_to_ps_helper(10000000, "10000000");
    u32_to_ps_helper(99999999, "99999999");
    u32_to_ps_helper(100000000, "100000000");
    u32_to_ps_helper(999999999, "999999999");
    u32_to_ps_helper(1000000000, "1000000000");
    u32_to_ps_helper(4294967295, "4294967295");
}

void i8_to_ps_helper(uint8_t val, const char* conv) {
    TO_PS_HELPER(i8_to_ps, val, conv, 4);
}

void test_i8_to_ps(void) {
    i8_to_ps_helper(-128, "-128");
    i8_to_ps_helper(-100, "-100");
    i8_to_ps_helper(-99, "-99");
    i8_to_ps_helper(-10, "-10");
    i8_to_ps_helper(-9, "-9");
    i8_to_ps_helper(-1, "-1");
    i8_to_ps_helper(0, "0");
    i8_to_ps_helper(1, "1");
    i8_to_ps_helper(9, "9");
    i8_to_ps_helper(10, "10");
    i8_to_ps_helper(99, "99");
    i8_to_ps_helper(100, "100");
    i8_to_ps_helper(127, "127");
}

void i16_to_ps_helper(uint16_t val, const char* conv) {
    TO_PS_HELPER(i16_to_ps, val, conv, 6);
}

void test_i16_to_ps(void) {
    i16_to_ps_helper(-32768, "-32768");
    i16_to_ps_helper(-10000, "-10000");
    i16_to_ps_helper(-9999, "-9999");
    i16_to_ps_helper(-1000, "-1000");
    i16_to_ps_helper(-999, "-999");
    i16_to_ps_helper(-100, "-100");
    i16_to_ps_helper(-99, "-99");
    i16_to_ps_helper(-10, "-10");
    i16_to_ps_helper(-9, "-9");
    i16_to_ps_helper(-1, "-1");
    i16_to_ps_helper(0, "0");
    i16_to_ps_helper(1, "1");
    i16_to_ps_helper(9, "9");
    i16_to_ps_helper(10, "10");
    i16_to_ps_helper(99, "99");
    i16_to_ps_helper(100, "100");
    i16_to_ps_helper(999, "999");
    i16_to_ps_helper(1000, "1000");
    i16_to_ps_helper(9999, "9999");
    i16_to_ps_helper(10000, "10000");
    i16_to_ps_helper(32767, "32767");
}

void i32_to_ps_helper(uint32_t val, const char* conv) {
    TO_PS_HELPER(i32_to_ps, val, conv, 11);
}

void test_i32_to_ps(void) {
    i32_to_ps_helper(-2147483648, "-2147483648");
    i32_to_ps_helper(-1000000000, "-1000000000");
    i32_to_ps_helper(-999999999, "-999999999");
    i32_to_ps_helper(-100000000, "-100000000");
    i32_to_ps_helper(-99999999, "-99999999");
    i32_to_ps_helper(-10000000, "-10000000");
    i32_to_ps_helper(-9999999, "-9999999");
    i32_to_ps_helper(-1000000, "-1000000");
    i32_to_ps_helper(-999999, "-999999");
    i32_to_ps_helper(-100000, "-100000");
    i32_to_ps_helper(-99999, "-99999");
    i32_to_ps_helper(-10000, "-10000");
    i32_to_ps_helper(-9999, "-9999");
    i32_to_ps_helper(-1000, "-1000");
    i32_to_ps_helper(-999, "-999");
    i32_to_ps_helper(-100, "-100");
    i32_to_ps_helper(-99, "-99");
    i32_to_ps_helper(-10, "-10");
    i32_to_ps_helper(-9, "-9");
    i32_to_ps_helper(-1, "-1");
    i32_to_ps_helper(0, "0");
    i32_to_ps_helper(1, "1");
    i32_to_ps_helper(9, "9");
    i32_to_ps_helper(10, "10");
    i32_to_ps_helper(99, "99");
    i32_to_ps_helper(100, "100");
    i32_to_ps_helper(999, "999");
    i32_to_ps_helper(1000, "1000");
    i32_to_ps_helper(9999, "9999");
    i32_to_ps_helper(10000, "10000");
    i32_to_ps_helper(99999, "99999");
    i32_to_ps_helper(100000, "100000");
    i32_to_ps_helper(999999, "999999");
    i32_to_ps_helper(1000000, "1000000");
    i32_to_ps_helper(9999999, "9999999");
    i32_to_ps_helper(10000000, "10000000");
    i32_to_ps_helper(99999999, "99999999");
    i32_to_ps_helper(100000000, "100000000");
    i32_to_ps_helper(999999999, "999999999");
    i32_to_ps_helper(1000000000, "1000000000");
    i32_to_ps_helper(2147483647, "2147483647");
}

void u8_to_pshex_helper(uint8_t val, const char* conv) {
    TO_PS_HELPER(u8_to_pshex, val, conv, 2);
}

void test_u8_to_pshex(void) {
    u8_to_pshex_helper(0x00, "00");
    u8_to_pshex_helper(0x01, "01");
    u8_to_pshex_helper(0x09, "09");
    u8_to_pshex_helper(0x0a, "0A");
    u8_to_pshex_helper(0x0f, "0F");
    u8_to_pshex_helper(0x10, "10");
    u8_to_pshex_helper(0x98, "98");
    u8_to_pshex_helper(0xa9, "A9");
    u8_to_pshex_helper(0xfa, "FA");
    u8_to_pshex_helper(0xff, "FF");
}

void u16_to_pshex_helper(uint16_t val, const char* conv) {
    TO_PS_HELPER(u16_to_pshex, val, conv, 4);
}

void test_u16_to_pshex(void) {
    u16_to_pshex_helper(0x0000, "0000");
    u16_to_pshex_helper(0x0001, "0001");
    u16_to_pshex_helper(0x0009, "0009");
    u16_to_pshex_helper(0x000a, "000A");
    u16_to_pshex_helper(0x000f, "000F");
    u16_to_pshex_helper(0x0010, "0010");
    u16_to_pshex_helper(0x0098, "0098");
    u16_to_pshex_helper(0x00a9, "00A9");
    u16_to_pshex_helper(0x00fa, "00FA");
    u16_to_pshex_helper(0x00ff, "00FF");
    u16_to_pshex_helper(0x0123, "0123");
    u16_to_pshex_helper(0x0934, "0934");
    u16_to_pshex_helper(0x0a45, "0A45");
    u16_to_pshex_helper(0x0f56, "0F56");
    u16_to_pshex_helper(0x1678, "1678");
    u16_to_pshex_helper(0x9789, "9789");
    u16_to_pshex_helper(0xa789, "A789");
    u16_to_pshex_helper(0xf89a, "F89A");
    u16_to_pshex_helper(0xffff, "FFFF");
}

void u32_to_pshex_helper(uint32_t val, const char* conv) {
    TO_PS_HELPER(u32_to_pshex, val, conv, 8);
}

void test_u32_to_pshex(void) {
    u32_to_pshex_helper(0x00000000, "00000000");
    u32_to_pshex_helper(0x00000001, "00000001");
    u32_to_pshex_helper(0x00000009, "00000009");
    u32_to_pshex_helper(0x0000000a, "0000000A");
    u32_to_pshex_helper(0x0000000f, "0000000F");
    u32_to_pshex_helper(0x00000010, "00000010");
    u32_to_pshex_helper(0x00000098, "00000098");
    u32_to_pshex_helper(0x000000a9, "000000A9");
    u32_to_pshex_helper(0x000000fa, "000000FA");
    u32_to_pshex_helper(0x000000ff, "000000FF");
    u32_to_pshex_helper(0x00000123, "00000123");
    u32_to_pshex_helper(0x00000934, "00000934");
    u32_to_pshex_helper(0x00000a45, "00000A45");
    u32_to_pshex_helper(0x00000f56, "00000F56");
    u32_to_pshex_helper(0x00001678, "00001678");
    u32_to_pshex_helper(0x00009789, "00009789");
    u32_to_pshex_helper(0x0000a789, "0000A789");
    u32_to_pshex_helper(0x0000f89a, "0000F89A");
    u32_to_pshex_helper(0x0000ffff, "0000FFFF");
    u32_to_pshex_helper(0x00012345, "00012345");
    u32_to_pshex_helper(0x000a2345, "000A2345");
    u32_to_pshex_helper(0x000f2345, "000F2345");
    u32_to_pshex_helper(0x000fffff, "000FFFFF");
    u32_to_pshex_helper(0x00123456, "00123456");
    u32_to_pshex_helper(0x00923456, "00923456");
    u32_to_pshex_helper(0x00a23456, "00A23456");
    u32_to_pshex_helper(0x00f23456, "00F23456");
    u32_to_pshex_helper(0x00ffffff, "00FFFFFF");
    u32_to_pshex_helper(0x01234567, "01234567");
    u32_to_pshex_helper(0x09234567, "09234567");
    u32_to_pshex_helper(0x0a234567, "0A234567");
    u32_to_pshex_helper(0x0f234567, "0F234567");
    u32_to_pshex_helper(0x0fffffff, "0FFFFFFF");
    u32_to_pshex_helper(0x12345678, "12345678");
    u32_to_pshex_helper(0x92345678, "92345678");
    u32_to_pshex_helper(0xa2345678, "A2345678");
    u32_to_pshex_helper(0xf2345678, "F2345678");
    u32_to_pshex_helper(0xFFFFFFFF, "FFFFFFFF");
}

void u8_to_psbinary_helper(uint8_t val, uint8_t gap, const char* conv) {
    uint8_t length = strlen(conv);
    const uint8_t* ptr = u8_to_psbinary(val, gap);
    assert_int_equal(gap ? 9 : 8, *ptr); \
    assert_buff_equal((uint8_t*)conv, ptr + 1, length); \
    assert_int_equal(0, ptr - pstr_buffer); \
}

void test_u8_to_psbinary(void) {
    u8_to_psbinary_helper(0x00, 0, "00000000");
    u8_to_psbinary_helper(0x01, 0, "00000001");
    u8_to_psbinary_helper(0x02, 0, "00000010");
    u8_to_psbinary_helper(0x03, 0, "00000011");
    u8_to_psbinary_helper(0x04, 0, "00000100");
    u8_to_psbinary_helper(0x07, 0, "00000111");
    u8_to_psbinary_helper(0x08, 0, "00001000");
    u8_to_psbinary_helper(0x0f, 0, "00001111");
    u8_to_psbinary_helper(0x10, 0, "00010000");
    u8_to_psbinary_helper(0x1f, 0, "00011111");
    u8_to_psbinary_helper(0x20, 0, "00100000");
    u8_to_psbinary_helper(0x3f, 0, "00111111");
    u8_to_psbinary_helper(0x40, 0, "01000000");
    u8_to_psbinary_helper(0x7f, 0, "01111111");
    u8_to_psbinary_helper(0x80, 0, "10000000");
    u8_to_psbinary_helper(0xff, 0, "11111111");

    u8_to_psbinary_helper(0x00, 1, "0000 0000");
    u8_to_psbinary_helper(0x01, 1, "0000 0001");
    u8_to_psbinary_helper(0x02, 1, "0000 0010");
    u8_to_psbinary_helper(0x03, 1, "0000 0011");
    u8_to_psbinary_helper(0x04, 1, "0000 0100");
    u8_to_psbinary_helper(0x07, 1, "0000 0111");
    u8_to_psbinary_helper(0x08, 1, "0000 1000");
    u8_to_psbinary_helper(0x0f, 1, "0000 1111");
    u8_to_psbinary_helper(0x10, 1, "0001 0000");
    u8_to_psbinary_helper(0x1f, 1, "0001 1111");
    u8_to_psbinary_helper(0x20, 1, "0010 0000");
    u8_to_psbinary_helper(0x3f, 1, "0011 1111");
    u8_to_psbinary_helper(0x40, 1, "0100 0000");
    u8_to_psbinary_helper(0x7f, 1, "0111 1111");
    u8_to_psbinary_helper(0x80, 1, "1000 0000");
    u8_to_psbinary_helper(0xff, 1, "1111 1111");
}

void test_ps_to_cs(void) {
    uint8_t pstr[8];
    
    pstr[0] = 0;
    assert_str_equal("", ps_to_cs(pstr));

    pstr[0] = 1;
    pstr[1] = 'X';
    assert_str_equal("X", ps_to_cs(pstr));

    pstr[0] = 5;
    pstr[1] = 'H';
    pstr[2] = 'e';
    pstr[3] = 'l';
    pstr[4] = 'l';
    pstr[5] = 'o';
    assert_str_equal("Hello", ps_to_cs(pstr));
}

void test_cs_to_ps_cpy(void) {
    uint8_t pstr[8];

    cs_to_ps_cpy(pstr, "");
    assert_int_equal(0, pstr[0]);

    cs_to_ps_cpy(pstr, "X");
    assert_buff_equal(((uint8_t[]){1, 'X'}), pstr, 2);

    cs_to_ps_cpy(pstr, "Hello");
    assert_buff_equal(
        ((uint8_t[]){5, 'H', 'e', 'l', 'l', 'o'}),
        pstr,
        6);
}

void test_cs_to_ps_cat(void) {
    uint8_t pstr[8];

    pstr[0] = '\0';
    cs_to_ps_cat(pstr, "");
    assert_int_equal(0, pstr[0]);

    pstr[0] = '\0';
    cs_to_ps_cat(pstr, "X");
    assert_buff_equal(((uint8_t[]){1, 'X'}), pstr, 2);

    pstr[0] = '\0';
    cs_to_ps_cat(pstr, "Hello");
    assert_buff_equal(
        ((uint8_t[]){5, 'H', 'e', 'l', 'l', 'o'}),
        pstr,
        6);

    cs_to_ps_cpy(pstr, "Hi");
    cs_to_ps_cat(pstr, "");
    assert_buff_equal(
        ((uint8_t[]){2, 'H', 'i'}),
        pstr,
        3);

    cs_to_ps_cpy(pstr, "Hi");
    cs_to_ps_cat(pstr, "X");
    assert_buff_equal(
        ((uint8_t[]){3, 'H', 'i', 'X'}),
        pstr,
        4);

    cs_to_ps_cpy(pstr, "Hi");
    cs_to_ps_cat(pstr, "Hello");
    assert_buff_equal(
        ((uint8_t[]){7, 'H', 'i', 'H', 'e', 'l', 'l', 'o'}),
        pstr,
        8);
}

void test_cs_to_ps(void) {
    char cstr[8];

    cstr[0] = '\0';
    assert_buff_equal(
        ((uint8_t[]){0}),
        cs_to_ps(cstr),
        1);

    strcpy(cstr, "X");
    assert_buff_equal(
        ((uint8_t[]){1, 'X'}),
        cs_to_ps(cstr),
        2);

    strcpy(cstr, "Hello");
    assert_buff_equal(
        ((uint8_t[]){5, 'H', 'e', 'l', 'l', 'o'}),
        cs_to_ps(cstr),
        6);
}

void test_pstrcmp(void) {
    uint8_t a[8];
    uint8_t b[8];

    a[0] = b[0] = 0;
    assert_int_equal(0, pstrcmp(a, b));

    cs_to_ps_cpy(a, "x");
    b[0] = 0;
    assert_int_equal(1, pstrcmp(a, b));

    a[0] = 0;
    cs_to_ps_cpy(b, "x");
    assert_int_equal(-1, pstrcmp(a, b));

    cs_to_ps_cpy(a, "x");
    cs_to_ps_cpy(b, "x");
    assert_int_equal(0, pstrcmp(a, b));

    cs_to_ps_cpy(a, "x");
    cs_to_ps_cpy(b, "y");
    assert_int_equal(-1, pstrcmp(a, b));

    cs_to_ps_cpy(a, "y");
    cs_to_ps_cpy(b, "x");
    assert_int_equal(1, pstrcmp(a, b));

    cs_to_ps_cpy(a, "xy");
    cs_to_ps_cpy(b, "x");
    assert_int_equal(1, pstrcmp(a, b));

    cs_to_ps_cpy(a, "x");
    cs_to_ps_cpy(b, "xy");
    assert_int_equal(-1, pstrcmp(a, b));

    cs_to_ps_cpy(a, "xy");
    cs_to_ps_cpy(b, "xy");
    assert_int_equal(0, pstrcmp(a, b));
}

void test_pstrequal(void) {
    uint8_t a[8];
    uint8_t b[8];

    a[0] = b[0] = 0;
    assert_int_equal(1, pstrequal(a, b));

    cs_to_ps_cpy(a, "x");
    b[0] = 0;
    assert_int_equal(0, pstrequal(a, b));

    a[0] = 0;
    cs_to_ps_cpy(b, "x");
    assert_int_equal(0, pstrequal(a, b));

    cs_to_ps_cpy(a, "x");
    cs_to_ps_cpy(b, "x");
    assert_int_equal(1, pstrequal(a, b));

    cs_to_ps_cpy(a, "xy");
    cs_to_ps_cpy(b, "x");
    assert_int_equal(0, pstrequal(a, b));

    cs_to_ps_cpy(a, "x");
    cs_to_ps_cpy(b, "xy");
    assert_int_equal(0, pstrequal(a, b));

    cs_to_ps_cpy(a, "xy");
    cs_to_ps_cpy(b, "xy");
    assert_int_equal(1, pstrequal(a, b));
}

void test_pstrcpy(void) {
    uint8_t a[8];
    uint8_t b[8];

    b[0] = '\0';
    assert_int_equal(0, a - pstrcpy(a, b));
    assert_buff_equal(
        ((uint8_t[]){0}),
        a,
        1);

    cs_to_ps_cpy(b, "X");
    assert_int_equal(0, a - pstrcpy(a, b));
    assert_buff_equal(
        ((uint8_t[]){1, 'X'}),
        a,
        2);

    cs_to_ps_cpy(b, "Hello");
    assert_int_equal(0, a - pstrcpy(a, b));
    assert_buff_equal(
        ((uint8_t[]){5, 'H', 'e', 'l', 'l', 'o'}),
        a,
        6);
}

void test_pstrappend(void) {
    uint8_t a[8];

    a[0] = '\0';
    assert_int_equal(0, a - pstrappend(a, 'a'));
    assert_buff_equal(
        ((uint8_t[]){1, 'a'}),
        a,
        2);

    assert_int_equal(0, a - pstrappend(a, 'b'));
    assert_buff_equal(
        ((uint8_t[]){2, 'a', 'b'}),
        a,
        3);
}

void test_pstrcat(void) {
    uint8_t a[8];
    uint8_t b[8];

    a[0] = '\0';
    b[0] = '\0';
    assert_int_equal(0, a - pstrcat(a, b));
    assert_buff_equal(
        ((uint8_t[]){0}),
        a,
        1);

    a[0] = '\0';
    cs_to_ps_cpy(b, "X");
    assert_int_equal(0, a - pstrcat(a, b));
    assert_buff_equal(
        ((uint8_t[]){1, 'X'}),
        a,
        2);

    a[0] = '\0';
    cs_to_ps_cpy(b, "Hello");
    assert_int_equal(0, a - pstrcat(a, b));
    assert_buff_equal(
        ((uint8_t[]){5, 'H', 'e', 'l', 'l', 'o'}),
        a,
        6);

    cs_to_ps_cpy(a, "X");
    b[0] = '\0';
    assert_int_equal(0, a - pstrcat(a, b));
    assert_buff_equal(
        ((uint8_t[]){1, 'X'}),
        a,
        2);

    cs_to_ps_cpy(a, "Hello");
    b[0] = '\0';
    assert_int_equal(0, a - pstrcat(a, b));
    assert_buff_equal(
        ((uint8_t[]){5, 'H', 'e', 'l', 'l', 'o'}),
        a,
        6);

    cs_to_ps_cpy(a, "Hello");
    cs_to_ps_cpy(b, "X");
    assert_int_equal(0, a - pstrcat(a, b));
    assert_buff_equal(
        ((uint8_t[]){6, 'H', 'e', 'l', 'l', 'o', 'X'}),
        a,
        7);

    cs_to_ps_cpy(a, "X");
    cs_to_ps_cpy(b, "Hello");
    assert_int_equal(0, a - pstrcat(a, b));
    assert_buff_equal(
        ((uint8_t[]){6, 'X', 'H', 'e', 'l', 'l', 'o'}),
        a,
        7);

    cs_to_ps_cpy(a, "He");
    cs_to_ps_cpy(b, "llo");
    assert_int_equal(0, a - pstrcat(a, b));
    assert_buff_equal(
        ((uint8_t[]){5, 'H', 'e', 'l', 'l', 'o'}),
        a,
        6);
}

int main(int argc, char** argv) {
    test(test_pstr);

    test(test_u8_to_ps);
    test(test_u16_to_ps);
    test(test_u32_to_ps);

    test(test_i8_to_ps);
    test(test_i16_to_ps);
    test(test_i32_to_ps);

    test(test_u8_to_pshex);
    test(test_u16_to_pshex);
    test(test_u32_to_pshex);

    test(test_u8_to_psbinary);

    test(test_ps_to_cs);
    test(test_cs_to_ps_cpy);
    test(test_cs_to_ps_cat);
    test(test_cs_to_ps);

    test(test_pstrequal);
    test(test_pstrcmp);

    test(test_pstrcpy);
    test(test_pstrcat);
    test(test_pstrappend);

    return 0;
}