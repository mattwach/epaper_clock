#include "text.h"

#include <test/unit_test.h>

// Directly include some deps to avoid making the test makefile more complex
#include "oledm_fake.c"

static struct VariableFont font = {
    .id = { 'V', 'A', 'R', '1' },
    .num_chars = 3,
    .height = 2,
    .data = {
      // the data table
      '+', 3, 0x00, 12,  // 12 bytes to the start of the data
      '-', 3, 0x00, 19,
      '|', 1, 0x00, 23, 

      // 7 -#-
      // 6 -#-
      // 5 -#-
      // 4 -#-
      // 3 -#-
      // 2 -#-
      // 1 -#-
      // 0 ###
      // 7 -#-
      // 6 -#-
      // 5 -#-
      // 4 -#-
      // 3 -#-
      // 2 -#-
      // 1 -#-
      // 0 -#-
      0x86, 0x01, 0xFF, 0x01, 0x00, 0xFF, 0x00,

      // 7 ---
      // 6 ---
      // 5 ---
      // 4 ---
      // 3 ---
      // 2 ---
      // 1 ---
      // 0 ###
      // 7 ---
      // 6 ---
      // 5 ---
      // 4 ---
      // 3 ---
      // 2 ---
      // 1 ---
      // 0 ---
      0x03, 0x01, 0x03, 0x00,

      // 7 #
      // 6 #
      // 5 #
      // 4 #
      // 3 #
      // 2 #
      // 1 #
      // 0 #
      // 7 #
      // 6 #
      // 5 #
      // 4 #
      // 3 #
      // 2 #
      // 1 #
      // 0 #
      0x02, 0xFF
    },
};

void test_init(void) {
    struct OLEDM display;
    struct Text text;
    oledm_basic_init(&display);
    text_init(&text, &font, &display);
    assert_int_equal(0, display.error);
    assert_int_equal(0, text.row);
    assert_int_equal(0, text.column);
    assert_int_equal(0, (uint8_t*)text.font - (uint8_t*)&font);
    assert_int_equal(0, text.display - &display);
}

void test_str(void) {
    struct OLEDM display;
    struct Text text;
    oledm_basic_init(&display);
    text_init(&text, &font, &display);
    oledm_log_reset();

    text.column = 16;
    text.row = 3;
    text_str(&text, "|-+");

    assert_int_equal(0, display.error);

    assert_int_equal(23, text.column);
    assert_int_equal(3, text.row);

    assert_u32_array_equal(
        ((uint32_t[]){
            // Draw the |
            OLEDM_SET_BOUNDS, 16, 3, 16, 4,
            OLEDM_START_PIXELS,
            OLEDM_WRITE_PIXELS, 0xFF,
            OLEDM_WRITE_PIXELS, 0xFF,
            OLEDM_STOP,

            // Draw the -
            OLEDM_SET_BOUNDS, 17, 3, 19, 4,
            OLEDM_START_PIXELS,
            OLEDM_WRITE_PIXELS, 0x01,
            OLEDM_WRITE_PIXELS, 0x01,
            OLEDM_WRITE_PIXELS, 0x01,
            OLEDM_WRITE_PIXELS, 0x00,
            OLEDM_WRITE_PIXELS, 0x00,
            OLEDM_WRITE_PIXELS, 0x00,
            OLEDM_STOP,

            // Draw the +
            OLEDM_SET_BOUNDS, 20, 3, 22, 4,
            OLEDM_START_PIXELS,
            OLEDM_WRITE_PIXELS, 0x01,
            OLEDM_WRITE_PIXELS, 0xFF,
            OLEDM_WRITE_PIXELS, 0x01,
            OLEDM_WRITE_PIXELS, 0x00,
            OLEDM_WRITE_PIXELS, 0xFF,
            OLEDM_WRITE_PIXELS, 0x00,
            OLEDM_STOP,
        }),
        oledm_log,
        oledm_logidx
    );
}

