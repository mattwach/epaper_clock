#include "gps.h"
#include <nmea_decoder/nmea_decoder.h>

#ifdef DEBUG
#include <pstr/pstr.h>
#include <uart/uart.h>
#include <util/delay.h>
#endif

#include <avr/interrupt.h>
#include <avr/io.h>

#if defined(HARDWARE_UART)
#include <uart/uart.h>
#elif defined(SOFTWARE_UART)
#include <uart/software_uart.h>
#else
#error Please define HARDWARE_UART or SOFTWARE_UART
#endif

// Handles GPS parsing

// The tale of GPS vs Clock Drift
//
// The clock is mostly timed using a 32k or CPU oscillator to save on
// power consumption.  These have an imperfect frequency which will vary
// per unit.  That imperfection will accumlate as clock drift.
//
// A GPS lock corrects the clock drift but uses a lot of power when it's
// on.
//
// Some people, the "lucky ones" have low-error oscillators and excellent
// GPS signal.  Just about any strategy will work for these people.
//
// Others will have a very weak GPS signal that takes a long time to lock
// or does not lock.  In order to save on batteries, the GPS will try and 
// lock less often.  Hopefully the oscillator error is acceptable, the
// code has no way to know.
//
// Onto the algorithm.
//
// First timeout, which is the longest the GPS will run before giving up
#define GPS_GIVEUP_TIME_SECONDS 600
//
// Next we have a step unit (86400 is a day)
#define GPS_ENABLE_STEP_SECONDS 86400
// It represents the minimum amount of time to wait between GPS activations,
// even when the GPS locks quickly.  It also represents the step increment for slower
// locks as explained in a bit.
//
// The next variable is budget:
#ifdef USE_32K_CRYSTAL
#define GPS_STEP_BUDGET_SECONDS 60
#endif
#ifdef USE_CPU_CRYSTAL
#define GPS_STEP_BUDGET_SECONDS (GPS_GIVEUP_TIME_SECONDS * 2)  // Assume power usage is not a concern
#endif
// This represent how many seconds we allow the GPS to be "on" per step
// It directly controls GPS power usage.
//
//
// When GPS is disabled the "next GPS activate" equation is:
// next_activate = (1 + (gps_active_seconds / GPS_STEP_BUDGET_SECONDS)) * GPS_ENABLE_STEP_SECONDS
//
// Let understand the formula via some examples.  First, consider a
// "best case senerio" a 5 second GPS lock with some typical settings
// 
// next_activate = (1 + (6 / 30)) * 86400
//               = (1 + 0) * 86400
//               = 86400  // 1 day
//
// Worst case would be a timeout
//
// next_activate = (1 + (600 / 30)) * 86400
//               = (1 + 20) * 86400
//               = 1814400  // 21 days
//
// Other GPS lock times will fall between these extremes.  Thinking of it
// another way, GPS being "on" eats up the GPS_STEP_BUDGET_SECONDS which is
// repaid by delaying the next activation some multiple of
// GPS_ENABLE_STEP_SECONDS.
//
// Note that all of the above is for the refresh case.  The "first lock" is a
// different story because we really need it and assume that it will take a
// while.  Thus we wait as long as it takes and do not apply a budget to this
// first attempt.


// The GPS is power hungry compared to everything else so
// we have a line to turn it off.  We want to run it as little
// as possible, while still getting the data that is needed to
// set the time and location (for sunrise/sunset calculations).
#define GPS_ENABLE_DDR DDRD
#define GPS_ENABLE_PORT PORTD
#define GPS_ENABLE_PIN 6

//
// Global variables
//

// Holds a buffer for incoming GPS data and other GPS state variables
struct NMEA_decoder gps;

struct GPSStats gps_stats;

#ifdef DEBUG
volatile uint32_t uart_reported_bytes_received = 0;
volatile uint8_t last_uart_error = 0;
#endif

//
// Private structures
//

// the avr set_location() function wants postion in GPS seconds
// But the GPS unit uses the units below.  This structure works
// as a temporary hold for the data until it can be converted.
struct GPSLocation {
  uint8_t degrees;
  uint8_t minutes;
  float seconds;
  char direction;
};

// uart callbacks
// These are called whenever a byte is received on the UART from the GPS unit.


#if defined(HARDWARE_UART)

ISR(USART_RX_vect) {
  uint8_t byte = 0;
  while (read_no_block(&byte)) {
    ++gps_stats.uart_bytes_received;
    nmea_encode(&gps, (char)byte);
  }
}

