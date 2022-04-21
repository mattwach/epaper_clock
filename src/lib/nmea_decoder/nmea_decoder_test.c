#include "nmea_decoder.h"

#include <test/unit_test.h>
#include <stdio.h>

const char* gpgga_example =
    "$GPGGA,181908.00,3404.7041778,N,07044.3966270,W,"
    "4,13,1.00,495.144,M,-29.200,M,0.10,0000*72\r\n";

const char* gpgsa_example =
    "$GPGSA,M,3,17,02,30,04,05,10,09,06,31,12,,,1.2,0.8,0.9*35\r\n";

const char* gpgsv_example =
    "$GPGSV,3,1,11,18,87,050,48,22,56,250,49,21,55,122,49,03,40,284,47*78\r\n"
    "$GPGSV,3,2,11,19,25,314,42,26,24,044,42,24,16,118,43,29,15,039,42*7E\r\n"
    "$GPGSV,3,3,11,09,15,107,44,14,11,196,41,07,03,173,*4D\r\n";

const char* gpgll_example =
    "$GPGLL,5107.0013414,N,11402.3279144,W,205412.00,A,A*73\r\n";

const char* gpvtg_example =
    "$GPVTG,172.516,T,155.295,M,0.049,N,0.090,K,D*2B\r\n";

const char* gprmc_example =
    "$GPRMC,144326.00,A,5107.0017737,N,11402.3291611,W,"
    "0.080,323.3,210307,0.0,E,A*20\r\n";

static void encode_str(struct NMEA_decoder* gps, const char* str) {
  const char* orig = str;
  for (; *str; ++str) {
    if (!nmea_encode(gps, *str)) {
      printf("Buffer full while encoding: %s\n", orig);
      break;
    }
  }
}

static void check_parse_alpha(
    struct NMEA_decoder* gps,
    uint8_t index,
    char expected_value,
    NMEA_error expected_error) {
  gps->last_error = 0;
  assert_int_equal(expected_value, nmea_parse_alpha(gps, index));
  assert_int_equal(expected_error, gps->last_error);
}

static void check_parse_u32(
    struct NMEA_decoder* gps,
    uint8_t index,
    uint32_t expected_value,
    NMEA_error expected_error) {
  gps->last_error = 0;
  assert_int_equal(expected_value, nmea_parse_u32(gps, index));
  assert_int_equal(expected_error, gps->last_error);
}

static void check_parse_u16(
    struct NMEA_decoder* gps,
    uint8_t index,
    uint16_t expected_value,
    NMEA_error expected_error) {
  gps->last_error = 0;
  assert_int_equal(expected_value, nmea_parse_u16(gps, index));
  assert_int_equal(expected_error, gps->last_error);
}

static void check_parse_u8(
    struct NMEA_decoder* gps,
    uint8_t index,
    uint8_t expected_value,
    NMEA_error expected_error) {
  gps->last_error = 0;
  assert_int_equal(expected_value, nmea_parse_u8(gps, index));
  assert_int_equal(expected_error, gps->last_error);
}

static void check_parse_float(
    struct NMEA_decoder* gps,
    uint8_t index,
    float expected_value,
    NMEA_error expected_error) {
  gps->last_error = 0;
  assert_float_near_equal(expected_value, nmea_parse_float(gps, index));
  assert_int_equal(expected_error, gps->last_error);
}

static void check_parse_double(
    struct NMEA_decoder* gps,
    uint8_t index,
    double expected_value,
    NMEA_error expected_error) {
  gps->last_error = 0;
  assert_double_near_equal(expected_value, nmea_parse_double(gps, index));
  assert_int_equal(expected_error, gps->last_error);
}

void test_encode(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, "Hello");
  nmea_encode(&gps, 0);

  assert_int_equal(6, gps.buffer_size);
  assert_int_equal(0, gps.ready);
  assert_str_equal("Hello", gps.buffer);

  encode_str(&gps, "Bye\r\n");
  nmea_encode(&gps, 0);

  assert_int_equal(12, gps.buffer_size);
  assert_int_equal(1, gps.ready);
  assert_str_equal("Hello", gps.buffer);
  assert_str_equal("Bye\r\n", gps.buffer + 6);
}


void test_encode_with_offset(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  gps.buffer_head = 253;
  encode_str(&gps, "Hello");
  nmea_encode(&gps, 0);

  assert_int_equal(6, gps.buffer_size);
  assert_int_equal(0, gps.ready);
  assert_int_equal('H', gps.buffer[253]);
  assert_int_equal('e', gps.buffer[254]);
  assert_int_equal('l', gps.buffer[255]);
  assert_str_equal("lo", gps.buffer);
}

void test_encode_with_overflow(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  memset(&gps.buffer, 0, NMEA_BUFFER_SIZE);
  gps.buffer_size = 253;
  assert_int_equal(1, nmea_encode(&gps, 'H'));
  assert_int_equal(1, nmea_encode(&gps, 'e'));
  assert_int_equal(1, nmea_encode(&gps, 'l'));
  assert_int_equal(0, nmea_encode(&gps, 'l'));
  assert_int_equal(0, nmea_encode(&gps, 'o'));

  assert_int_equal(256, gps.buffer_size);
  assert_int_equal(0, gps.ready);
  assert_int_equal('H', gps.buffer[253]);
  assert_int_equal('e', gps.buffer[254]);
  assert_int_equal('l', gps.buffer[255]);
  assert_int_equal(0, gps.buffer[0]);
}

