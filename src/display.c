#include "display.h"
#include <oledm/oledm_spi.h>
#include <oledm/epaper.h>
#include <oledm/text.h>
#include <pstr/pstr.h>

#include <time.h>
#include "clock_number_font.h"
#include "detail_numbers_font.h"
#include "gps.h"
#include "gps_stats_font.h"
#include "labels_font.h"
#include "pressure_font.h"
#include "pressure_graph.h"
#include "sun_moon_icons_light.h"
#include "sun_moon_icons_dark.h"

#include <avr/interrupt.h>

// This module is resposible for rendering time/weather data on the epaper display
// It does not collect the data, only displays it.

// MOSI and SCK help control the epaper display via SPI
#define MOSI_DDR DDRB
#define MOSI_PORT PORTB
#define MOSI_PIN 3  // Nano pin 11

#define SCK_DDR DDRB
#define SCK_PORT PORTB
#define SCK_PIN 5  // Nano pin 13

//
// UI Positioning constants.  These all assume a 296x128 (16 row) display.
//

#define AM_PM_COLUMN 173
#define AM_PM_ROW 7

#define DATE_COLUMN 209

#define VLINE_DATE_COLUMN 205

#define SUN_COLUMN 270
#define SUNRISE_TIME_ROW 3
#define SUNSET_TIME_ROW 6

#define TEMPERATURE_ROW 10
#define TEMPERATURE_COLUMN 233

#define HUMIDITY_ROW 13

#define VLINE_TEMPERATURE_COLUMN 230

#define MAX_PRESSURE_ROW 10
#define PRESSURE_ROW 12
#define MIN_PRESSURE_ROW 14
#define PRESSURE_COLUMN 154

#define GPS_STATS_ROW 10

//
// Global Vars
//

// epaper state
struct OLEDM display;
// font state
static struct Text text;

// This enum maps the incon indexes as defined in sun_moon_icons.c
typedef enum {
  SUN_ICON = 0,
  NEW_MOON = 1,
  WAXING_CRESCENT = 2,
  FIRST_QUARTER = 3,
  WAXING_GIBBOUS = 4,
  FULL_MOON = 5,
  WANING_GIBBOUS = 6,
  LAST_QUARTER = 7,
  WANING_CRESCENT = 8,
} StellarIcon;


// Calculating sunrise/sunset every minute is a waste of battery resources
// we can just calculate it once per day and hold it in SRAM.
#define SUNRISE_RECALC_HOUR 0
#define SUNRISE_RECALC_MINUTE 1
uint8_t sunrise_hour;
uint8_t sunrise_minute;
uint8_t sunset_hour;
uint8_t sunset_minute;
uint8_t moon_icon;

//
// Helpers - these should all be declared static
//

// Renders a 2 digit number with a leading zero.
// i.e. 8 -> '08' but 13 -> '13'
static void render_u8_02(uint8_t v) {
  if (v < 10) {
    text_char(&text, '0');
  }
  text_pstr(&text, u8_to_ps(v));
}

// Renders a 2 digit number with a leading space.
// i.e. 8 -> ' 8' but 13 -> '13'
static void render_u8_sp2(uint8_t v) {
  if (v < 10) {
    text_char(&text, ' ');
  }
  text_pstr(&text, u8_to_ps(v));
}


// Renders a number and units while trying to make use of space.
// number columns is the number of available columns not including the units.
//
// Depending on the value and the number columns, many different renderings are possible
// say we have 5 columns and the unit is 'C', all of the following are possible
//
// 25.6C
//  -25C
//  0.0C
//  100C
static void render_i32x100(
    int32_t v,
    const char* units,
    int8_t number_columns,
    const uint8_t* number_font,
    const uint8_t* label_font) {
  const uint8_t is_negative = v < 0;
  if (is_negative) {
    v = -v;
    --number_columns;  // negative takes a column
  }

  // each integetr digit takes a column
  for (uint32_t tempv = v/100; tempv >= 10; tempv /= 10) {
    number_columns -= 1;
  }
  number_columns -= 1;

  // make leading spaces.  1 is a special case because it's too narrow for
  // a decimal so we use a space and whole number for this case
  text.font = label_font;
  for (; number_columns == 1 || number_columns >= 3; --number_columns) {
    text_char(&text, ' ');
  }

  text.font = number_font;
  if (is_negative) {
    text_char(&text, '-');
  }

  text_pstr(&text, u32_to_ps(v / 100));

  if (number_columns >= 2) {
    text_char(&text, '.');
    const uint8_t r = (uint8_t)(v % 100) / 10;
    text_pstr(&text, u8_to_ps(r));
  }

  text.font = label_font;
  text_str(&text, units);
}

