#define SSD1680
#include "epaper.h"
#include <twi/twi_fake.h>

#include <test/unit_test.h>

// The ssd1680 uses SPI but we have better testing support for I2C and
// The oled interface mostly abstracts away the differences.

// Directly include some deps to avoid making the test makefile more complex
#include "oledm_i2c.c"
#include <twi/twi_fake.c>

void test_basic_init(void) {
  struct OLEDM display;
  twi_log_reset();
  oledm_basic_init(&display);

  assert_int_equal(296, display.visible_columns);
  assert_int_equal(16, display.visible_rows);
  assert_int_equal(296, display.memory_columns);
  assert_int_equal(16, display.visible_rows);
  assert_int_equal(0, display.start_line);
  assert_int_equal(0, display.error);
  assert_int_equal(0x00, display.option_bits);
  assert_int_equal(0, twi_logidx);
}

void test_start(void) {
  struct OLEDM display;
  oledm_basic_init(&display);
  twi_log_reset();

  oledm_start(&display);
  assert_int_equal(0, display.error);
  assert_history(DDRD, 0x00);
  assert_history(PORTD, 0x00);
  assert_reg_activity(PIND);
  assert_reg_activity(PORTB);

  // Note: the 0x80 are due to i2c protocol.  Just extra noise for this
  // case.
  //
  // Not checking the entire log becuase it contains all of the magic
  // LUT settings, which would be repetitive to include here.
  uint8_t expected_data[] = {
    TWI_INIT,

    TWI_START_WRITE, 0x3C,    // SW reset
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x12,
    TWI_STOP,

    TWI_START_WRITE, 0x3C,    // driver output
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x01,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x27,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x01,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x00,
    TWI_STOP,

    TWI_START_WRITE, 0x3C,    // data entry mode
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x11,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x06,
    TWI_STOP,

    TWI_START_WRITE, 0x3C,    // display update control 1
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x21,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x88,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_STOP,

    // Set memory bounds

    TWI_START_WRITE, 0x3C,    // set x start/end
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x44,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x0F,  // start row (15)
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x00,  // end row
    TWI_STOP,

    TWI_START_WRITE, 0x3C,    // set y start/end
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x45,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x00,  // start column: 0
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x00,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x27,  // end column: 295 (0x127)
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x01,
    TWI_STOP,

    // set memory pointer

    TWI_START_WRITE, 0x3C,    // set x address counter
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x4E,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x0F,  // row (15)
    TWI_STOP,

    TWI_START_WRITE, 0x3C,    // set y address counter
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x4F,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x00,  // column (0)
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x00,
    TWI_STOP,

    TWI_START_WRITE, 0x3C,   // set lut
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x32,
    TWI_WRITE_NO_STOP, 0x80, // a couple of bytes from WS_20_30 
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x66,
    // stop here, since the array has 159 members
  };
  assert_buff_equal(expected_data, twi_log, sizeof(expected_data));
}

void test_epaper_set_full_mode(void) {
  struct OLEDM display;
  oledm_basic_init(&display);
  twi_log_reset();

  epaper_set_full_mode(&display);
  assert_int_equal(0, display.error);

  assert_reg_activity(PIND);  // wait_until_idle()
  assert_reg_activity(PORTB);  // command_data()

  // Not checking the entire log becuase it contains all of the magic
  // LUT settings, which would be repetitive to include here.
  uint8_t expected_data[] = {
    TWI_START_WRITE, 0x3C,   // set lut
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x32,
    TWI_WRITE_NO_STOP, 0x80, // a couple of bytes from WS_20_30 
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x66,
    // stop here, since the array has 159 members
  };
  assert_buff_equal(expected_data, twi_log, sizeof(expected_data));
}

void test_epaper_set_partial_mode(void) {
  struct OLEDM display;
  oledm_basic_init(&display);
  twi_log_reset();

  epaper_set_partial_mode(&display);
  assert_int_equal(0, display.error);

  assert_reg_activity(PIND);  // epaper_reset(), wait_until_idle()
  assert_reg_activity(PORTB);  // command_data()

  // Not checking the entire log becuase it contains all of the magic
  // LUT settings, which would be repetitive to include here.
  uint8_t expected_data[] = {
    TWI_START_WRITE, 0x3C,   // set lut
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x32,
    TWI_WRITE_NO_STOP, 0x80, // a couple of bytes from WS_20_30 
    TWI_WRITE_NO_STOP, 0x00,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x40,
    // stop here, since the array has 159 members
  };
  assert_buff_equal(expected_data, twi_log, sizeof(expected_data));
}

