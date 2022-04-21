#include "ms8607.h"
#include <twi/twi_fake.h>

#include <test/unit_test.h>

// Directly include some deps to avoid making the test makefile more complex
#include <twi/twi_fake.c>

uint8_t pt_cal[] = {
    0x42, 0xA1,  // CRC
    0xA9, 0x47,  // C1
    0xA7, 0x43,  // C2
    0x64, 0xD3,  // C3
    0x65, 0xCA,  // C4
    0x7A, 0x50,  // C5
    0x68, 0x77,  // C6
};

void test_init(void) {
  struct MS8607 ms8607;

  twi_set_read_data(pt_cal);
  ms8607_init(&ms8607);
  assert_int_equal(0, ms8607.err);

  assert_buff_equal(
      ((uint8_t[]){
       TWI_INIT,

       // Reset
       TWI_START_WRITE, 0x76,
       TWI_WRITE_NO_STOP, 0x1E,  // PT_RESET
       TWI_STOP,

       TWI_START_WRITE, 0x40,
       TWI_WRITE_NO_STOP, 0xFE,  // HUM_RESET
       TWI_STOP,

       // Read calibration values
       TWI_START_WRITE, 0x76,
       TWI_WRITE_NO_STOP, 0xA0,  // Read CRC
       TWI_READ_NO_STOP, 0x76, 2,
       TWI_STOP,

       TWI_START_WRITE, 0x76,
       TWI_WRITE_NO_STOP, 0xA2,  // Read C1
       TWI_READ_NO_STOP, 0x76, 2,
       TWI_STOP,

       TWI_START_WRITE, 0x76,
       TWI_WRITE_NO_STOP, 0xA4,  // Read C2
       TWI_READ_NO_STOP, 0x76, 2,
       TWI_STOP,

       TWI_START_WRITE, 0x76,
       TWI_WRITE_NO_STOP, 0xA6,  // Read C3
       TWI_READ_NO_STOP, 0x76, 2,
       TWI_STOP,

       TWI_START_WRITE, 0x76,
       TWI_WRITE_NO_STOP, 0xA8,  // Read C4
       TWI_READ_NO_STOP, 0x76, 2,
       TWI_STOP,

       TWI_START_WRITE, 0x76,
       TWI_WRITE_NO_STOP, 0xAA,  // Read C5
       TWI_READ_NO_STOP, 0x76, 2,
       TWI_STOP,

       TWI_START_WRITE, 0x76,
       TWI_WRITE_NO_STOP, 0xAC,  // Read C6
       TWI_READ_NO_STOP, 0x76, 2,
       TWI_STOP,
       }),
      twi_log,
      twi_logidx
    );

  assert_int_equal(0, ms8607.err);
  assert_int_equal(0x08, ms8607.temperature_resolution);
  assert_int_equal(0x08, ms8607.pressure_resolution);

  assert_int_equal(0x42A1, ms8607.pt_cal.crc);
  assert_int_equal(0xA947, ms8607.pt_cal.sens);
  assert_int_equal(0xA743, ms8607.pt_cal.off);
  assert_int_equal(0x64D3, ms8607.pt_cal.tcs);
  assert_int_equal(0x65CA, ms8607.pt_cal.tco);
  assert_int_equal(0x7A50, ms8607.pt_cal.tref);
  assert_int_equal(0x6877, ms8607.pt_cal.tempsens);
}

void test_humidity_settings(void) {
  struct MS8607 ms8607;
  twi_set_read_data(pt_cal);
  ms8607_init(&ms8607);
  twi_log_reset();

  twi_set_read_data(
      ((uint8_t[]){0x00,}));

  ms8607_humidity_settings(&ms8607, OSR_1024, TRUE);

  assert_buff_equal(
      ((uint8_t[]){
       TWI_START_WRITE, 0x40,
       TWI_WRITE_NO_STOP, 0xE7,  // Read user register
       TWI_READ_NO_STOP, 0x40, 1,
       TWI_STOP,

       TWI_START_WRITE, 0x40,
       TWI_WRITE_NO_STOP, 0xE6,  // Write user register
       TWI_WRITE_NO_STOP, 0x84, 
       TWI_STOP,
       }),
      twi_log,
      twi_logidx);

  assert_int_equal(0, ms8607.err);
}