void test_command_done(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);

  encode_str(&gps, "Hello\r\n");
  assert_int_equal(1, gps.ready);

  encode_str(&gps, "Bye\r\n");
  assert_int_equal(2, gps.ready);

  nmea_command_done(&gps);
  assert_int_equal(1, gps.ready);
  assert_int_equal(7, gps.buffer_head);
  assert_int_equal(5, gps.buffer_size);

  nmea_command_done(&gps);
  assert_int_equal(0, gps.ready);
  assert_int_equal(12, gps.buffer_head);
  assert_int_equal(0, gps.buffer_size);

  nmea_command_done(&gps);
  assert_int_equal(0, gps.ready);
  assert_int_equal(12, gps.buffer_head);
  assert_int_equal(0, gps.buffer_size);

  assert_reg_activity(SREG);
}

void test_parse_sat_type(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgga_example);
  assert_int_equal(1, gps.ready);
  assert_int_equal('P', nmea_parse_sat_type(&gps));
}

void test_parse_alpha(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgga_example);
  assert_int_equal(1, gps.ready);
  check_parse_alpha(&gps, 1, 0, NMEA_NON_ALPHA_CHARACTER);
  check_parse_alpha(&gps, 2, 0, NMEA_NON_ALPHA_CHARACTER);
  check_parse_alpha(&gps, 3, 'N', 0);
  check_parse_alpha(&gps, 4, 0, NMEA_NON_ALPHA_CHARACTER);
  check_parse_alpha(&gps, 5, 'W', 0);
  check_parse_alpha(&gps, 6, 0, NMEA_NON_ALPHA_CHARACTER);
  check_parse_alpha(&gps, 7, 0, NMEA_NON_ALPHA_CHARACTER);
  check_parse_alpha(&gps, 8, 0, NMEA_NON_ALPHA_CHARACTER);
  check_parse_alpha(&gps, 9, 0, NMEA_NON_ALPHA_CHARACTER);
  check_parse_alpha(&gps, 10, 'M', 0);
  check_parse_alpha(&gps, 11, 0, NMEA_NON_ALPHA_CHARACTER);
  check_parse_alpha(&gps, 12, 'M', 0);
  check_parse_alpha(&gps, 13, 0, NMEA_NON_ALPHA_CHARACTER);
  check_parse_alpha(&gps, 14, 0, NMEA_NON_ALPHA_CHARACTER);
  check_parse_alpha(&gps, 15, 0, NMEA_BAD_INDEX);

  check_parse_alpha(&gps, 5, 'W', 0);
  check_parse_alpha(&gps, 3, 'N', 0);

  nmea_init(&gps);
  encode_str(&gps, "A\r\n");
  check_parse_alpha(&gps, 0, 'A', 0);

  nmea_init(&gps);
  encode_str(&gps, "X,,");
  check_parse_alpha(&gps, 0, 'X', 0);
  check_parse_alpha(&gps, 1, 0, NMEA_EMPTY_FIELD);
  check_parse_alpha(&gps, 2, 0, NMEA_BAD_INDEX);
  check_parse_alpha(&gps, 3, 0, NMEA_BAD_INDEX);
}

void test_parse_u32(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgga_example);
  assert_int_equal(1, gps.ready);
  check_parse_u32(&gps, 1, 181908, 0);
  check_parse_u32(&gps, 2, 3404, 0);
  check_parse_u32(&gps, 3, 0, NMEA_NON_NUMERIC_CHARACTER);
  check_parse_u32(&gps, 4, 7044, 0);
  check_parse_u32(&gps, 5, 0, NMEA_NON_NUMERIC_CHARACTER);
  check_parse_u32(&gps, 6, 4, 0);
  check_parse_u32(&gps, 7, 13, 0);
  check_parse_u32(&gps, 8, 1, 0);
  check_parse_u32(&gps, 9, 495, 0);
  check_parse_u32(&gps, 10, 0, NMEA_NON_NUMERIC_CHARACTER);
  check_parse_u32(&gps, 11, 0, NMEA_NEGATIVE_NUMBER);
  check_parse_u32(&gps, 12, 0, NMEA_NON_NUMERIC_CHARACTER);
  check_parse_u32(&gps, 13, 0, 0);
  check_parse_u32(&gps, 14, 0, 0);
  check_parse_u32(&gps, 15, 0, NMEA_BAD_INDEX);

  check_parse_u32(&gps, 1, 181908, 0);
  check_parse_u32(&gps, 1, 181908, 0);
  check_parse_u32(&gps, 0, 0, NMEA_NON_NUMERIC_CHARACTER);
  check_parse_u32(&gps, 3, 0, NMEA_NON_NUMERIC_CHARACTER);

  nmea_init(&gps);
  encode_str(&gps, "1\r\n");
  check_parse_u32(&gps, 0, 1, 0);

  nmea_init(&gps);
  encode_str(&gps, "1");
  check_parse_u32(&gps, 0, 1, 0);

  nmea_init(&gps);
  encode_str(&gps, "0");
  check_parse_u32(&gps, 0, 0, 0);

  nmea_init(&gps);
  encode_str(&gps, "7,,");
  check_parse_u32(&gps, 0, 7, 0);
  check_parse_u32(&gps, 1, 0, NMEA_EMPTY_FIELD);
  check_parse_u32(&gps, 2, 0, NMEA_BAD_INDEX);
  check_parse_u32(&gps, 3, 0, NMEA_BAD_INDEX);

  nmea_init(&gps);
  encode_str(&gps, "$GPGGA,10000000000*80\r\n");
  assert_int_equal(1, gps.ready);
  check_parse_u32(&gps, 1, 0, NMEA_NUMBER_OVERFLOW);

  nmea_init(&gps);
  encode_str(&gps, "$GPGGA,4294967295*80\r\n");
  check_parse_u32(&gps, 1, 4294967295, 0);

  nmea_init(&gps);
  encode_str(&gps, "$GPGGA,4294967297*80\r\n");
  check_parse_u32(&gps, 1, 0, NMEA_NUMBER_OVERFLOW);
}