// Draws a vertical line.  end_row is inclusive
static void vline(column_t column, uint8_t start_row, uint8_t end_row) {
  oledm_set_bounds(&display, column, start_row, column, end_row);

  oledm_start_pixels(&display);
  for (uint8_t row=start_row; row <= end_row; ++row) {
    oledm_write_pixels(&display, 0xFF);
  }
  oledm_stop(&display);
}

// Renders either a 12h or 24h time to the display.
static void render_hour(uint8_t hour, uint8_t use_24h_time) {
  if (use_24h_time) {
    render_u8_02(hour);
  } else {
    if (hour > 12) {
      hour -= 12;
    }
    if (hour == 0) {
      hour = 12;
    }
    render_u8_sp2(hour);
  }
}

// Calculates the moon phase and chooses an icon (stored in the global
// variable moon_icon).
static void recalc_moon_phase(const time_t time_y2k) {
  int8_t phase = moon_phase(&time_y2k);
  // we have a range of -100 to 100.  I assume negative numbers are waning
  if (phase <= -87) {
    moon_icon = FULL_MOON;
  } else if (phase <= -63) {
    moon_icon = WANING_GIBBOUS;
  } else if (phase <= -38) {
    moon_icon = LAST_QUARTER;
  } else if (phase <= -13) {
    moon_icon = WANING_CRESCENT;
  } else if (phase <= 13) {
    moon_icon = NEW_MOON;
  } else if (phase <= 38) {
    moon_icon = WAXING_CRESCENT;
  } else if (phase <= 64) {
    moon_icon = FIRST_QUARTER;
  } else if (phase <= 87) {
    moon_icon = WAXING_GIBBOUS;
  } else {
    moon_icon = FULL_MOON; 
  }
}

// Calculates sunrise/sunset and stores them in global variables.
static void calc_sunrise_sunset(const time_t time_y2k) {
  struct tm t;
  time_t time = sun_rise(&time_y2k);
  localtime_r(&time, &t);
  sunrise_hour = t.tm_hour;
  sunrise_minute = t.tm_min;

  time = sun_set(&time_y2k);
  localtime_r(&time, &t);
  sunset_hour = t.tm_hour;
  sunset_minute = t.tm_min;
}

