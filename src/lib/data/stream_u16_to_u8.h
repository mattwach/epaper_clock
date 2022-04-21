#ifndef STREAM_U16_TO_U8_H
#define STREAM_U16_TO_U8_H

#include <inttypes.h>

// takes a uint16_t data stream and scales it to fit in a u8 range
// tracks min and max and rescales target data to fit within target_max
struct StreamU16ToU8 {
  uint16_t buff_size;  // size of rolling buffer
  uint16_t head;  // location of next data entry
  uint8_t wrapped; // set to 1 if the data has wrapped
  uint8_t target_max; // maximum value of target data

  uint16_t min;  // minimum src value in current window
  uint16_t max;  // maximum src value in current window
  uint16_t* src; // source data
  uint8_t* target; // target data
};

void stream_u16_to_u8_init(
    struct StreamU16ToU8* stream,
    uint16_t buff_size,
    uint8_t target_max,
    uint16_t* src,
    uint8_t* target);

void stream_u16_to_u8_add_point(
    struct StreamU16ToU8* stream,
    uint16_t point);
#endif