void test_parse_u16(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgga_example);
  assert_int_equal(1, gps.ready);
  check_parse_u16(&gps, 1, 0, NMEA_NUMBER_OVERFLOW);
  check_parse_u16(&gps, 2, 3404, 0);
  check_parse_u16(&gps, 3, 0, NMEA_NON_NUMERIC_CHARACTER);
  check_parse_u16(&gps, 4, 7044, 0);
  check_parse_u16(&gps, 5, 0, NMEA_NON_NUMERIC_CHARACTER);
  check_parse_u16(&gps, 6, 4, 0);
  check_parse_u16(&gps, 7, 13, 0);
  check_parse_u16(&gps, 8, 1, 0);
  check_parse_u16(&gps, 9, 495, 0);
  check_parse_u16(&gps, 10, 0, NMEA_NON_NUMERIC_CHARACTER);
  check_parse_u16(&gps, 11, 0, NMEA_NEGATIVE_NUMBER);
  check_parse_u16(&gps, 12, 0, NMEA_NON_NUMERIC_CHARACTER);
  check_parse_u16(&gps, 13, 0, 0);
  check_parse_u16(&gps, 14, 0, 0);
  check_parse_u16(&gps, 15, 0, NMEA_BAD_INDEX);

  check_parse_u16(&gps, 2, 3404, 0);
  check_parse_u16(&gps, 2, 3404, 0);
  check_parse_u16(&gps, 0, 0, NMEA_NON_NUMERIC_CHARACTER);
  check_parse_u16(&gps, 3, 0, NMEA_NON_NUMERIC_CHARACTER);

  nmea_init(&gps);
  encode_str(&gps, "1\r\n");
  check_parse_u16(&gps, 0, 1, 0);

  nmea_init(&gps);
  encode_str(&gps, "1");
  check_parse_u16(&gps, 0, 1, 0);

  nmea_init(&gps);
  encode_str(&gps, "0");
  check_parse_u16(&gps, 0, 0, 0);

  nmea_init(&gps);
  encode_str(&gps, "7,,");
  check_parse_u16(&gps, 0, 7, 0);
  check_parse_u16(&gps, 1, 0, NMEA_EMPTY_FIELD);
  check_parse_u16(&gps, 2, 0, NMEA_BAD_INDEX);
  check_parse_u16(&gps, 3, 0, NMEA_BAD_INDEX);

  nmea_init(&gps);
  encode_str(&gps, "$GPGGA,100000*80\r\n");
  assert_int_equal(1, gps.ready);
  check_parse_u16(&gps, 1, 0, NMEA_NUMBER_OVERFLOW);

  nmea_init(&gps);
  encode_str(&gps, "$GPGGA,65535*80\r\n");
  check_parse_u16(&gps, 1, 65535, 0);

  nmea_init(&gps);
  encode_str(&gps, "$GPGGA,65536*80\r\n");
  check_parse_u16(&gps, 1, 0, NMEA_NUMBER_OVERFLOW);
}

void test_parse_u8(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgga_example);
  assert_int_equal(1, gps.ready);
  check_parse_u8(&gps, 1, 0, NMEA_NUMBER_OVERFLOW);
  check_parse_u8(&gps, 2, 0, NMEA_NUMBER_OVERFLOW);
  check_parse_u8(&gps, 3, 0, NMEA_NON_NUMERIC_CHARACTER);
  check_parse_u8(&gps, 4, 0, NMEA_NUMBER_OVERFLOW);
  check_parse_u8(&gps, 5, 0, NMEA_NON_NUMERIC_CHARACTER);
  check_parse_u8(&gps, 6, 4, 0);
  check_parse_u8(&gps, 7, 13, 0);
  check_parse_u8(&gps, 8, 1, 0);
  check_parse_u8(&gps, 9, 0, NMEA_NUMBER_OVERFLOW);
  check_parse_u8(&gps, 10, 0, NMEA_NON_NUMERIC_CHARACTER);
  check_parse_u8(&gps, 11, 0, NMEA_NEGATIVE_NUMBER);
  check_parse_u8(&gps, 12, 0, NMEA_NON_NUMERIC_CHARACTER);
  check_parse_u8(&gps, 13, 0, 0);
  check_parse_u8(&gps, 14, 0, NMEA_NUMBER_OVERFLOW);
  check_parse_u8(&gps, 15, 0, NMEA_BAD_INDEX);
  check_parse_u8(&gps, 16, 0, NMEA_BAD_INDEX);

  check_parse_u8(&gps, 6, 4, 0);
  check_parse_u8(&gps, 6, 4, 0);
  check_parse_u8(&gps, 0, 0, NMEA_NON_NUMERIC_CHARACTER);
  check_parse_u8(&gps, 3, 0, NMEA_NON_NUMERIC_CHARACTER);

  nmea_init(&gps);
  encode_str(&gps, "1\r\n");
  check_parse_u8(&gps, 0, 1, 0);

  nmea_init(&gps);
  encode_str(&gps, "1");
  check_parse_u8(&gps, 0, 1, 0);

  nmea_init(&gps);
  encode_str(&gps, "0");
  check_parse_u8(&gps, 0, 0, 0);

  nmea_init(&gps);
  encode_str(&gps, "7,,");
  check_parse_u8(&gps, 0, 7, 0);
  check_parse_u8(&gps, 1, 0, NMEA_EMPTY_FIELD);
  check_parse_u8(&gps, 2, 0, NMEA_BAD_INDEX);
  check_parse_u8(&gps, 3, 0, NMEA_BAD_INDEX);

  nmea_init(&gps);
  encode_str(&gps, "$GPGGA,1000*80\r\n");
  assert_int_equal(1, gps.ready);
  check_parse_u8(&gps, 1, 0, NMEA_NUMBER_OVERFLOW);

  nmea_init(&gps);
  encode_str(&gps, "$GPGGA,255*80\r\n");
  check_parse_u8(&gps, 1, 255, 0);

  nmea_init(&gps);
  encode_str(&gps, "$GPGGA,256*80\r\n");
  check_parse_u8(&gps, 1, 0, NMEA_NUMBER_OVERFLOW);
}

