#ifndef GPS_H
#define GPS_H

#include <time.h>

// GPS can be a major source of power usage so it's good to keep
// track of its history.
typedef enum {
  GPS_STATS_AUTO = 0,
  GPS_STATS_HIDE = 1,
  GPS_STATS_SHOW = 2,
} GPSStatShowPolicy;

struct GPSStats {
  time_t enable_time_y2k;
  time_t last_enable;
  time_t last_lock;
  uint32_t total_enable_seconds;
  uint32_t received_messages;
  volatile uint32_t uart_bytes_received;
  uint16_t last_enable_seconds;
  uint16_t enable_count;
  uint16_t timeouts;
  GPSStatShowPolicy show_policy;
};

void gps_init(uint8_t enable, time_t local_y2k);
// May update current_time_ytk (with interrupts disabled)
void check_gps(volatile time_t* current_time_y2k);

// returns 1 if gpd is currently enabled
uint8_t gps_is_enabled(void);

// returns true if GPS set the position in time.h
uint8_t gps_position_was_set(void);

// returns pointer to GPSStats
const struct GPSStats* gps_get_stats(void);
// set the show policy
void gps_stat_show_policy(GPSStatShowPolicy show_policy);


#endif
