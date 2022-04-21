#include "text.h"

#include "oledm.h"
#include <avr/pgmspace.h>
#include <string.h>

#include "text_common.inc"

struct RLETracker {
  const uint8_t* pgm_data;  // address of next byte
  uint8_t bytes_remaining;  // number of bytes remaining on the current run
  bool_t repeat_mode;  // if true, then we are in repeat mode
};

static uint8_t next_rle_byte(struct RLETracker* self, error_t* err) {
  if (*err) {
    return 0xFF;
  }
  if (self->bytes_remaining == 0) {
    // prep for the next sequence
    self->bytes_remaining = pgm_read_byte_near(self->pgm_data);
    ++self->pgm_data;
    if (self->bytes_remaining & 0x80) {
      self->bytes_remaining &= 0x7F;
      self->repeat_mode = 0;
    } else {
      self->repeat_mode = 1;
    }

    if (self->bytes_remaining == 0) {
      // data is invalid
      *err = TEXT_INVALID_RLE_DATA;
      return 0xFF;
    }
  }

  const uint8_t b = pgm_read_byte_near(self->pgm_data);
  --self->bytes_remaining;
  if (!self->repeat_mode || (self->bytes_remaining == 0)) {
    ++self->pgm_data;
  }
  return b;
}

void text_verifyFont(struct Text* text) {
  if (text->display->error) {
    return;
  }
  const struct VariableFont* font = (struct VariableFont*)text->font;
  const uint8_t* id = font->id;
  if ((pgm_read_byte_near(id) != 'V') ||
      (pgm_read_byte_near(id + 1) != 'A') || 
      (pgm_read_byte_near(id + 2) != 'R') ||
      (pgm_read_byte_near(id + 3) != '1')) {
    text->display->error = SSD1306_BAD_FONT_ID_ERROR;
  }
}

static const uint8_t* find_character_data(
    const struct VariableFont* font,
    const char c,
    uint8_t* width) {
  const uint8_t num_chars = pgm_read_byte_near(&font->num_chars);

  // binary search
  uint8_t mask = 0x80;
  while (mask > num_chars) {
    mask >>= 1;
  }

  uint8_t search_idx = 0;
  char slot_c = 0;
  const uint8_t* pgm_slot_addr = 0;
  for (; mask; mask >>= 1) {
    search_idx |= mask;
    if (search_idx >= num_chars) {
      // too high
      search_idx &= ~mask;
    } else {
      pgm_slot_addr = font->data + (search_idx << 2);
      slot_c = pgm_read_byte_near(pgm_slot_addr);
      if (slot_c > c) {
        // overshot the mark
        search_idx &= ~mask;
      }
    }
  }

  pgm_slot_addr = font->data + (search_idx << 2);
  slot_c = pgm_read_byte_near(pgm_slot_addr);

  if (c == slot_c) {
    // found it
    *width = pgm_read_byte_near(pgm_slot_addr + 1);
    const uint16_t offset =
      (((uint16_t)pgm_read_byte_near(pgm_slot_addr + 2)) << 8) |
      pgm_read_byte_near(pgm_slot_addr + 3);
    return font->data + offset;
  }

  // did not find anything
  return 0;
}

static void render_char(
    struct Text*  text,
    const uint8_t height,
    const char c) {
  if (text->column >= text->display->memory_columns) {
    // off the right edge
    return;
  }

  const struct VariableFont* font = (struct VariableFont*)text->font;

  uint8_t width = 0;
  struct RLETracker rle_tracker;
  rle_tracker.bytes_remaining = 0;
  rle_tracker.pgm_data = find_character_data(font, c, &width);

  if (!rle_tracker.pgm_data) {
    // Character not found
    return;
  }

  // setup boundaries for this character
  column_t max_column = text->column + width;
  if (max_column > text->display->memory_columns) {
    max_column = text->display->memory_columns;
  }

  uint8_t max_row = text->row + height;
  if (max_row > text->display->memory_rows) {
    max_row = text->display->memory_rows;
  }

  // lower-right is inclusive
  oledm_set_bounds(
      text->display,
      text->column,
      text->row,
      max_column - 1,
      max_row - 1);

  oledm_start_pixels(text->display);

  const uint8_t normal = ((text->options & TEXT_OPTION_INVERTED) == 0);
  for (uint8_t row = text->row; row < max_row; ++row) {
    for (uint8_t x = 0; x < width; ++x) {
      // always read the next byte
      const uint8_t data_byte = next_rle_byte(&rle_tracker, &text->display->error);
      // only send it if it's within bounds
      if ((text->column + x) < max_column) {
        oledm_write_pixels(text->display, normal ? data_byte : (data_byte ^ 0xFF));
      }
    }
  }

  oledm_stop(text->display);
  text->column = max_column;
}

void text_strLen(struct Text* text, const char* str, uint8_t len) {
  text_verifyFont(text);
  if ((len == 0) || (text->display->error)) {
    return;
  }

  const uint8_t memory_rows = text->display->memory_rows;
  if (text->row >= memory_rows) {
    // off the bottom
    return;
  }

  const struct VariableFont* font = (struct VariableFont*)text->font;
  const uint8_t height = pgm_read_byte_near(&font->height);

  // write the characters one at a time
  for (uint8_t i=0; i<len; ++i) {
    render_char(text, height, str[i]);
  }
}

void text_clear_row(struct Text* text) {
  text_verifyFont(text);

  if (text->display->error) {
    // display is in an error state
    return;
  }

  const uint8_t memory_rows = text->display->memory_rows;
  const column_t memory_columns = text->display->memory_columns;

  if (text->column >= memory_columns) { 
    // already at the edge
    return;
  }

  if (text->row >= memory_rows) {
    // already off the bottom
    return;
  }

  const struct VariableFont* font = (struct VariableFont*)text->font;
  const uint8_t font_height = pgm_read_byte_near(&font->height);
  const column_t start_column = text->column;
  uint8_t max_row = text->row + font_height - 1;
  if (max_row > (memory_rows - 1)) {
    max_row = memory_rows - 1;
  }

  oledm_set_bounds(
      text->display,
      text->column,
      text->row,
      memory_columns - 1,
      max_row);

  oledm_start_pixels(text->display);
  for (uint8_t row = text->row; row <= max_row; ++row) {
    for (text->column = start_column;
         text->column < memory_columns;
         ++text->column) {
        oledm_write_pixels(text->display, 0);
    }
  }
  oledm_stop(text->display);
}
