#ifndef PRESSURE_GRAPH_H
#define PRESSURE_GRAPH_H

// Interface to pressure graph
#define PRESSURE_GRAPH_FIRST_ROW 10
#define PRESSURE_GRAPH_ROWS 6
#define PRESSURE_GRAPH_COLS 150

#include <oledm/oledm.h>

#include <inttypes.h>

void pressure_graph_init(struct OLEDM* display);

void pressure_graph_add_point(uint32_t pressure_pa);

// Returns true if the pressure graph has any data stored in it
uint8_t pressure_graph_has_data(void);

// Returns the maximum pressure in the last PRESSURE_GRAPH_COLS samples
uint32_t pressure_graph_max_pa(void);

// Returns the minimum pressure in the last PRESSURE_GRAPH_COLS samples
uint32_t pressure_graph_min_pa(void);

// plot pressure graph to the display
void pressure_graph_plot(void);

#endif