void test_parse_float(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgga_example);
  assert_int_equal(1, gps.ready);
  check_parse_float(&gps, 1, 181908.0, 0);
  check_parse_float(&gps, 2, 3404.7041778, 0);
  check_parse_float(&gps, 3, 0.0, NMEA_NON_NUMERIC_CHARACTER);
  check_parse_float(&gps, 4, 7044.3966270, 0);
  check_parse_float(&gps, 5, 0.0, NMEA_NON_NUMERIC_CHARACTER);
  check_parse_float(&gps, 6, 4.0, 0);
  check_parse_float(&gps, 7, 13.0, 0);
  check_parse_float(&gps, 8, 1.0, 0);
  check_parse_float(&gps, 9, 495.144, 0);
  check_parse_float(&gps, 10, 0.0, NMEA_NON_NUMERIC_CHARACTER);
  check_parse_float(&gps, 11, -29.2, 0);
  check_parse_float(&gps, 12, 0.0, NMEA_NON_NUMERIC_CHARACTER);
  check_parse_float(&gps, 13, 0.1, 0);
  check_parse_float(&gps, 14, 0, 0);
  check_parse_float(&gps, 15, 0.0, NMEA_BAD_INDEX);

  check_parse_float(&gps, 4, 7044.3966270, 0);
  check_parse_float(&gps, 2, 3404.7041778, 0);

  nmea_init(&gps);
  encode_str(&gps, "1\r\n");
  check_parse_float(&gps, 0, 1.0, 0);

  nmea_init(&gps);
  encode_str(&gps, "1");
  check_parse_float(&gps, 0, 1.0, 0);

  nmea_init(&gps);
  encode_str(&gps, "-1");
  check_parse_float(&gps, 0, -1.0, 0);

  nmea_init(&gps);
  encode_str(&gps, "0");
  check_parse_float(&gps, 0, 0.0, 0);

  nmea_init(&gps);
  encode_str(&gps, "0.0");
  check_parse_float(&gps, 0, 0.0, 0);

  nmea_init(&gps);
  encode_str(&gps, "0.1");
  check_parse_float(&gps, 0, 0.1, 0);

  nmea_init(&gps);
  encode_str(&gps, "-0.1");
  check_parse_float(&gps, 0, -0.1, 0);

  nmea_init(&gps);
  encode_str(&gps, ".1");
  check_parse_float(&gps, 0, 0.1, 0);

  nmea_init(&gps);
  encode_str(&gps, "-.1");
  check_parse_float(&gps, 0, -0.1, 0);

  nmea_init(&gps);
  encode_str(&gps, ".");
  check_parse_float(&gps, 0, 0.0, 0);

  nmea_init(&gps);
  encode_str(&gps, "7,,");
  check_parse_float(&gps, 0, 7, 0);
  check_parse_float(&gps, 1, 0, NMEA_EMPTY_FIELD);
  check_parse_float(&gps, 2, 0, NMEA_BAD_INDEX);

  nmea_init(&gps);
  encode_str(&gps, "$GPGGA,10000000000*80\r\n");
  assert_int_equal(1, gps.ready);
  check_parse_float(&gps, 1, 0, NMEA_NUMBER_OVERFLOW);
}

void test_parse_double(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgga_example);
  assert_int_equal(1, gps.ready);
  check_parse_double(&gps, 1, 181908.0, 0);
  check_parse_double(&gps, 2, 3404.7041778, 0);
  check_parse_double(&gps, 3, 0.0, NMEA_NON_NUMERIC_CHARACTER);
  check_parse_double(&gps, 4, 7044.3966270, 0);
  check_parse_double(&gps, 5, 0.0, NMEA_NON_NUMERIC_CHARACTER);
  check_parse_double(&gps, 6, 4.0, 0);
  check_parse_double(&gps, 7, 13.0, 0);
  check_parse_double(&gps, 8, 1.0, 0);
  check_parse_double(&gps, 9, 495.144, 0);
  check_parse_double(&gps, 10, 0.0, NMEA_NON_NUMERIC_CHARACTER);
  check_parse_double(&gps, 11, -29.2, 0);
  check_parse_double(&gps, 12, 0.0, NMEA_NON_NUMERIC_CHARACTER);
  check_parse_double(&gps, 13, 0.1, 0);
  check_parse_double(&gps, 14, 0, 0);
  check_parse_double(&gps, 15, 0.0, NMEA_BAD_INDEX);

  check_parse_double(&gps, 4, 7044.3966270, 0);
  check_parse_double(&gps, 2, 3404.7041778, 0);

  nmea_init(&gps);
  encode_str(&gps, "1\r\n");
  check_parse_double(&gps, 0, 1.0, 0);

  nmea_init(&gps);
  encode_str(&gps, "1");
  check_parse_double(&gps, 0, 1.0, 0);

  nmea_init(&gps);
  encode_str(&gps, "-1");
  check_parse_double(&gps, 0, -1.0, 0);

  nmea_init(&gps);
  encode_str(&gps, "0");
  check_parse_double(&gps, 0, 0.0, 0);

  nmea_init(&gps);
  encode_str(&gps, "0.0");
  check_parse_double(&gps, 0, 0.0, 0);

  nmea_init(&gps);
  encode_str(&gps, "0.1");
  check_parse_double(&gps, 0, 0.1, 0);

  nmea_init(&gps);
  encode_str(&gps, "-0.1");
  check_parse_double(&gps, 0, -0.1, 0);

  nmea_init(&gps);
  encode_str(&gps, ".1");
  check_parse_double(&gps, 0, 0.1, 0);

  nmea_init(&gps);
  encode_str(&gps, "-.1");
  check_parse_double(&gps, 0, -0.1, 0);

  nmea_init(&gps);
  encode_str(&gps, ".");
  check_parse_double(&gps, 0, 0.0, 0);

  nmea_init(&gps);
  encode_str(&gps, "7,,");
  check_parse_double(&gps, 0, 7, 0);
  check_parse_double(&gps, 1, 0, NMEA_EMPTY_FIELD);
  check_parse_double(&gps, 2, 0, NMEA_BAD_INDEX);

  nmea_init(&gps);
  encode_str(&gps, "$GPGGA,10000000000*80\r\n");
  assert_int_equal(1, gps.ready);
  check_parse_double(&gps, 1, 0, NMEA_NUMBER_OVERFLOW);
}

