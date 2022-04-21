#include "oledm_fake.h"
#include <string.h>

uint32_t oledm_log[16384];
uint16_t oledm_logidx;

#define LOG(v) oledm_log[oledm_logidx++] = (v)

void oledm_basic_init(struct OLEDM* display) {
    LOG(OLEDM_BASIC_INIT);
    memset(display, 0, sizeof(struct OLEDM));
    display->visible_columns = 128;
    display->memory_columns = 128;
    display->visible_rows = 8;
    display->memory_rows = 8;
}

void oledm_start(struct OLEDM* display) {
    LOG(OLEDM_START);
}

void oledm_display_off(struct OLEDM* display) {
  LOG(OLEDM_DISPLAY_OFF);
}

void oledm_display_on(struct OLEDM* display) {
  LOG(OLEDM_DISPLAY_ON);
}

void oledm_vscroll(struct OLEDM* display, int8_t rows) {
  LOG(OLEDM_VSCROLL);
  LOG(rows);
}

void oledm_clear(struct OLEDM* display, uint8_t byte) {
  LOG(OLEDM_CLEAR);
  LOG(byte);
}

void oledm_start_pixels(struct OLEDM* display) {
  LOG(OLEDM_START_PIXELS);
}

void oledm_stop(struct OLEDM* display) {
  LOG(OLEDM_STOP);
}


void oledm_set_bounds(struct OLEDM* display,
  column_t left_column, uint8_t top_row,
  column_t right_column, uint8_t bottom_row) {
  LOG(OLEDM_SET_BOUNDS);
  LOG(left_column);
  LOG(top_row);
  LOG(right_column);
  LOG(bottom_row);
}

void oledm_set_memory_bounds(struct OLEDM* display,
  column_t left_column, uint8_t top_row,
  column_t right_column, uint8_t bottom_row) {
  LOG(OLEDM_SET_MEMORY_BOUNDS);
  LOG(left_column);
  LOG(top_row);
  LOG(right_column);
  LOG(bottom_row);
}

void oledm_write_pixels(struct OLEDM* display, uint8_t byte) {
  LOG(OLEDM_WRITE_PIXELS);
  LOG(byte);
}