// converts current_time_ytk into a human-readable time and
// renders it on the epaper display which is assumed to be
// ready to accept commands.
static void render_time(
    const time_t time_y2k,
    uint8_t position_was_set,
    uint32_t pressure_pa,
    const struct EEPromVars* eeprom) {
  struct tm t;
  localtime_r(&time_y2k, &t);

  if (!pressure_graph_has_data()) {
    // First call.  Initialize all points to the current value
    // to form a baseline.
    for (column_t i=0; i<PRESSURE_GRAPH_COLS; ++i) {
      pressure_graph_add_point(pressure_pa);
    }
  } else if ((t.tm_min % 10) == 0) {
      pressure_graph_add_point(pressure_pa);
  }

  const uint8_t use_24h_time = (eeprom->option_bits & OPTION_USE_24H_TIME);

  if (position_was_set &&
      ((sunrise_hour == 0) ||
       ((t.tm_hour == SUNRISE_RECALC_HOUR) && (t.tm_min == SUNRISE_RECALC_MINUTE)))) {
    // Recalculate sunrise/sunset if they have never been set or if it's the trigger
    // point for the current day (currently 12:01 AM but check the constants to
    // be sure about that.)
    calc_sunrise_sunset(time_y2k);
    recalc_moon_phase(time_y2k);
  }

  // Rander the time to the display
  text.font = clock_number_font;
  text.row = 0;
  // 24h time is shifted over because there is no AM/PM to render
  text.column = use_24h_time ? 12 : 0;
  render_hour(t.tm_hour, use_24h_time);
  text_char(&text, ':');
  render_u8_02(t.tm_min);

  if (!use_24h_time) {
    // Render AM or PM to complete the 12h time.
    text.font = labels_font;
    text.row = AM_PM_ROW;
    text.column = AM_PM_COLUMN;
    text_char(&text, t.tm_hour >= 12 ? 'P' : 'A');
    text_char(&text, 'M');
  }

  // Separate time and date with a vertical line
  vline(VLINE_DATE_COLUMN, 0, 9);

  text.row = 0;
  text.column = DATE_COLUMN;
  // Render the current date in either YY/MM/DD or DD/MM/YY format.
  const bool_t use_english = (eeprom->option_bits & OPTION_USE_METRIC) == 0; 
  const uint8_t year = t.tm_year % 100; 
  const uint8_t month = t.tm_mon + 1;
  text.font = detail_numbers_font;

  render_u8_02(use_english ? month : year);
  text.font = labels_font;
  text_char(&text, '/');
  text.font = detail_numbers_font;
  render_u8_02(use_english ? t.tm_mday : month);
  text.font = labels_font;
  text_char(&text, '/');
  text.font = detail_numbers_font;
  render_u8_02(use_english ? year : t.tm_mday);
}

// Time rendering function that will render either sunrise or sunset time
static void render_sunrise_sunset_numbers(
    const uint8_t hour,
    const uint8_t minute,
    const uint8_t use_24h_time,
    const StellarIcon icon,
    const uint8_t dark_mode) {
  text.column = DATE_COLUMN;
  text.font = detail_numbers_font;
  render_hour(hour, use_24h_time);
  text.font = labels_font;
  text_char(&text, ':');
  text.font = detail_numbers_font;
  render_u8_02(minute);
  text.column = SUN_COLUMN;
  text.font = dark_mode ? sun_moon_icons_dark : sun_moon_icons_light;
  text_char(&text, icon);
}

// Renders both sunrise and sunset times (and associated icons)
static void render_sunrise_sunset(
    const time_t time_y2k,
    const struct EEPromVars* eeprom) {
  const uint8_t use_24h_time = (eeprom->option_bits & OPTION_USE_24H_TIME);
  const uint8_t dark_mode = eeprom->option_bits & OPTION_DARK_MODE;
  text.row = SUNRISE_TIME_ROW;
  render_sunrise_sunset_numbers(
      sunrise_hour, sunrise_minute, use_24h_time, SUN_ICON, dark_mode);
  text.row = SUNSET_TIME_ROW;
  render_sunrise_sunset_numbers(
      sunset_hour, sunset_minute, use_24h_time, moon_icon, dark_mode);
}