void test_str_Edge(void) {
    struct OLEDM display;
    struct Text text;
    oledm_basic_init(&display);
    text_init(&text, &font, &display);
    oledm_log_reset();

    text.column = 126;
    text.row = 7;
    text_str(&text, "+-");

    assert_int_equal(0, display.error);

    assert_int_equal(128, text.column);
    assert_int_equal(7, text.row);
    assert_u32_array_equal(
        ((uint32_t[]){
            // Draw the +
            OLEDM_SET_BOUNDS, 126, 7, 127, 7,
            OLEDM_START_PIXELS,
            OLEDM_WRITE_PIXELS, 0x01,
            OLEDM_WRITE_PIXELS, 0xFF,
            OLEDM_STOP,
        }),
        oledm_log,
        oledm_logidx
    );
}

void test_str_OffRight(void) {
    struct OLEDM display;
    struct Text text;
    oledm_basic_init(&display);
    text_init(&text, &font, &display);
    oledm_log_reset();

    text.column = 128;
    text.row = 0;
    text_str(&text, "+-");

    assert_int_equal(0, display.error);

    assert_int_equal(128, text.column);
    assert_int_equal(0, text.row);
    assert_int_equal(0, oledm_logidx);
}

void test_str_OffBottom(void) {
    struct OLEDM display;
    struct Text text;
    oledm_basic_init(&display);
    text_init(&text, &font, &display);
    oledm_log_reset();

    text.column = 0;
    text.row = 8;
    text_str(&text, "+-");

    assert_int_equal(0, display.error);

    assert_int_equal(0, text.column);
    assert_int_equal(8, text.row);
    assert_int_equal(0, oledm_logidx);
}

void test_str_Empty(void) {
    struct OLEDM display;
    struct Text text;
    oledm_basic_init(&display);
    text_init(&text, &font, &display);
    oledm_log_reset();

    text.column = 1;
    text.row = 2;
    text_str(&text, "");

    assert_int_equal(0, display.error);

    assert_int_equal(1, text.column);
    assert_int_equal(2, text.row);
    assert_int_equal(0, oledm_logidx);
}

void test_str_preerr(void) {
    struct OLEDM display;
    struct Text text;
    oledm_basic_init(&display);
    text_init(&text, &font, &display);
    oledm_log_reset();
    display.error = 0x23;

    text.column = 1;
    text.row = 2;
    text_str(&text, "+-");

    assert_int_equal(0x23, display.error);

    assert_int_equal(1, text.column);
    assert_int_equal(2, text.row);
    assert_int_equal(0, oledm_logidx);
}

void test_clear_row(void) {
    struct OLEDM display;
    struct Text text;
    oledm_basic_init(&display);
    text_init(&text, &font, &display);
    oledm_log_reset();

    text.column = 125;
    text.row = 3;
    text_clear_row(&text);

    assert_int_equal(0, display.error);

    assert_int_equal(128, text.column);
    assert_int_equal(3, text.row);

    assert_u32_array_equal(
        ((uint32_t[]){
            OLEDM_SET_BOUNDS, 125, 3, 127, 4,
            OLEDM_START_PIXELS,
            OLEDM_WRITE_PIXELS, 0x00,  // 125
            OLEDM_WRITE_PIXELS, 0x00,  // 126
            OLEDM_WRITE_PIXELS, 0x00,  // 127
            OLEDM_WRITE_PIXELS, 0x00,  // 125
            OLEDM_WRITE_PIXELS, 0x00,  // 126
            OLEDM_WRITE_PIXELS, 0x00,  // 127
            OLEDM_STOP,
        }),
        oledm_log,
        oledm_logidx
    );
}

int main(void) {
    test(test_init);
    test(test_str);
    test(test_str_Edge);
    test(test_str_OffRight);
    test(test_str_OffBottom);
    test(test_str_Empty);
    test(test_str_preerr);
    test(test_clear_row);

    return 0;
}
