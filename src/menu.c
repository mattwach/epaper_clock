#include "display.h"
#include <oledm/oledm_spi.h>
#include <oledm/epaper.h>
#include <oledm/text.h>
#include <pstr/pstr.h>

#include "buttons.h"
#include "display.h"
#include "eeprom_vars.h"
#include "pressure_font.h"

typedef enum {
  FINISHED = 0,
  INCREMENT_TIMEZONE = 1,
  DECREMENT_TIMEZONE = 2,
  DARK_MODE = 3,
  USE_24H_TIME = 4,
  USE_METRIC = 5,
} MenuRows;
#define LAST_MODE USE_METRIC

void init_display_for_partial_refresh(time_t current_y2k, struct EEPromVars* eeprom);

#define TIME_CHAR_WIDTH 9
#define STARTING_COLUMN 50
#define TEXT_HEIGHT 2

//
// Global Vars
//

uint8_t selected_row;

// font state
static struct Text text;

static void render_increment_timezone(
    uint8_t select_pressed,
    struct EEPromVars* eeprom) {
  if (select_pressed) {
    if (eeprom->utc_offset >= 14) {
      eeprom->utc_offset = -12;
    } else {
      ++eeprom->utc_offset;
    }
    set_zone(((int32_t)eeprom->utc_offset) * ONE_HOUR);
  }
  text_str(&text, "TIMEZONE+1");
}

static void render_decrement_timezone(
    uint8_t select_pressed,
    struct EEPromVars* eeprom) {
  if (select_pressed) {
    if (eeprom->utc_offset <= -12) {
      eeprom->utc_offset = 14;
    } else {
      --eeprom->utc_offset;
    }
    set_zone(((int32_t)eeprom->utc_offset) * ONE_HOUR);
  }
  text_str(&text, "TIMEZONE-1");
}

static void render_dark_mode(
    uint8_t select_pressed,
    struct EEPromVars* eeprom) {
  if (select_pressed) {
    eeprom->option_bits ^= OPTION_DARK_MODE;
    struct OLEDM* display = display_device();
    if (eeprom->option_bits & OPTION_DARK_MODE) {
      display->option_bits |= OLEDM_WHITE_ON_BLACK;
    } else {
      display->option_bits &= ~OLEDM_WHITE_ON_BLACK;
    }
  }
  text_str(
      &text,
      (eeprom->option_bits & OPTION_DARK_MODE) ?
      "DARK MODE" :
      "LIGHT MODE");
}

static void render_24h_time(
    uint8_t select_pressed,
    struct EEPromVars* eeprom) {
  if (select_pressed) {
    eeprom->option_bits ^= OPTION_USE_24H_TIME;
  }
  text_str(
      &text,
      (eeprom->option_bits & OPTION_USE_24H_TIME) ?
      "24H TIME" :
      "12H TIME");
}

static void render_use_metric(
    uint8_t select_pressed,
    struct EEPromVars* eeprom) {
  if (select_pressed) {
    eeprom->option_bits ^= OPTION_USE_METRIC;
  }
  text_str(
      &text,
      (eeprom->option_bits & OPTION_USE_METRIC) ?
      "METRIC UNITS" :
      "ENGLISH UNITS");
}

static void render_finished(
    uint8_t select_pressed,
    struct EEPromVars* eeprom) {
  text_str(&text, "FINISHED");
}

void (*render_functions[])(uint8_t, struct EEPromVars*) = {
  render_finished,
  render_increment_timezone,
  render_decrement_timezone,
  render_dark_mode,
  render_24h_time,
  render_use_metric,
};

 
static void render_u8_02(uint8_t v) {
  if (v < 10) {
    text_char(&text, '0');
  }
  text_pstr(&text, u8_to_ps(v));
}

static void render_u8_sp2(uint8_t v) {
  if (v < 10) {
    text_char(&text, ' ');
  }
  text_pstr(&text, u8_to_ps(v));
}