void test_read_temperature(void) {
  struct MS8607 ms8607;
  twi_set_read_data(pt_cal);
  ms8607_init(&ms8607);
  twi_log_reset();

  twi_set_read_data(
      ((uint8_t[]){
       0x7A, 0x41, 0x11,  // Raw temperature 
  }));

  int16_t temp_cc = 0;

  ms8607_read_values(&ms8607, &temp_cc, NULL, NULL);

  assert_buff_equal(
      ((uint8_t[]){
       TWI_START_WRITE, 0x76,
       TWI_WRITE_NO_STOP, 0x58,  // Read D2 (4096)
       TWI_STOP,

       TWI_START_WRITE, 0x76,
       TWI_WRITE_NO_STOP, 0x00, // ADC read
       TWI_STOP,

       TWI_READ_NO_STOP, 0x76, 3,  // Read temperature data
       TWI_STOP,
       }),
      twi_log,
      twi_logidx);

  assert_int_equal(0, ms8607.err);
  assert_int_equal(1987, temp_cc);
}

void test_read_pressure(void) {
  struct MS8607 ms8607;
  twi_set_read_data(pt_cal);
  ms8607_init(&ms8607);
  twi_log_reset();

  twi_set_read_data(
      ((uint8_t[]){
       0x7A, 0x41, 0x11,  // Raw temperature 
       0x64, 0x33, 0xD9,  // Raw Pressure
  }));

  uint32_t pressure_pa = 0;

  ms8607_read_values(&ms8607, NULL, &pressure_pa, NULL);

  assert_buff_equal(
      ((uint8_t[]){
       TWI_START_WRITE, 0x76,
       TWI_WRITE_NO_STOP, 0x58,  // Read D2 (4096)
       TWI_STOP,

       TWI_START_WRITE, 0x76,
       TWI_WRITE_NO_STOP, 0x00, // ADC read
       TWI_STOP,

       TWI_READ_NO_STOP, 0x76, 3,  // Read temperature data
       TWI_STOP,

       TWI_START_WRITE, 0x76,
       TWI_WRITE_NO_STOP, 0x48,  // Read D1 (4096)
       TWI_STOP,

       TWI_START_WRITE, 0x76,
       TWI_WRITE_NO_STOP, 0x00, // ADC read
       TWI_STOP,

       TWI_READ_NO_STOP, 0x76, 3,  // Read pressure data
       TWI_STOP,
       }),
      twi_log,
      twi_logidx);

  assert_int_equal(0, ms8607.err);
  assert_int_equal(100090, pressure_pa);
}

void test_read_humidity(void) {
  struct MS8607 ms8607;
  twi_set_read_data(pt_cal);
  ms8607_init(&ms8607);
  twi_log_reset();

  twi_set_read_data(
      ((uint8_t[]){
       0x7A, 0x41, 0x11,  // Raw temperature 
       0x6F, 0x6E, 0x68,  // Raw Humidity + CRC
  }));

  uint16_t humidity_cpct = 0;

  ms8607_read_values(&ms8607, NULL, NULL, &humidity_cpct);

  assert_buff_equal(
      ((uint8_t[]){
       TWI_START_WRITE, 0x76,
       TWI_WRITE_NO_STOP, 0x58,  // Read D2 (4096)
       TWI_STOP,

       TWI_START_WRITE, 0x76,
       TWI_WRITE_NO_STOP, 0x00, // ADC read
       TWI_STOP,

       TWI_READ_NO_STOP, 0x76, 3,  // Read temperature data
       TWI_STOP,

       TWI_START_WRITE, 0x40,
       TWI_WRITE_NO_STOP, 0xF5,  // Measure no hold
       TWI_STOP,

       TWI_READ_NO_STOP, 0x40, 3,  // Read humidity data + CRC
       TWI_STOP,
       }),
      twi_log,
      twi_logidx);

  assert_int_equal(0, ms8607.err);
  assert_int_equal(4838, humidity_cpct);
}

int main(void) {
  test(test_init);
  test(test_humidity_settings);
  test(test_read_temperature);
  test(test_read_pressure);
  test(test_read_humidity);

  return 0;
}
