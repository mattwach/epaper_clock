#ifndef GRAPH_DISPLAY_H
#define GRAPH_DISPLAY_H

// Provides a memory-efficient way to display a line graph
// e.g. instead of needing a bitmap which needs rows * columns bytes,
// you can provide just a single column of magnitudes.

// If you need your data scaled, consider using data/stream_u16_to_u8.h or similar

#include "oledm.h"

#include <inttypes.h>

struct GraphDisplay {
  // display object to use
  struct OLEDM* display;

  // size of the final graph
  uint8_t rows;
  column_t columns;

  // column offset, this can be used to efficiently "scroll"
  // a graph
  column_t column_offset;

  // optiona tic mark callback.  If != NULL, this will be called
  // with a data offset and can return a number from 0-height which indicates
  // the height of a tic mark to make
  uint8_t (*ticmark_callback)(column_t data_idx);

  // memory for the graph which should be columns bytes in size
  // note that zero for this data == the bottom of the graph which
  // is opposite of the usual convention
  uint8_t* data;
};

void graph_display_init(
    struct GraphDisplay* gd,
    struct OLEDM* display,
    column_t columns,
    uint8_t rows,
    uint8_t* data);

void graph_display_render(
    struct GraphDisplay* gd,
    column_t left_column,
    uint8_t top_row);

#endif