void render_time(time_t current_y2k, uint8_t option_bits) {
  struct tm t;
  localtime_r(&current_y2k, &t);
  text.row = 0;
  const uint8_t use_24h_time = (option_bits & OPTION_USE_24H_TIME);
  const uint8_t time_chars = use_24h_time ? 5 : 8;
  text.column = (display_device()->visible_columns - time_chars * TIME_CHAR_WIDTH) / 2;

  if (use_24h_time) {
    render_u8_02(t.tm_hour);
  } else {
    uint8_t hour = t.tm_hour;
    if (hour > 12) {
      hour -= 12;
    }
    if (hour == 0) {
      hour = 12;
    }
    render_u8_sp2(hour);
  }
  text_char(&text, ':');
  render_u8_02(t.tm_min);

  if (!use_24h_time) {
    text_char(&text, ' ');
    text_char(&text, t.tm_hour >= 12 ? 'P' : 'A');
    text_char(&text, 'M');
  }
}


// When this is called, the epaper display is updated
static uint8_t update_menu_internal(
    uint8_t button_pressed,
    time_t current_y2k,
    struct EEPromVars* eeprom,
    uint8_t partial_update) {
  if (button_pressed & OPTION_WAS_PRESSED) {
    ++selected_row;
    if (selected_row > LAST_MODE) {
      selected_row = 0;
    }
  }

  struct OLEDM* display = display_device();
  if (partial_update) {
    epaper_set_partial_mode(display);
  }
  oledm_clear(display, 0x00);

  text.row = TEXT_HEIGHT;
  const uint8_t select_pressed = button_pressed & SELECT_WAS_PRESSED;
  for (uint8_t i=0; i <= LAST_MODE; ++i, text.row += TEXT_HEIGHT) {
    text.column = STARTING_COLUMN;
    text.options = (i == selected_row) ? TEXT_OPTION_INVERTED : 0;
    render_functions[i](select_pressed && (i == selected_row), eeprom);
  }

  text.options = 0;
  render_time(current_y2k, eeprom->option_bits);

  if (partial_update) {
    epaper_update_partial(display, SLEEP_MODE_1);
  }

  if (select_pressed) {
    save_eeprom(eeprom);

    if (selected_row == DARK_MODE) {
      // reinit now
      oledm_start(display);
      // Note this is recursive but should not go infinite loop becuase it always passes 0
      // for the button_pressed argument
      init_display_for_partial_refresh(current_y2k, eeprom);
    } else if (selected_row == FINISHED) {
      display_recalc_sunrise_sunset();
      return 0;
    }
  }

  return 1;
}

void init_display_for_partial_refresh(
    time_t current_y2k, struct EEPromVars* eeprom) {
  // need to initially render on both the front and back displays
  struct OLEDM* display = display_device();
  if (eeprom->option_bits & OPTION_DARK_MODE) {
    display->option_bits |= OLEDM_WHITE_ON_BLACK;
  } else {
    display->option_bits &= ~OLEDM_WHITE_ON_BLACK;
  }
  oledm_start(display);
  update_menu_internal(0, current_y2k, eeprom, 0);
  display->option_bits |= OLEDM_WRITE_COLOR_RAM;
  update_menu_internal(0, current_y2k, eeprom, 0);
  display->option_bits &= ~OLEDM_WRITE_COLOR_RAM;
  epaper_swap_buffers(display, SLEEP_MODE_0);
}



uint8_t update_menu(
    uint8_t button_pressed,
    time_t current_y2k,
    struct EEPromVars* eeprom) {
  return update_menu_internal(button_pressed, current_y2k, eeprom, 1);
}

// Initialization of epaper display and text object.  Called each time the
// menu is entered.
void menu_init(time_t current_y2k, struct EEPromVars* eeprom) {
  selected_row = 0;
  struct OLEDM* display = display_device();
  text_init(&text, pressure_font, display);
  init_display_for_partial_refresh(current_y2k, eeprom);
}