void test_is_command(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgga_example);
  assert_int_equal(1, gps.ready);
  assert_int_equal(1, nmea_is_command_type(&gps, "GGA"));
  assert_int_equal(0, gps.last_error);
}

void test_is_command_too_short(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgga_example);
  assert_int_equal(1, gps.ready);
  assert_int_equal(0, nmea_is_command_type(&gps, "GGAX"));
  assert_int_equal(0, gps.last_error);
}

void test_is_command_too_long(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgga_example);
  assert_int_equal(1, gps.ready);
  assert_int_equal(0, nmea_is_command_type(&gps, "GG"));
  assert_int_equal(0, gps.last_error);
}

void test_is_command_bad_checksum(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, "$GPGGA,181908.00,3404.7041778,N,07044.3966270*FF\r\n");
  assert_int_equal(1, gps.ready);
  assert_int_equal(0, nmea_is_command_type(&gps, "GGA"));
  assert_int_equal(NMEA_CHECKSUM_ERROR, gps.last_error);
}

void test_is_command_missing_dollar(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, "GPGGA,181908.00,3404.7041778,N,07044.3966270*FF\r\n");
  assert_int_equal(1, gps.ready);
  assert_int_equal(0, nmea_is_command_type(&gps, "GGA"));
  assert_int_equal(NMEA_INVALID_MESSAGE_START, gps.last_error);
}

void test_is_command_message_too_short(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, "$*00\r\n");
  assert_int_equal(1, gps.ready);
  assert_int_equal(0, nmea_is_command_type(&gps, "GGA"));
  assert_int_equal(NMEA_MESSAGE_TOO_SHORT, gps.last_error);
}

void test_is_command_message_truncated(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, "$GPGGA,181908.00,3404.7041778,");
  assert_int_equal(0, gps.ready);
  assert_int_equal(0, nmea_is_command_type(&gps, "GGA"));
  assert_int_equal(NMEA_MESSAGE_TRUNCATED, gps.last_error);
}

//
// $GPGGA
//

void test_nmea_is_gga(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgga_example);
  assert_int_equal(1, gps.ready);
  assert_int_equal(1, nmea_is_gga(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gpgga_timestamp(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgga_example);
  assert_int_equal(181908, nmea_gga_timestamp(&gps));
}

void test_gpgga_time(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgga_example);
  uint8_t hours = 0;
  uint8_t minutes = 0;
  uint8_t seconds = 0;
  nmea_gga_time(&gps, &hours, &minutes, &seconds);
  assert_int_equal(18, hours);
  assert_int_equal(19, minutes);
  assert_int_equal(8, seconds);
  assert_int_equal(0, gps.last_error);
}

void test_gpgga_latitude(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgga_example);
  assert_float_near_equal(34.0784029633333, nmea_gga_latitude(&gps));

  nmea_init(&gps);
  encode_str(&gps, "$GPGGA,181908.00,3404.7041778,S,");
  assert_float_near_equal(-34.0784029633333, nmea_gga_latitude(&gps));

  nmea_init(&gps);
  encode_str(&gps, "$GPGGA,181908.00,3404.7041778,X,");
  assert_float_near_equal(0.0, nmea_gga_latitude(&gps));
  assert_int_equal(NMEA_INVALID_DIRECTION, gps.last_error);
}

void test_gpgga_latitude_double(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgga_example);
  assert_double_near_equal(
      34.0784029633333, nmea_gga_latitude_double(&gps));

  nmea_init(&gps);
  encode_str(&gps, "$GPGGA,181908.00,3404.7041778,S,");
  assert_double_near_equal(
      -34.0784029633333, nmea_gga_latitude_double(&gps));

  nmea_init(&gps);
  encode_str(&gps, "$GPGGA,181908.00,3404.7041778,X,");
  assert_double_near_equal(
      0.0, nmea_gga_latitude_double(&gps));
  assert_int_equal(NMEA_INVALID_DIRECTION, gps.last_error);
}

void test_gpgga_latitude_dms(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgga_example);
  uint8_t degrees = 0;
  uint8_t minutes = 0;
  float seconds = 0;
  char direction = 0;
  nmea_gga_latitude_dms(&gps, &degrees, &minutes, &seconds, &direction);
  assert_int_equal(0, gps.last_error);
  assert_int_equal(34, degrees);
  assert_int_equal(4, minutes);
  assert_float_near_equal(42.250668, seconds);
  assert_int_equal('N', direction);
}

void test_gpgga_longitude(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgga_example);
  assert_float_near_equal(-70.739937, nmea_gga_longitude(&gps));

  nmea_init(&gps);
  encode_str(&gps, "$GPGGA,,,,3404.7041778,E,");
  assert_float_near_equal(34.0784029633333, nmea_gga_longitude(&gps));

  nmea_init(&gps);
  encode_str(&gps, "$GPGGA,,,,3404.7041778,X,");
  assert_float_near_equal(0.0, nmea_gga_longitude(&gps));
  assert_int_equal(NMEA_INVALID_DIRECTION, gps.last_error);
}

