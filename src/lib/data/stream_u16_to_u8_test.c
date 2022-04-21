#include "stream_u16_to_u8.h"

#include <test/unit_test.h>

void test_stream(void) {
  struct StreamU16ToU8 s;
  uint16_t src[4];
  uint8_t target[4];

  // initial conditions
  stream_u16_to_u8_init(&s, 4, 100, src, target);
  assert_int_equal(4, s.buff_size);
  assert_int_equal(0, s.head);
  assert_int_equal(0, s.wrapped);
  assert_int_equal(100, s.target_max);
  assert_int_equal(0xFFFF, s.min);
  assert_int_equal(0x0, s.max);
  assert_int_equal(0, src - s.src);
  assert_int_equal(0, target - s.target);

  // add a point
  stream_u16_to_u8_add_point(&s, 500);
  assert_int_equal(1, s.head);
  assert_int_equal(0, s.wrapped);
  assert_int_equal(500, s.min);
  assert_int_equal(500, s.max);

  assert_int_equal(500, src[0]);
  assert_int_equal(0, target[0]);

  // add another point
  stream_u16_to_u8_add_point(&s, 600);
  assert_int_equal(2, s.head);
  assert_int_equal(0, s.wrapped);
  assert_int_equal(500, s.min);
  assert_int_equal(600, s.max);

  assert_int_equal(500, src[0]);
  assert_int_equal(600, src[1]);
  assert_int_equal(0, target[0]);
  assert_int_equal(100, target[1]);

  // add another point
  stream_u16_to_u8_add_point(&s, 700);
  assert_int_equal(3, s.head);
  assert_int_equal(0, s.wrapped);
  assert_int_equal(500, s.min);
  assert_int_equal(700, s.max);

  assert_int_equal(500, src[0]);
  assert_int_equal(600, src[1]);
  assert_int_equal(700, src[2]);
  assert_int_equal(0, target[0]);
  assert_int_equal(50, target[1]);
  assert_int_equal(100, target[2]);

  // add last point before wrap
  stream_u16_to_u8_add_point(&s, 600);
  assert_int_equal(0, s.head);
  assert_int_equal(1, s.wrapped);
  assert_int_equal(500, s.min);
  assert_int_equal(700, s.max);

  assert_int_equal(500, src[0]);
  assert_int_equal(600, src[1]);
  assert_int_equal(700, src[2]);
  assert_int_equal(600, src[3]);
  assert_int_equal(0, target[0]);
  assert_int_equal(50, target[1]);
  assert_int_equal(100, target[2]);
  assert_int_equal(50, target[3]);

  // This wrapping point is going to take out
  // the minimum, leaving the new range as 600-700
  stream_u16_to_u8_add_point(&s, 650);
  assert_int_equal(1, s.head);
  assert_int_equal(1, s.wrapped);
  assert_int_equal(600, s.min);
  assert_int_equal(700, s.max);

  assert_int_equal(650, src[0]);
  assert_int_equal(600, src[1]);
  assert_int_equal(700, src[2]);
  assert_int_equal(600, src[3]);
  assert_int_equal(50, target[0]);
  assert_int_equal(0, target[1]);
  assert_int_equal(100, target[2]);
  assert_int_equal(0, target[3]);

  // Create a new minimum again
  stream_u16_to_u8_add_point(&s, 500);
  assert_int_equal(2, s.head);
  assert_int_equal(1, s.wrapped);
  assert_int_equal(500, s.min);
  assert_int_equal(700, s.max);

  assert_int_equal(650, src[0]);
  assert_int_equal(500, src[1]);
  assert_int_equal(700, src[2]);
  assert_int_equal(600, src[3]);
  assert_int_equal(75, target[0]);
  assert_int_equal(0, target[1]);
  assert_int_equal(100, target[2]);
  assert_int_equal(50, target[3]);

  // Now there will be a new maximum
  stream_u16_to_u8_add_point(&s, 450);
  assert_int_equal(3, s.head);
  assert_int_equal(1, s.wrapped);
  assert_int_equal(450, s.min);
  assert_int_equal(650, s.max);

  assert_int_equal(650, src[0]);
  assert_int_equal(500, src[1]);
  assert_int_equal(450, src[2]);
  assert_int_equal(600, src[3]);
  assert_int_equal(100, target[0]);
  assert_int_equal(25, target[1]);
  assert_int_equal(0, target[2]);
  assert_int_equal(75, target[3]);

  // Now lets fill with 500 until that is all that is left
  stream_u16_to_u8_add_point(&s, 500);
  assert_int_equal(0, s.head);
  assert_int_equal(1, s.wrapped);
  assert_int_equal(450, s.min);
  assert_int_equal(650, s.max);

  assert_int_equal(650, src[0]);
  assert_int_equal(500, src[1]);
  assert_int_equal(450, src[2]);
  assert_int_equal(500, src[3]);
  assert_int_equal(100, target[0]);
  assert_int_equal(25, target[1]);
  assert_int_equal(0, target[2]);
  assert_int_equal(25, target[3]);

  // 500 again
  stream_u16_to_u8_add_point(&s, 500);
  assert_int_equal(1, s.head);
  assert_int_equal(1, s.wrapped);
  assert_int_equal(450, s.min);
  assert_int_equal(500, s.max);

  assert_int_equal(500, src[0]);
  assert_int_equal(500, src[1]);
  assert_int_equal(450, src[2]);
  assert_int_equal(500, src[3]);
  assert_int_equal(100, target[0]);
  assert_int_equal(100, target[1]);
  assert_int_equal(0, target[2]);
  assert_int_equal(100, target[3]);

  // 500 again
  stream_u16_to_u8_add_point(&s, 500);
  assert_int_equal(2, s.head);
  assert_int_equal(1, s.wrapped);
  assert_int_equal(450, s.min);
  assert_int_equal(500, s.max);

  assert_int_equal(500, src[0]);
  assert_int_equal(500, src[1]);
  assert_int_equal(450, src[2]);
  assert_int_equal(500, src[3]);
  assert_int_equal(100, target[0]);
  assert_int_equal(100, target[1]);
  assert_int_equal(0, target[2]);
  assert_int_equal(100, target[3]);

  // last time
  stream_u16_to_u8_add_point(&s, 500);
  assert_int_equal(3, s.head);
  assert_int_equal(1, s.wrapped);
  assert_int_equal(500, s.min);
  assert_int_equal(500, s.max);

  assert_int_equal(500, src[0]);
  assert_int_equal(500, src[1]);
  assert_int_equal(500, src[2]);
  assert_int_equal(500, src[3]);
  assert_int_equal(0, target[0]);
  assert_int_equal(0, target[1]);
  assert_int_equal(0, target[2]);
  assert_int_equal(0, target[3]);
}

int main(void) {
    test(test_stream);

    return 0;
}