void test_set_memory_bounds(void) {
  struct OLEDM display;
  oledm_basic_init(&display);
  twi_log_reset();

  oledm_set_memory_bounds(&display, 258, 1, 290, 3);
  assert_int_equal(0, display.error);
  assert_reg_activity(PIND);  // wait_until_idle()
  assert_reg_activity(PORTB);  // command_data()

  uint8_t expected_data[] = {
    // Set memory bounds

    TWI_START_WRITE, 0x3C,    // set x start/end
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x44,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x0E,  // start row
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x0C,  // end row
    TWI_STOP,

    TWI_START_WRITE, 0x3C,    // set y start/end
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x45,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x02,  // start column: 258 (0x102)
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x01,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x22,  // end column: 290 (0x122)
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x01,
    TWI_STOP,

    // set memory pointer

    TWI_START_WRITE, 0x3C,    // set x address counter
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x4E,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x0E,  // row
    TWI_STOP,

    TWI_START_WRITE, 0x3C,    // set y address counter
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x4F,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x02,  // column (258)
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x01,
    TWI_STOP,
  };
  assert_buff_equal(expected_data, twi_log, twi_logidx);
}

void test_set_memory_bounds_rotated(void) {
  struct OLEDM display;
  oledm_basic_init(&display);
  display.option_bits |= OLEDM_ROTATE_180;
  twi_log_reset();

  oledm_set_memory_bounds(&display, 258, 1, 290, 3);
  assert_int_equal(0, display.error);
  assert_reg_activity(PIND);  // wait_until_idle()
  assert_reg_activity(PORTB);  // command_data()

  uint8_t expected_data[] = {
    // Set memory bounds

    TWI_START_WRITE, 0x3C,    // set x start/end
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x44,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x01,  // start row
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x03,  // end row
    TWI_STOP,

    TWI_START_WRITE, 0x3C,    // set y start/end
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x45,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x25,  // start column: 258 (0x102)
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x00,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x05,  // end column: 290 (0x122)
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x00,
    TWI_STOP,

    // set memory pointer

    TWI_START_WRITE, 0x3C,    // set x address counter
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x4E,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x01,  // row
    TWI_STOP,

    TWI_START_WRITE, 0x3C,    // set y address counter
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x4F,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x25,  // column (258)
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x00,
    TWI_STOP,
  };
  assert_buff_equal(expected_data, twi_log, twi_logidx);
}

void test_output_pixels(void) {
  struct OLEDM display;
  oledm_basic_init(&display);
  twi_log_reset();

  oledm_start_pixels(&display);
  oledm_write_pixels(&display, 0x98);
  oledm_stop(&display);
  assert_int_equal(0, display.error);
  assert_reg_activity(PORTB);  // command_data()

  uint8_t expected_data[] = {
    TWI_START_WRITE, 0x3C,    // write ram BW
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x24,

    TWI_WRITE_NO_STOP, 0x19,  // because of bit reversal

    TWI_STOP,
  };
  assert_buff_equal(expected_data, twi_log, twi_logidx);
}

void test_start_color_pixels(void) {
  struct OLEDM display;
  oledm_basic_init(&display);
  twi_log_reset();

  epaper_start_color_pixels(&display);
  assert_int_equal(0, display.error);
  assert_reg_activity(PORTB);  // command_data()

  uint8_t expected_data[] = {
    TWI_START_WRITE, 0x3C,    // write ram BW
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x26,
  };
  assert_buff_equal(expected_data, twi_log, twi_logidx);
}

