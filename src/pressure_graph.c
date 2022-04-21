#include "pressure_graph.h"

#include <data/stream_u16_to_u8.h>
#include <oledm/graph_display.h>

struct GraphDisplay gd;
struct StreamU16ToU8 stream;

uint8_t graph_data[PRESSURE_GRAPH_COLS];
uint16_t pressure_data[PRESSURE_GRAPH_COLS];

// Converts a 32-bit pa value to a 16-bit "measurement" which
// had a narrower range and lower resolution but also requires
// half of the storage bits.
static inline uint16_t pa_to_measurement(uint32_t pa) {
  return (uint16_t)((pa - 50000) / 10);
}

// Converts a 16-bit "measurement" back into a 32-bit pa (Pascal)
// pressure value.
static inline uint32_t measurement_to_pa(uint16_t m) {
  return ((uint32_t)(m) * 10) + 50000;
}

// This is a GraphDisplay callback for ticmarks.  It says to
// Use a 3-pixel high tic mark every 6 pixels (6 pixels is
// an hour of data since data is sampled every 10 minutes).
uint8_t every6(column_t column) {
  return (column % 6) == 0 ? 3 : 0;
}

// Called at startup to initialize everyting.
void pressure_graph_init(struct OLEDM* display) {
  graph_display_init(
      &gd, display, PRESSURE_GRAPH_COLS, PRESSURE_GRAPH_ROWS, graph_data);
  gd.ticmark_callback = every6;
  stream_u16_to_u8_init(
      &stream,
      PRESSURE_GRAPH_COLS,
      PRESSURE_GRAPH_ROWS * 8,
      pressure_data,
      graph_data);
}

// Returns true if any data has been added to the graph.
uint8_t pressure_graph_has_data(void) {
  return stream.head || stream.wrapped;
}

// Adds one pressure point to the graph
void pressure_graph_add_point(uint32_t pressure_pa) {
  stream_u16_to_u8_add_point(&stream, pa_to_measurement(pressure_pa));
}

// Returns the maximum pressure available in the graph.
// Note this is a "rolling" max and represents the duration
// if the graph (about 25 hours with the current setup)
uint32_t pressure_graph_max_pa(void) {
  return measurement_to_pa(stream.max);
}

// Returns the "rolling" minimum pressure available in the graph.
uint32_t pressure_graph_min_pa(void) {
  return measurement_to_pa(stream.min);
}

void pressure_graph_plot(void) {
  // we want head to represent the end of the plot
  gd.column_offset = stream.head;
  graph_display_render(&gd, 0, PRESSURE_GRAPH_FIRST_ROW);
}