// Renders PTH (Pressure/Time/Humidity) values.
static void render_pth(
    uint16_t humidity_cpct,
    uint32_t pressure_pa,
    int16_t temp_cc,
    const struct EEPromVars* eeprom) {
  const bool_t use_english = (eeprom->option_bits & OPTION_USE_METRIC) == 0; 

  // Get the min/max pressure from the graph's POV.
  uint32_t min_pressure_pa = pressure_graph_min_pa();
  uint32_t max_pressure_pa = pressure_graph_max_pa();

  // pressure_pa is updated every minute while the graph is updated every 10
  // minutes.  Thus is possible for pressure_pa to wander outside of the
  // graph's min/max
  if (pressure_pa < min_pressure_pa) {
    min_pressure_pa = pressure_pa;
  } else if (pressure_pa > max_pressure_pa) {
    max_pressure_pa = pressure_pa;
  }

  // Assert there is at least 40 PA before showing the graph, otherwise
  // the graph just looks like a bunch of noise due to over-magnification.
  uint8_t show_pressure_graph = (max_pressure_pa - min_pressure_pa) >= 40;

  if (use_english) {
    // convert pa to inches.  The variables are still called cc and pa, which
    // might make the code more confusing to read, but anyone looking at the
    // display will not know/care.
    temp_cc = (uint16_t)(((uint32_t)temp_cc) * 9 / 5 + 3200);
    min_pressure_pa = (min_pressure_pa * 1000) / 33864;
    max_pressure_pa = (max_pressure_pa * 1000) / 33864;
    pressure_pa = (pressure_pa * 1000) / 33864;
  }

  // Render the temperature
  text.row = TEMPERATURE_ROW;
  text.column = TEMPERATURE_COLUMN;
  render_i32x100(
      temp_cc,
      use_english ? "F" : "C",
      4,
      detail_numbers_font,
      labels_font);

  // render the humidity
  text.row = HUMIDITY_ROW;
  text.column = TEMPERATURE_COLUMN;
  render_i32x100(
      humidity_cpct,
      "%rH",
      2,
      detail_numbers_font,
      labels_font);

  // put a vertical line between temperature/humidity and the pressure graph
  vline(VLINE_TEMPERATURE_COLUMN, 10, 15);

  // render minimum, maximum and current pressure values
  text.row = PRESSURE_ROW;
  text.column = PRESSURE_COLUMN;
  render_i32x100(
      pressure_pa,
      use_english ? " in" : " hPa",
      4,
      pressure_font,
      pressure_font);

  text.row = MAX_PRESSURE_ROW;
  text.column = PRESSURE_COLUMN;
  render_i32x100(
      max_pressure_pa,
      " max",
      4,
      pressure_font,
      pressure_font);

  text.row = MIN_PRESSURE_ROW;
  text.column = PRESSURE_COLUMN;
  render_i32x100(
      min_pressure_pa,
      " min",
      4,
      pressure_font,
      pressure_font);

  if (show_pressure_graph) {
    pressure_graph_plot();
  }
}

void render_gps_stats(const time_t time_y2k) {
  const struct GPSStats* gps_stats = gps_get_stats();
  if ((gps_stats->show_policy == GPS_STATS_AUTO) &&
      (gps_stats->last_lock > 0)) {
    return;
  }
  if (gps_stats->show_policy == GPS_STATS_HIDE) {
    return;
  }
  text.font = gps_stats_font;

  text.row = GPS_STATS_ROW;
  text.column = 0;
  uint16_t active_seconds = 0;
  if (time_y2k >= gps_stats->enable_time_y2k) {
    active_seconds = time_y2k - gps_stats->enable_time_y2k;
    text_str(&text, "GPS: ON FOR ");
    text_pstr(&text, u32_to_ps(active_seconds));
  } else {
    text_str(&text, "GPS: NEXT ");
    text_pstr(&text, u32_to_ps(gps_stats->enable_time_y2k - time_y2k));
  }

  ++text.row;
  text.column = 0;
  if (gps_stats->last_enable > 0) {
    text_str(&text, "LAST EN: ");
    text_pstr(&text, u32_to_ps(time_y2k - gps_stats->last_enable)); 
  }
  if (gps_stats->last_lock > 0) {
    text_str(&text, " LAST LOCK: ");
    text_pstr(&text, u32_to_ps(time_y2k - gps_stats->last_lock)); 
  } else {
    text_str(&text, " LOCKING...");
  }

  ++text.row;
  text.column = 0;
  text_str(&text, "EN COUNT: ");
  text_pstr(&text, u16_to_ps(gps_stats->enable_count)); 
  text_str(&text, " TIMEOUT: ");
  text_pstr(&text, u16_to_ps(gps_stats->timeouts)); 

  ++text.row;
  text.column = 0;
  text_str(&text, "EN S TOT: ");
  text_pstr(&text, u32_to_ps(gps_stats->total_enable_seconds + active_seconds)); 
  text_str(&text, " LAST: ");
  text_pstr(&text, u16_to_ps(gps_stats->last_enable_seconds + active_seconds)); 

  if (gps_stats->enable_count > 0) {
    ++text.row;
    text.column = 0;
    text_str(&text, "EN S AVG: ");
    text_pstr(&text, u16_to_ps((gps_stats->total_enable_seconds + active_seconds) / gps_stats->enable_count)); 
  }

  ++text.row;
  text.column = 0;
  text_str(&text, "MESSAGES: ");
  text_pstr(&text, u32_to_ps(gps_stats->received_messages)); 
  text_str(&text, " UART: ");
  cli();
  uint32_t uart_bytes_received = gps_stats->uart_bytes_received;
  sei();
  text_pstr(&text, u32_to_ps(uart_bytes_received)); 
}