#elif defined(SOFTWARE_UART)
static void suart_byte_received(void) {
  uint8_t byte = 0;
#ifdef DEBUG
  const uint8_t last_uart_error_local = software_uart_last_error();
  if (last_uart_error_local) {
    last_uart_error |= 1 << last_uart_error_local;
  }
#endif
  while (software_uart_read(&byte)) {
    ++gps_stats.uart_bytes_received;
    nmea_encode(&gps, (char)byte);
  }
}
#else
#error Please define HARDWARE_UART or SOFTWARE_UART
#endif


//
// Helper functions - these should all be marked static
//

static void enable_uart(void) {
#if defined(HARDWARE_UART) || defined(DEBUG)
  // Set the uart pin to floating
  PORTD &= ~(1 << 0);
  uint8_t uart_flags = 0;
  #ifdef HARDWARE_UART
    uart_flags |= ENABLE_RX | ENABLE_RX_INTERRUPT;
  #endif

  #ifdef DEBUG
    uart_flags |= ENABLE_TX;
  #endif

  uart_init(9600, uart_flags);
#endif

#ifdef SOFTWARE_UART
  software_uart_init(suart_byte_received);
#endif
}

static void enable_gps(const time_t current_time_y2k) {
  if (gps_is_enabled()) {
      // already enabled, abort to leave the timestamp alone
      return;
  }
  // clear out any existing GPS data since it would be leftover
  // partial data from the last enable
  nmea_init(&gps);
#ifdef SOFTWARE_UART
  software_uart_init(suart_byte_received);
#endif
  GPS_ENABLE_PORT |= (1 << GPS_ENABLE_PIN);
  // enable_time_y2k and last_enable are a bit different in that enable_time_y2k
  // points to the future when gps is disabled.
  gps_stats.enable_time_y2k = current_time_y2k;
  ++gps_stats.enable_count;
  gps_stats.last_enable = current_time_y2k;
}

static void disable_gps(const time_t current_time_y2k) {
  if (!gps_is_enabled()) {
      // already disabled, abort to leave the timestamp alone
      return;
  }
  // Even if the GPS remains powered on, constantly parsing
  // it's stream of messages can be a waste of CPU cycles.
#ifdef SOFTWARE_UART
  software_uart_disable();
#endif
  const uint16_t active_seconds = current_time_y2k - gps_stats.enable_time_y2k;
  gps_stats.last_enable_seconds = active_seconds;
  gps_stats.total_enable_seconds += gps_stats.last_enable_seconds;
  // Calculate the next enable time. (see top of file for a discussion)
  if (gps_stats.enable_count <= 1) {
    gps_stats.enable_time_y2k = current_time_y2k + GPS_ENABLE_STEP_SECONDS;
  } else {
    gps_stats.enable_time_y2k = current_time_y2k + 
      ((1 + (active_seconds / GPS_STEP_BUDGET_SECONDS)) * GPS_ENABLE_STEP_SECONDS);
  }
  GPS_ENABLE_PORT &= ~(1 << GPS_ENABLE_PIN);
}

#ifdef DEBUG
static void dump_debug_info(void) {
  if (last_uart_error || (gps_stats.uart_bytes_received > uart_reported_bytes_received)) {
    uart_str("UART_BYTES: ");
    uart_pstrln(u32_to_ps(gps_stats.uart_bytes_received));
    uart_reported_bytes_received = gps_stats.uart_bytes_received + 5000;
    uart_str("UART_ERR: ");
    uart_pstrln(u8_to_pshex(last_uart_error));
    last_uart_error = 0;
    _delay_ms(15);
  }
}
#endif


// converts DMS GPS format into GPS seconds, which is used
// by time.h set_location().
static int32_t parse_dms(const struct GPSLocation* loc) {
  int32_t seconds =
    ((int32_t)loc->degrees * 3600) +
    ((int32_t)loc->minutes * 60) +
    ((int32_t)loc->seconds);
  if ((loc->direction == 'W') || (loc->direction == 'S')) {
    seconds = -seconds;
  }
  return seconds;
}


