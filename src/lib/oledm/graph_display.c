#include "graph_display.h"

void graph_display_init(
    struct GraphDisplay* gd,
    struct OLEDM* display,
    column_t columns,
    uint8_t rows,
    uint8_t* data) {
  gd->display = display;
  gd->rows = rows;
  gd->columns = columns;
  gd->column_offset = 0;
  gd->ticmark_callback = 0;
  gd->data = data;
}

void graph_display_render(
    struct GraphDisplay* gd,
    column_t left_column,
    uint8_t top_row) {
  const uint8_t rows = gd->rows;
  const column_t columns = gd->columns;
  const uint8_t graph_height = ((rows - 1) << 3) + 7;

  oledm_set_bounds(
      gd->display,
      left_column,
      top_row,
      left_column + columns - 1,
      top_row + rows - 1);

  oledm_start_pixels(gd->display);

  for (uint8_t row = 0; row < rows; ++row) {
    column_t data_column = gd->column_offset;
    uint8_t previous_height = graph_height >= gd->data[data_column] ?
      graph_height - gd->data[data_column] :
      0;

    for (uint8_t column = 0; column < columns; ++column) {
      const uint8_t pixel_height = graph_height >= gd->data[data_column] ?
        graph_height - gd->data[data_column] :
        0;
      uint8_t pixels = 0x00;

      // If there is a tickmark callback, we need to call it on the bottom row
      if (gd->ticmark_callback && (row == (rows - 1))) {
        const uint8_t ticmark_height = gd->ticmark_callback(data_column);
        for (uint8_t ticmark_pixel = 0; ticmark_pixel < ticmark_height; ++ticmark_pixel) {
          pixels = (pixels >> 1) | 0x80;  // draw some pixels on the bottom
        }
      }

      // the objective is to render a vertical line from prevous_height to pixel_height
      // while avoiding a pixel on previous height unless it's also height
      uint8_t low_span = pixel_height;
      uint8_t high_span = pixel_height;
      if (previous_height > pixel_height) {
        high_span = previous_height - 1;
      } else if (previous_height < pixel_height) {
        low_span = previous_height + 1;
      }

      // now we look for an overlap of row and low-high span
      uint8_t y_start = row << 3;
      const uint8_t y_end = y_start + 7;
      if ((y_start <= high_span) && (y_end >= low_span)) {
        // there is some overlap
        // The bit system goes from N->S
        //
        // 1
        // 0
        // 0
        // 0
        // 0
        // 0
        // 0
        // 0
        //
        // Thus pixel_height=0 would represent y=0 and byte=0x01
        uint8_t mask = 0x01;
        for (uint8_t y = y_start; y <= y_end; ++y, mask <<= 1) {
          if (y >= low_span) {
            if (y > high_span) {
              break;
            }
            pixels |= mask;
          }
        }
      } 

      oledm_write_pixels(gd->display, pixels);

      // chain to the next pixel
      previous_height = pixel_height;
      ++data_column;
      if (data_column == columns) {
        data_column = 0;
      }
    } 
  }

  oledm_stop(gd->display);
}