//
// Interface Implementation
//

// one-time initialization of epaper display and text object
void display_init(void) {
  sunrise_hour = 0;
  oledm_basic_init(&display);
  text_init(&text, clock_number_font, &display);
  pressure_graph_init(&display);
}

// Called to power down the SPI port that communicates with the
// epaper display.
void display_disable_spi(void) {
  // Set every pin to an input.  If we do not do this then
  // there is parasitic current draw through the these pins
  // of a couple of mA, at least for my waveshare unit (measured
  // with a multimeter).
  MOSI_DDR &= ~(1 << MOSI_PIN);
  SCK_DDR &= ~(1 << SCK_PIN);
  CS_DDR &= ~(1 << CS_PIN);
  DC_DDR &= ~(1 << DC_PIN);
  RES_DDR &= ~(1 << RES_PIN);

  // also disable any pullup resistors to save more power.
  MOSI_PORT &= ~(1 << MOSI_PIN);
  SCK_PORT &= ~(1 << SCK_PIN);
  CS_PORT &= ~(1 << CS_PIN);
  DC_PORT &= ~(1 << DC_PIN);
  RES_PORT &= ~(1 << RES_PIN);
}

// Power back up the SPI port so that communications with the
// epaper display can resume.
void display_enable_spi(void) {
  // After wake-up from sleep, bring everything back
  MOSI_DDR |= (1 << MOSI_PIN);
  SCK_DDR |= (1 << SCK_PIN);
  CS_DDR |= (1 << CS_PIN);
  DC_DDR |= (1 << DC_PIN);
  RES_DDR |= (1 << RES_PIN);
}

// When this is called, the epaper display is updated
void update_display(
    const struct DisplayInfo* dinfo,
    const struct EEPromVars* eeprom,
    void (*wait_for_next_second)(void)) {
  // Get ready
  if (eeprom->option_bits & OPTION_DARK_MODE) {
    display.option_bits |= OLEDM_WHITE_ON_BLACK;
  } else {
    display.option_bits &= ~OLEDM_WHITE_ON_BLACK;
  }
  oledm_start(&display);

  // Render
  oledm_clear(&display, 0x00);
  render_time(
      dinfo->time_y2k,
      dinfo->position_was_set,
      dinfo->pressure_pa,
      eeprom);
  if (dinfo->position_was_set) {
    render_sunrise_sunset(dinfo->time_y2k, eeprom);
  }
  render_pth(
      dinfo->humidity_cpct,
      dinfo->pressure_pa,
      dinfo->temp_cc,
      eeprom);

  render_gps_stats(dinfo->time_y2k);

  // The extra steps are in place to minimize power usage.
  epaper_swap_buffers_no_wait(&display);
  // try to save a little power while waiting for the epaper to
  // do it's update dance.
  wait_for_next_second();
  wait_for_next_second();
  epaper_wait(&display);

  // Deep sleep until next time
  epaper_sleep_mode(&display, SLEEP_MODE_2);
}

struct OLEDM* display_device(void) {
  return &display;
}

void display_recalc_sunrise_sunset(void) {
  sunrise_hour = 0;
}