void test_gpgga_longitude_double(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgga_example);
  assert_double_near_equal(
      -70.739943783333, nmea_gga_longitude_double(&gps));

  nmea_init(&gps);
  encode_str(&gps, "$GPGGA,,,,3404.7041778,E,");
  assert_double_near_equal(
      34.0784029633333, nmea_gga_longitude_double(&gps));

  nmea_init(&gps);
  encode_str(&gps, "$GPGGA,,,,3404.7041778,X,");
  assert_double_near_equal(
      0.0, nmea_gga_longitude_double(&gps));
  assert_int_equal(NMEA_INVALID_DIRECTION, gps.last_error);
}

void test_gpgga_longitude_dms(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgga_example);
  uint8_t degrees = 0;
  uint8_t minutes = 0;
  float seconds = 0;
  char direction = 0;
  nmea_gga_longitude_dms(&gps, &degrees, &minutes, &seconds, &direction);
  assert_int_equal(0, gps.last_error);
  assert_int_equal(70, degrees);
  assert_int_equal(44, minutes);
  assert_float_near_equal(23.79762, seconds);
  assert_int_equal('W', direction);
}

void test_gpgga_quality(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgga_example);
  assert_int_equal(
      NMEA_QUALITY_RTK_FIXED_AMBIGUITY, nmea_gga_quality(&gps));
  assert_int_equal(0, gps.last_error);

  nmea_init(&gps);
  encode_str(&gps, "$GPGGA,,,,,,10");
  assert_int_equal(10, nmea_gga_quality(&gps));
  assert_int_equal(NMEA_VALUE_OUT_OF_RANGE, gps.last_error);
}