// Parses a $GPxRMC NMEA message
static void parse_rmc(volatile time_t* current_time_y2k) {
  struct tm t;
  t.tm_isdst = 0;
  uint8_t year;

  // Try to grab time out of the GPS string.  The data might not be there,
  // in which case gps.last_error will be set by the nmea_* functions.
  gps.last_error = 0;
  nmea_rmc_time(
      &gps,
      (uint8_t*)(&t.tm_hour),
      (uint8_t*)(&t.tm_min),
      (uint8_t*)(&t.tm_sec));
  nmea_rmc_date(
      &gps, 
      (uint8_t*)(&t.tm_mday),
      (uint8_t*)(&t.tm_mon),
      &year);
  
  // correct definition differences between NMEA and the tm time structure.
  t.tm_mon -= 1;
  t.tm_year = year + 100;

  if (gps.last_error) {
    // Time is not yet available.  The GPS was likely turned on only recently.
    return;
  }

  // Take a snapshot if the current time that main/display are using.
  cli();
  time_t old_time_y2k = *current_time_y2k;
  sei();
  time_t gps_time_y2k = mk_gmtime(&t);

  uint32_t delta = gps_time_y2k > old_time_y2k ?
    gps_time_y2k - old_time_y2k :
    old_time_y2k - gps_time_y2k;
  if (delta > 60) {
    // The time was significantly changed by the GPS so reset the
    // enable time so avoid drawing false conclusions
    gps_stats.enable_time_y2k = gps_time_y2k;
  }

  // update the current time
  cli();
  *current_time_y2k = gps_time_y2k;
  sei();

  // It's commonly the case that time/date is available for a while
  // before there is a position lock.  The position lock is needed
  // to validate the time correctness on some GPS models.  It is also
  // needed for certain time calculations, like sunrise/sunset.

  struct GPSLocation loc;
  nmea_rmc_latitude_dms(
      &gps,
      &loc.degrees,
      &loc.minutes,
      &loc.seconds,
      &loc.direction);
  int32_t latitude = parse_dms(&loc);
  nmea_rmc_longitude_dms(
      &gps,
      &loc.degrees,
      &loc.minutes,
      &loc.seconds,
      &loc.direction);
  int32_t longitude = parse_dms(&loc);

  // If there was an error (such as data not available), then latitude and
  // longitude will be invalid.
  if (gps.last_error == 0) {
    // sucessfully got the position
    set_position(latitude, longitude);
    gps_stats.last_lock =  gps_time_y2k;
    disable_gps(gps_time_y2k);
  }
}

//
// Public interface
//

void gps_init(uint8_t enable, time_t local_y2k) {
  gps_stats.last_lock = 0;
  nmea_init(&gps);

  // uart is always enabled for the debug case
  enable_uart();

  GPS_ENABLE_PORT &= ~(1 << GPS_ENABLE_PIN);
  GPS_ENABLE_DDR |= (1 << GPS_ENABLE_PIN);
  if (enable) {
    gps_stats.last_enable = local_y2k;
    enable_gps(local_y2k);
  } else {
    disable_gps(local_y2k);
    gps_stats.enable_time_y2k = local_y2k + 900;  // 10 minutes into the future
  }
}


static void check_for_timeout(time_t time_y2k) {
 if ((time_y2k > (gps_stats.enable_time_y2k + GPS_GIVEUP_TIME_SECONDS)) && gps_is_enabled()) {
    ++gps_stats.timeouts;
    if (gps_stats.last_lock > 0) {
      // give up to save on battery power.
      disable_gps(time_y2k);
    } else {
      // If we never got a lock there is no point in disabling GPS since
      // we don't have good data to work with.
      // reset enable time so that the timeout stat doesn't count up every second.
      const uint16_t active_seconds = time_y2k - gps_stats.enable_time_y2k;
      gps_stats.last_enable_seconds = active_seconds;
      gps_stats.total_enable_seconds += active_seconds;
      gps_stats.enable_time_y2k = time_y2k;
    }
  }
}

// called in the main loop to unload messages from the GPS buffer memory and
// parse messages that are of interest.
void check_gps(volatile time_t* current_time_y2k) {
  cli();
  const time_t local_y2k = *current_time_y2k;
  sei();
  // the GPS unit will send a lot of strings but this code is only interested
  // in certain ones.  Still, the logic requires the commands be marked "done"
  // to allow the space in the ring-buffer to be freed.
#ifdef DEBUG
  dump_debug_info();
#endif
  if (gps_is_enabled()) {
    for (; gps.ready; nmea_command_done(&gps)) {
      ++gps_stats.received_messages;
      if (nmea_is_rmc(&gps)) {
        parse_rmc(current_time_y2k);
      }
    }

    check_for_timeout(local_y2k);
  } else if (local_y2k >= gps_stats.enable_time_y2k) {
    enable_gps(local_y2k);
  }
}

uint8_t gps_is_enabled(void) {
  return (GPS_ENABLE_PORT & (1 << GPS_ENABLE_PIN)) != 0;
}

// Returns 1 if the gps position was set
uint8_t gps_position_was_set(void) {
  return gps_stats.last_lock != 0;
}

const struct GPSStats* gps_get_stats(void) {
  return &gps_stats;
}

void gps_stat_show_policy(GPSStatShowPolicy show_policy) {
  gps_stats.show_policy = show_policy;
}