void test_clear(void) {
  struct OLEDM display;
  oledm_basic_init(&display);
  twi_log_reset();

  oledm_clear(&display, 0x98);
  assert_int_equal(0, display.error);
  assert_reg_activity(PIND);  // wait_until_idle()
  assert_reg_activity(PORTB);  // command_data()

  uint8_t expected_data[] = {
    // Set memory bounds

    TWI_START_WRITE, 0x3C,    // set x start/end
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x44,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x0F,  // start row
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x00,  // end row
    TWI_STOP,

    TWI_START_WRITE, 0x3C,    // set y start/end
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x45,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x00,  // start column: 0
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x00,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x27,  // end column: 295 (0x127)
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x01,
    TWI_STOP,

    // set memory pointer

    TWI_START_WRITE, 0x3C,    // set x address counter
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x4E,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x0F,  // row
    TWI_STOP,

    TWI_START_WRITE, 0x3C,    // set y address counter
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x4F,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x00,  // column 0
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x00,
    TWI_STOP,

    TWI_START_WRITE, 0x3C,    // write ram BW
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x24,

    TWI_WRITE_NO_STOP, 0x19,  // because of bit reversal
    TWI_WRITE_NO_STOP, 0x19,  // a couple more
    TWI_WRITE_NO_STOP, 0x19,  
    // testing them all would lead to a really long array
  };
  assert_buff_equal(expected_data, twi_log, sizeof(expected_data));
}

void test_swap_buffers(void) {
  struct OLEDM display;
  oledm_basic_init(&display);
  twi_log_reset();

  epaper_swap_buffers(&display, SLEEP_MODE_0);
  assert_int_equal(0, display.error);
  assert_reg_activity(PIND);  // wait_until_idle()
  assert_reg_activity(PORTB);  // command_data()

  uint8_t expected_data[] = {
    TWI_START_WRITE, 0x3C,    // Display update control 2
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x22,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0xC7,
    TWI_STOP,

    TWI_START_WRITE, 0x3C,    // Master activation
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x20,
    TWI_STOP,

    TWI_START_WRITE, 0x3C,    // Deep sleep mode
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x10,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x00,
    TWI_STOP,
  };
  assert_buff_equal(expected_data, twi_log, twi_logidx);
}

void test_update_partial(void) {
  struct OLEDM display;
  oledm_basic_init(&display);
  twi_log_reset();

  epaper_update_partial(&display, SLEEP_MODE_1);
  assert_int_equal(0, display.error);
  assert_reg_activity(PIND);  // wait_until_idle()
  assert_reg_activity(PORTB);  // command_data()

  uint8_t expected_data[] = {
    TWI_START_WRITE, 0x3C,    // Display update control 2
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x22,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x0F,
    TWI_STOP,

    TWI_START_WRITE, 0x3C,    // Master activation
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x20,
    TWI_STOP,

    TWI_START_WRITE, 0x3C,    // Deep sleep mode
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x10,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x01,
    TWI_STOP,
  };
  assert_buff_equal(expected_data, twi_log, twi_logidx);
}

void test_sleep_mode(void) {
  struct OLEDM display;
  oledm_basic_init(&display);
  twi_log_reset();

  epaper_sleep_mode(&display, SLEEP_MODE_2);
  assert_int_equal(0, display.error);
  assert_reg_activity(PORTB);  // command_data()

  uint8_t expected_data[] = {
    TWI_START_WRITE, 0x3C,    // Deep sleep mode
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x10,
    TWI_WRITE_NO_STOP, 0x80,
    TWI_WRITE_NO_STOP, 0x03,
    TWI_STOP,
  };
  assert_buff_equal(expected_data, twi_log, twi_logidx);
}

void test_reset(void) {
  epaper_reset();
  assert_history(PORTB, 0x00, 0x01);
}

void test_epaper_wait(void) {
  struct OLEDM display;
  oledm_basic_init(&display);
  twi_log_reset();
  epaper_wait(&display);
  assert_reg_activity(PIND);
}

int main(void) {
  test(test_basic_init);
  test(test_start);
  test(test_epaper_set_full_mode);
  test(test_epaper_set_partial_mode);
  test(test_set_memory_bounds);
  test(test_set_memory_bounds_rotated);
  test(test_output_pixels);
  test(test_start_color_pixels);
  test(test_clear);
  test(test_swap_buffers);
  test(test_update_partial);
  test(test_sleep_mode);
  test(test_reset);
  test(test_epaper_wait);

  return 0;
}
