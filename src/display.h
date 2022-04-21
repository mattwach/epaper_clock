#ifndef DISPLAY_H
#define DISPLAY_H

#include "eeprom_vars.h"
#include <oledm/oledm.h>
#include <inttypes.h>
#include <time.h>

// Data collected by the caller that this module may render.
struct DisplayInfo {
  uint16_t humidity_cpct;
  uint32_t pressure_pa;
  int16_t temp_cc;
  time_t time_y2k;
  uint8_t position_was_set;  // 0|1
};

// Called as a part of power up
void display_init(void);

// Called whenever the display should be updated.  Expected to be around once
// per minute.
void update_display(
    const struct DisplayInfo* dinfo,
    const struct EEPromVars* eeprom,
    void (*wait_for_next_second)(void));

// Used to enable/disable the SPI hardware to avoid
// leaking current through SPI pins.
void display_enable_spi(void);
void display_disable_spi(void);

// Shares display device (with the menu)
struct OLEDM* display_device(void);

// call this to recalculate the sunrise/sunset hour.  This
// normally happens automatically but needs a nudge by the menu
// in the case where the UTC offset was changed.
void display_recalc_sunrise_sunset(void);

#endif

