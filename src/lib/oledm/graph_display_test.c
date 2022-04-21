#include "graph_display.h"

#include <test/unit_test.h>

// Directly include some deps to avoid making the test makefile more complex
#include "oledm_fake.c"

// example tickmark callback function
uint8_t every_other(uint8_t column) {
  return column & 1 ?  2 : 0;
}

void test_init(void) {
  struct OLEDM display;
  struct GraphDisplay gd;
  uint8_t data[3];
  oledm_basic_init(&display);
  graph_display_init(&gd, &display, 3, 2, data);
  assert_int_equal(0, gd.display - &display);
  assert_int_equal(2, gd.rows);
  assert_int_equal(3, gd.columns);
  assert_int_equal(0, gd.column_offset);
  assert_int_equal(0, gd.data - data);
}

void test_render(void) {
  struct OLEDM display;
  struct GraphDisplay gd;

  // Render target:
  //
  // ....
  // ....
  // ....
  // ....
  // ....
  // *...
  // .*..
  // ..*.
  //
  // ..*.
  // ..*.
  // ...*
  // ....
  // ....
  // ....
  // ....
  // ....
  //
  // Which equates to a data string of [10, 9, 6, 5]

  uint8_t data[4] = {10, 9, 6, 5};
  oledm_basic_init(&display);
  graph_display_init(&gd, &display, 4, 2, data);
  oledm_log_reset();

  graph_display_render(&gd, 45, 1);
  assert_u32_array_equal(
      ((uint32_t[]){
          OLEDM_SET_BOUNDS, 45, 1, 48, 2,
          OLEDM_START_PIXELS,

          OLEDM_WRITE_PIXELS, 0x20,
          OLEDM_WRITE_PIXELS, 0x40,
          OLEDM_WRITE_PIXELS, 0x80,
          OLEDM_WRITE_PIXELS, 0x00,

          OLEDM_WRITE_PIXELS, 0x00,
          OLEDM_WRITE_PIXELS, 0x00,
          OLEDM_WRITE_PIXELS, 0x03,
          OLEDM_WRITE_PIXELS, 0x04,
          OLEDM_STOP,
      }),
      oledm_log,
      oledm_logidx
  );

}

void test_render_column_offset(void) {
  struct OLEDM display;
  struct GraphDisplay gd;

  // same as above but with an offset of 1
  // ....
  // ....
  // ....
  // ....
  // ...*
  // *..*
  // .*.*
  //     
  // .*.*
  // .*.*
  // ..*.
  // ....
  // ....
  // ....
  // ....
  // ....
  // Which equates to a data string of [10, 9, 6, 5]

  uint8_t data[4] = {10, 9, 6, 5};
  oledm_basic_init(&display);
  graph_display_init(&gd, &display, 4, 2, data);
  gd.column_offset = 1;
  oledm_log_reset();

  graph_display_render(&gd, 45, 1);
  assert_u32_array_equal(
      ((uint32_t[]){
          OLEDM_SET_BOUNDS, 45, 1, 48, 2,
          OLEDM_START_PIXELS,

          OLEDM_WRITE_PIXELS, 0x40,
          OLEDM_WRITE_PIXELS, 0x80,
          OLEDM_WRITE_PIXELS, 0x00,
          OLEDM_WRITE_PIXELS, 0xE0,

          OLEDM_WRITE_PIXELS, 0x00,
          OLEDM_WRITE_PIXELS, 0x03,
          OLEDM_WRITE_PIXELS, 0x04,
          OLEDM_WRITE_PIXELS, 0x03,
          OLEDM_STOP,
      }),
      oledm_log,
      oledm_logidx
  );

}

void test_render_column_offset_tickmarks(void) {
  struct OLEDM display;
  struct GraphDisplay gd;

  // same as column offset but with ticmarks
  // ....
  // ....
  // ....
  // ....
  // ...*
  // *..*
  // .*.*
  //     
  // .*.*
  // .*.*
  // ..*.
  // ....
  // ....
  // ....
  // *.*.
  // *.*.
  // Which equates to a data string of [10, 9, 6, 5]

  uint8_t data[4] = {10, 9, 6, 5};
  oledm_basic_init(&display);
  graph_display_init(&gd, &display, 4, 2, data);
  gd.column_offset = 1;
  gd.ticmark_callback = every_other;
  oledm_log_reset();

  graph_display_render(&gd, 45, 1);
  assert_u32_array_equal(
      ((uint32_t[]){
          OLEDM_SET_BOUNDS, 45, 1, 48, 2,
          OLEDM_START_PIXELS,

          OLEDM_WRITE_PIXELS, 0x40,
          OLEDM_WRITE_PIXELS, 0x80,
          OLEDM_WRITE_PIXELS, 0x00,
          OLEDM_WRITE_PIXELS, 0xE0,

          OLEDM_WRITE_PIXELS, 0xC0,
          OLEDM_WRITE_PIXELS, 0x03,
          OLEDM_WRITE_PIXELS, 0xC4,
          OLEDM_WRITE_PIXELS, 0x03,
          OLEDM_STOP,
      }),
      oledm_log,
      oledm_logidx
  );

}

int main(void) {
    test(test_init);
    test(test_render);
    test(test_render_column_offset);
    test(test_render_column_offset_tickmarks);

    return 0;
}