void test_gpgga_num_satellites_in_use(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgga_example);
  assert_int_equal(
      13, nmea_gga_num_satellites_in_use(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gpgga_hdop(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgga_example);
  assert_float_near_equal(
      1.0, nmea_gga_hdop(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gpgga_altitude(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgga_example);
  assert_float_near_equal(
      495.144, nmea_gga_altitude(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gpgga_altitude_units(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgga_example);
  assert_int_equal(
      'M', nmea_gga_altitude_units(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gpgga_undulation(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgga_example);
  assert_float_near_equal(
      -29.2, nmea_gga_undulation(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gpgga_undulation_units(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgga_example);
  assert_int_equal(
      'M', nmea_gga_undulation_units(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gpgga_age(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgga_example);
  assert_float_near_equal(
      0.10, nmea_gga_age(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gpgga_stn_id(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgga_example);
  assert_int_equal(
      0, nmea_gga_stn_id(&gps));
  assert_int_equal(0, gps.last_error);
}

//
// $GPGSA
//

void test_nmea_is_gsa(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgsa_example);
  assert_int_equal(0, gps.last_error);
  assert_int_equal(1, gps.ready);
  assert_int_equal(1, nmea_is_gsa(&gps));
}

void test_gpgsa_mode_ma(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgsa_example);
  assert_int_equal(
      'M', nmea_gsa_mode_ma(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gpgsa_mode_123(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgsa_example);
  assert_int_equal(
      3, nmea_gsa_mode_123(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gpgsa_satellite_prn(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgsa_example);
  uint8_t prn[12];
  uint8_t num_prn = 0;
  nmea_gsa_satellite_prn(&gps, prn, &num_prn);
  assert_int_equal(0, gps.last_error);
  assert_int_equal(10, num_prn);
  assert_int_equal(17, prn[0]);
  assert_int_equal(2, prn[1]);
  assert_int_equal(30, prn[2]);
  assert_int_equal(4, prn[3]);
  assert_int_equal(5, prn[4]);
  assert_int_equal(10, prn[5]);
  assert_int_equal(9, prn[6]);
  assert_int_equal(6, prn[7]);
  assert_int_equal(31, prn[8]);
  assert_int_equal(12, prn[9]);
}

void test_gpgsa_pdop(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgsa_example);
  assert_float_near_equal(
      1.2, nmea_gsa_pdop(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gpgsa_hdop(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgsa_example);
  assert_float_near_equal(
      0.8, nmea_gsa_hdop(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gpgsa_vdop(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgsa_example);
  assert_float_near_equal(
      0.9, nmea_gsa_vdop(&gps));
  assert_int_equal(0, gps.last_error);
}

//
// $GPGSV
//

void test_nmea_is_gsv(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgsv_example);
  assert_int_equal(0, gps.last_error);
  assert_int_equal(3, gps.ready);
  assert_int_equal(1, nmea_is_gsv(&gps));
}

void test_gpgsv_num_msgs(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgsv_example);
  assert_int_equal(
      3, nmea_gsv_num_msgs(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gpgsv_msg_num(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgsv_example);
  assert_int_equal(
      1, nmea_gsv_msg_num(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gpgsv_num_sats(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgsv_example);
  assert_int_equal(
      11, nmea_gsv_num_sats(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gpgsv_satellites(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgsv_example);

  struct SatelliteInView sat[4];
  uint8_t count = 0;

  nmea_gsv_satellites(&gps, sat, &count);
  assert_int_equal(0, gps.last_error);
  assert_int_equal(4, count);

  assert_int_equal(18, sat[0].prn);
  assert_int_equal(87, sat[0].elevation);
  assert_int_equal(50, sat[0].azimuth);
  assert_int_equal(48, sat[0].snr);

  assert_int_equal(22, sat[1].prn);
  assert_int_equal(56, sat[1].elevation);
  assert_int_equal(250, sat[1].azimuth);
  assert_int_equal(49, sat[1].snr);

  assert_int_equal(21, sat[2].prn);
  assert_int_equal(55, sat[2].elevation);
  assert_int_equal(122, sat[2].azimuth);
  assert_int_equal(49, sat[2].snr);

  assert_int_equal(3, sat[3].prn);
  assert_int_equal(40, sat[3].elevation);
  assert_int_equal(284, sat[3].azimuth);
  assert_int_equal(47, sat[3].snr);

  nmea_command_done(&gps);
  nmea_command_done(&gps);
  assert_reg_activity(SREG);

  nmea_gsv_satellites(&gps, sat, &count);
  assert_int_equal(0, gps.last_error);
  assert_int_equal(3, count);

  assert_int_equal(9, sat[0].prn);
  assert_int_equal(15, sat[0].elevation);
  assert_int_equal(107, sat[0].azimuth);
  assert_int_equal(44, sat[0].snr);

  assert_int_equal(14, sat[1].prn);
  assert_int_equal(11, sat[1].elevation);
  assert_int_equal(196, sat[1].azimuth);
  assert_int_equal(41, sat[1].snr);

  assert_int_equal(7, sat[2].prn);
  assert_int_equal(3, sat[2].elevation);
  assert_int_equal(173, sat[2].azimuth);
  assert_int_equal(0, sat[2].snr);
}

//
// $GPGLL
//

void test_nmea_is_gll(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgll_example);
  assert_int_equal(1, gps.ready);
  assert_int_equal(1, nmea_is_gll(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gpgll_timestamp(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgll_example);
  assert_int_equal(205412, nmea_gll_timestamp(&gps));
}

void test_gpgll_time(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgll_example);
  uint8_t hours = 0;
  uint8_t minutes = 0;
  uint8_t seconds = 0;
  nmea_gll_time(&gps, &hours, &minutes, &seconds);
  assert_int_equal(20, hours);
  assert_int_equal(54, minutes);
  assert_int_equal(12, seconds);
  assert_int_equal(0, gps.last_error);
}

void test_gpgll_latitude(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgll_example);
  assert_float_near_equal(51.116692, nmea_gll_latitude(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gpgll_latitude_double(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgll_example);
  assert_double_near_equal(
      51.116689023333, nmea_gll_latitude_double(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gpgll_latitude_dms(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgll_example);
  uint8_t degrees = 0;
  uint8_t minutes = 0;
  float seconds = 0;
  char direction = 0;
  nmea_gll_latitude_dms(&gps, &degrees, &minutes, &seconds, &direction);
  assert_int_equal(0, gps.last_error);
  assert_int_equal(51, degrees);
  assert_int_equal(7, minutes);
  assert_float_near_equal(0.080484, seconds);
  assert_int_equal('N', direction);
}

void test_gpgll_longitude(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgll_example);
  assert_float_near_equal(-114.038811, nmea_gll_longitude(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gpgll_longitude_double(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgll_example);
  assert_double_near_equal(
      -114.0387985733333, nmea_gll_longitude_double(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gpgll_longitude_dms(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgll_example);
  uint8_t degrees = 0;
  uint8_t minutes = 0;
  float seconds = 0;
  char direction = 0;
  nmea_gll_longitude_dms(&gps, &degrees, &minutes, &seconds, &direction);
  assert_int_equal(0, gps.last_error);
  assert_int_equal(114, degrees);
  assert_int_equal(2, minutes);
  assert_float_near_equal(19.674864, seconds);
  assert_int_equal('W', direction);
}

void test_gpgll_data_status(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgll_example);
  assert_int_equal(
      'A', nmea_gll_data_status(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gpgll_mode_ind(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpgll_example);
  assert_int_equal(
      'A', nmea_gll_mode_ind(&gps));
  assert_int_equal(0, gps.last_error);
}

//
// $GPVTG
//

void test_nmea_is_vtg(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpvtg_example);
  assert_int_equal(1, gps.ready);
  assert_int_equal(1, nmea_is_vtg(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gpvtg_track_true(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpvtg_example);
  assert_float_near_equal(
      172.516, nmea_vtg_track_true(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gpvtg_track_mag(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpvtg_example);
  assert_float_near_equal(
      155.295, nmea_vtg_track_mag(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gpvtg_speed_knots(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpvtg_example);
  assert_float_near_equal(
      0.049, nmea_vtg_speed_knots(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gpvtg_speed_km(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpvtg_example);
  assert_float_near_equal(
      0.09, nmea_vtg_speed_km(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gpvtg_mode_ind(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gpvtg_example);
  assert_int_equal(
      'D', nmea_vtg_mode_ind(&gps));
  assert_int_equal(0, gps.last_error);
}

//
// $GPRMC
//

void test_nmea_is_rmc(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gprmc_example);
  assert_int_equal(1, gps.ready);
  assert_int_equal(1, nmea_is_rmc(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gprmc_timestamp(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gprmc_example);
  assert_int_equal(144326, nmea_rmc_timestamp(&gps));
}

void test_gprmc_time(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gprmc_example);
  uint8_t hours = 0;
  uint8_t minutes = 0;
  uint8_t seconds = 0;
  nmea_rmc_time(&gps, &hours, &minutes, &seconds);
  assert_int_equal(14, hours);
  assert_int_equal(43, minutes);
  assert_int_equal(26, seconds);
  assert_int_equal(0, gps.last_error);
}

void test_gprmc_pos_status(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gprmc_example);
  assert_int_equal(
      'A', nmea_rmc_pos_status(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gprmc_latitude(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gprmc_example);
  assert_float_near_equal(51.116699, nmea_rmc_latitude(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gprmc_latitude_double(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gprmc_example);
  assert_double_near_equal(51.116696228333, nmea_rmc_latitude_double(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gprmc_latitude_dms(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gprmc_example);
  uint8_t degrees = 0;
  uint8_t minutes = 0;
  float seconds = 0;
  char direction = 0;
  nmea_rmc_latitude_dms(&gps, &degrees, &minutes, &seconds, &direction);
  assert_int_equal(0, gps.last_error);
  assert_int_equal(51, degrees);
  assert_int_equal(7, minutes);
  assert_float_near_equal(0.106422, seconds);
  assert_int_equal('N', direction);
}

void test_gprmc_longitude(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gprmc_example);
  assert_float_near_equal(-114.0388193516667, nmea_rmc_longitude(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gprmc_longitude_double(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gprmc_example);
  assert_double_near_equal(-114.0388193516667, nmea_rmc_longitude_double(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gprmc_longitude_dms(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gprmc_example);
  uint8_t degrees = 0;
  uint8_t minutes = 0;
  float seconds = 0;
  char direction = 0;
  nmea_rmc_longitude_dms(&gps, &degrees, &minutes, &seconds, &direction);
  assert_int_equal(0, gps.last_error);
  assert_int_equal(114, degrees);
  assert_int_equal(2, minutes);
  assert_float_near_equal(19.749666, seconds);
  assert_int_equal('W', direction);
}

void test_gprmc_speed_knots(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gprmc_example);
  assert_float_near_equal(
      0.080, nmea_rmc_speed_knots(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gprmc_track_true(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gprmc_example);
  assert_float_near_equal(
      323.3, nmea_rmc_track_true(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gprmc_datestamp(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gprmc_example);
  assert_int_equal(
      210307, nmea_rmc_datestamp(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gprmc_date(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gprmc_example);
  uint8_t day = 0;
  uint8_t month = 0;
  uint8_t year = 0;
  nmea_rmc_date(&gps, &day, &month, &year);
  assert_int_equal(0, gps.last_error);
  assert_int_equal(21, day);
  assert_int_equal(3, month);
  assert_int_equal(7, year);
}

void test_gprmc_mag_var(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gprmc_example);
  assert_float_near_equal(
      0.0, nmea_rmc_mag_var(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gprmc_var_dir(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gprmc_example);
  assert_int_equal(
      'E', nmea_rmc_var_dir(&gps));
  assert_int_equal(0, gps.last_error);
}

void test_gprmc_mode_ind(void) {
  struct NMEA_decoder gps;
  nmea_init(&gps);
  encode_str(&gps, gprmc_example);
  assert_int_equal(
      'A', nmea_rmc_mode_ind(&gps));
  assert_int_equal(0, gps.last_error);
}

//
// main
//

int main(void) {
  test(test_encode);
  test(test_encode_with_offset);
  test(test_encode_with_overflow);
  test(test_is_command);
  test(test_is_command_too_short);
  test(test_is_command_too_long);
  test(test_is_command_bad_checksum);
  test(test_is_command_missing_dollar);
  test(test_is_command_message_too_short);
  test(test_is_command_message_truncated);
  test(test_command_done);
  test(test_parse_alpha);
  test(test_parse_u32);
  test(test_parse_u16);
  test(test_parse_u8);
  test(test_parse_float);
  test(test_parse_double);
  test(test_parse_sat_type);

  test(test_nmea_is_gga);
  test(test_gpgga_timestamp);
  test(test_gpgga_time);
  test(test_gpgga_latitude);
  test(test_gpgga_latitude_double);
  test(test_gpgga_latitude_dms);
  test(test_gpgga_longitude);
  test(test_gpgga_longitude_double);
  test(test_gpgga_longitude_dms);
  test(test_gpgga_quality);
  test(test_gpgga_num_satellites_in_use);
  test(test_gpgga_hdop);
  test(test_gpgga_altitude);
  test(test_gpgga_altitude_units);
  test(test_gpgga_undulation);
  test(test_gpgga_undulation_units);
  test(test_gpgga_age);
  test(test_gpgga_stn_id);

  test(test_nmea_is_gsa);
  test(test_gpgsa_mode_ma);
  test(test_gpgsa_mode_123);
  test(test_gpgsa_satellite_prn);
  test(test_gpgsa_pdop);
  test(test_gpgsa_hdop);
  test(test_gpgsa_vdop);

  test(test_nmea_is_gsv);
  test(test_gpgsv_num_msgs);
  test(test_gpgsv_msg_num);
  test(test_gpgsv_num_sats);
  test(test_gpgsv_satellites);

  test(test_nmea_is_gll);
  test(test_gpgll_timestamp);
  test(test_gpgll_time);
  test(test_gpgll_latitude);
  test(test_gpgll_latitude_double);
  test(test_gpgll_latitude_dms);
  test(test_gpgll_longitude);
  test(test_gpgll_longitude_double);
  test(test_gpgll_longitude_dms);
  test(test_gpgll_data_status);
  test(test_gpgll_mode_ind);

  test(test_nmea_is_vtg);
  test(test_gpvtg_track_true);
  test(test_gpvtg_track_mag);
  test(test_gpvtg_speed_knots);
  test(test_gpvtg_speed_km);
  test(test_gpvtg_mode_ind);

  test(test_nmea_is_rmc);
  test(test_gprmc_timestamp);
  test(test_gprmc_time);
  test(test_gprmc_pos_status);
  test(test_gprmc_latitude);
  test(test_gprmc_latitude_double);
  test(test_gprmc_latitude_dms);
  test(test_gprmc_longitude);
  test(test_gprmc_longitude_double);
  test(test_gprmc_longitude_dms);
  test(test_gprmc_speed_knots);
  test(test_gprmc_track_true);
  test(test_gprmc_datestamp);
  test(test_gprmc_date);
  test(test_gprmc_mag_var);
  test(test_gprmc_var_dir);
  test(test_gprmc_mode_ind);

  return 0;
}
