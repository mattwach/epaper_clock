#include "stream_u16_to_u8.h"

void stream_u16_to_u8_init(
    struct StreamU16ToU8* s,
    uint16_t buff_size,
    uint8_t target_max,
    uint16_t* src,
    uint8_t* target) {
  s->buff_size = buff_size;
  s->head = 0;
  s->wrapped = 0;
  s->target_max = target_max;
  s->min = 0xFFFF;
  s->max = 0x0;
  s->src = src;
  s->target = target;
}

static void scale_target_points(
    struct StreamU16ToU8* s, uint16_t start_idx, uint16_t count) {
  // if min-max, we could have a divide by zero.  Just set it to one
  // and the result will be zero in this case.
  const uint16_t range = s->max > s->min ?  s->max - s->min : 1;

  for (uint16_t i=0; i<count; ++i) {
    // need to take the measurement in the range of min-max and
    // scale it to target_max
    s->target[start_idx] = 
      (uint8_t)(s->target_max *
          (uint32_t)(s->src[start_idx] - s->min) /
          range);
    ++start_idx;
    if (start_idx == s->buff_size) {
      start_idx = 0;
    }
  }
}

static void add_src_point(
    struct StreamU16ToU8* s, uint16_t value, uint8_t call_scale_target_points) {
  uint8_t set_all_target_points = 0;
  if (value < s->min) {
    s->min = value;
    set_all_target_points = 1;
  }
  
  if (value > s->max) {
    s->max = value;
    set_all_target_points = 1;
  }

  s->src[s->head] = value;

  if (call_scale_target_points) {
    if (set_all_target_points) {
      scale_target_points(s, 0, s->buff_size);
    } else {
      scale_target_points(s, s->head, 1);
    }
  }
  ++s->head;
  if (s->head == s->buff_size) {
    s->head = 0;
    s->wrapped = 1;
  }
}

static void find_new_min(struct StreamU16ToU8* s) {
  s->min = 0xFFFF;
  for (uint8_t i=0; i < s->buff_size; ++i) {
    if (s->src[i] < s->min) {
      s->min = s->src[i];
    }
  }
}

static void find_new_max(struct StreamU16ToU8* s) {
  s->max = 0x0;
  for (uint8_t i=0; i < s->buff_size; ++i) {
    if (s->src[i] > s->max) {
      s->max = s->src[i];
    }
  }
}

void stream_u16_to_u8_add_point(struct StreamU16ToU8* s, uint16_t value) {
  //
  // lot of cases needed to efficienctly maintain min/max
  //

  if (!s->wrapped || (s->min == s->max)) {
    // No need to check the dropped points
    add_src_point(s, value, 1);
    return;
  }

  // if the point that is being dropped was a min or a max,
  // then the whole array needs to scanned to find a new one
  const uint8_t scan_for_new_min = 
    (s->min == s->src[s->head]) &&
    (value > s->min);

  const uint8_t scan_for_new_max =
    (s->max == s->src[s->head]) &&
    (value < s->max);

  // Note that call_set_pixels should only be set if scan_for_new_min
  // and scan_for_new_max are false.  Otherwise the CPU might waste
  // cycles calculating all pixels (because measurement sets a new
  // min or max) onyl to set them all again due to scan_for_new_min or
  // scan_for_new_max being set.
  add_src_point(s, value, !scan_for_new_min && !scan_for_new_max);

  if (scan_for_new_min) {
    find_new_min(s);
  }
  if (scan_for_new_max) {
    find_new_max(s);
  }

  if (scan_for_new_min || scan_for_new_max) {
    // all pixels need to be recalculated if min or max changes
    scale_target_points(s, 0, s->buff_size);
  }
}
