#ifdef UNIT_TEST
#define SSD1680
#endif

#include "epaper.h"

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <error_codes.h>
#include "oledm_spi.h"
#include "oledm_driver_common.inc"
#include <util/delay.h>

#ifndef SSD1680
#error "Please define SSD1680 for all modules when using ssd1680.o"
#endif

enum ESsd1608Commands
{
    SSD1680_DRIVER_OUTPUT            = 0x01,
    SSD1680_GATE_VOLTAGE             = 0x03,
    SSD1680_SOURCE_VOLTAGE           = 0x04,
    SSD1680_DEEP_SLEEP_MODE          = 0x10,
    SSD1680_DATA_ENTRY_MODE          = 0x11,
    SSD1680_SW_RESET                 = 0x12,
    SSD1680_MASTER_ACTIVATION        = 0x20,
    SSD1680_DISPLAY_UPDATE_CONTROL1  = 0x21,
    SSD1680_DISPLAY_UPDATE_CONTROL2  = 0x22,
    SSD1680_WRITE_RAM_BW             = 0x24,
    SSD1680_WRITE_RAM_RED            = 0x26,
    SSD1680_VCOM_SENSE               = 0x28,
    SSD1680_VCOM_SENSE_DURATION      = 0x29,
    SSD1680_PROGRAM_VCOM_OTP         = 0x2A,
    SSD1680_WRITE_REGISTER_VCOM_CTL  = 0x2B,
    SSD1680_WRITE_VCOM_REGISTER      = 0x2C,
    SSD1680_OTP_REGISTER_READ        = 0x2D,
    SSD1680_USER_READ                = 0x2E,
    SSD1680_PROGRAM_WS_OTP           = 0x30,
    SSD1680_LOAD_WS_OTP              = 0x31,
    SSD1680_WRITE_LUT_REGISTER       = 0x32,
    SSD1680_PROGRAM_OTP_SELECTION    = 0x36,
    SSD1680_WRITE_REG_FOR_DISPLAY    = 0x37,
    SSD1680_WRITE_REGISTER_FOR_UID   = 0x38,
    SSD1680_OTP_PROGRAM_MODE         = 0x39,
    SSD1680_BORDER_WAVEFORM_CONTROL  = 0x3C,
    SSD1680_END_OPTION               = 0x3F,
    SSD1680_SET_X_START_END          = 0x44,
    SSD1680_SET_Y_START_END          = 0x45,
    SSD1680_SET_X_ADDRESS_COUNTER    = 0x4E,
    SSD1680_SET_Y_ADDRESS_COUNTER    = 0x4F,
};

#define DISPLAY_MEMORY_ROWS 16
#define DISPLAY_MEMORY_COLUMNS 296

#ifndef BUSY_PIN
#define BUSY_DDR DDRD
#define BUSY_PORT PORTD
#define BUSY_INPUT PIND
#define BUSY_PIN 7  // D7
#endif

const uint8_t WF_PARTIAL_2IN9[159] PROGMEM = {
    0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x40, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x00, 0x00,
    0x00, 0x22, 0x17, 0x41, 0xB0, 0x32, 0x36,
};

const uint8_t WS_20_30[159] PROGMEM = {
    0x80, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x40, 0x00, 0x00, 0x00, 0x10, 0x66, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,
    0x80, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x40, 0x00, 0x00, 0x00, 0x10, 0x66, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x14, 0x08, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x0A, 0x0A, 0x00, 0x0A, 0x0A,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x14, 0x08, 0x00, 0x01,
    0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x00, 0x00,
    0x00, 0x22, 0x17, 0x41, 0x00, 0x32, 0x36
};

const uint8_t WRITE_REG_FOR_DISPLAY_PARTIAL[10] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00,
    0x00, 0x00
};

static inline uint8_t reverse_byte(uint8_t x) {
    x = ((x >> 1) & 0x55) | ((x << 1) & 0xaa);
    x = ((x >> 2) & 0x33) | ((x << 2) & 0xcc);
    x = ((x >> 4) & 0x0f) | ((x << 4) & 0xf0);
    return x;
}

uint8_t epaper_is_busy(void) {
  return (BUSY_INPUT & (1 << BUSY_PIN)) != 0;
}

#define BUSY_TIMEOUT_MS 4000
void epaper_wait(struct OLEDM* display) {
  error_t* err = &(display->error);
  uint16_t i = 0;
  for (;
       i < BUSY_TIMEOUT_MS && (BUSY_INPUT & (1 << BUSY_PIN));
       ++i) {
    _delay_ms(1);
  }

  _delay_ms(4);
  if (i >= BUSY_TIMEOUT_MS) {
    *err = NO_ACK_ERROR;
  }
}

static inline void command_data() {
  DC_PORT |= (1 << DC_PIN);
  CS_PORT &= ~(1 << CS_PIN);
}

void oledm_basic_init(struct OLEDM* display) {
  memset(display, 0, sizeof(struct OLEDM));
  display->visible_columns = DISPLAY_MEMORY_COLUMNS;
  display->visible_rows = DISPLAY_MEMORY_ROWS;
  display->memory_columns = DISPLAY_MEMORY_COLUMNS;
  display->memory_rows = DISPLAY_MEMORY_ROWS;
  display->option_bits = 0x00;
}

static void set_lut(struct OLEDM* display, const uint8_t* lut) {
  error_t* err = &(display->error);

  oledm_startCommands(err);
  oledm_command(SSD1680_WRITE_LUT_REGISTER, err);
  command_data();
  uint8_t idx = 0;
  for (; idx < 153; ++idx) {
    oledm_command(pgm_read_byte_near(&lut[idx]), err);
  }
  oledm_stop(display);
  epaper_wait(display);
}

void epaper_reset(void) {
  RES_PORT &= ~(1 << RES_PIN);
  _delay_ms(2);
  RES_PORT |= (1 << RES_PIN);
  _delay_ms(2);
}

void epaper_set_full_mode(struct OLEDM* display) {
  // Do we need a reset here, like partial mode?
  set_lut(display, WS_20_30);
  error_t* err = &(display->error);
  oledm_startCommands(err);
  oledm_command(SSD1680_END_OPTION, err);
  command_data();
  oledm_command(pgm_read_byte_near(&WS_20_30[153]), err);
  oledm_stop(display);

  oledm_startCommands(err);
  oledm_command(SSD1680_GATE_VOLTAGE, err);
  command_data();
  oledm_command(pgm_read_byte_near(&WS_20_30[154]), err);
  oledm_stop(display);

  oledm_startCommands(err);
  oledm_command(SSD1680_SOURCE_VOLTAGE, err);
  command_data();
  oledm_command(pgm_read_byte_near(&WS_20_30[155]), err);
  oledm_command(pgm_read_byte_near(&WS_20_30[156]), err);
  oledm_command(pgm_read_byte_near(&WS_20_30[157]), err);
  oledm_stop(display);

  oledm_startCommands(err);
  oledm_command(SSD1680_WRITE_VCOM_REGISTER, err);
  command_data();
  oledm_command(pgm_read_byte_near(&WS_20_30[158]), err);
  oledm_stop(display);
}

void epaper_set_partial_mode(struct OLEDM* display) {
  error_t* err = &(display->error);

  epaper_reset();
  set_lut(display, WF_PARTIAL_2IN9);

  oledm_startCommands(err);
  oledm_command(SSD1680_WRITE_REG_FOR_DISPLAY, err);
  command_data();
  for (uint8_t i=0; i<sizeof(WRITE_REG_FOR_DISPLAY_PARTIAL); ++i) {
    oledm_command(pgm_read_byte_near(&WRITE_REG_FOR_DISPLAY_PARTIAL[i]), err);
  }
  oledm_stop(display);

  oledm_startCommands(err);
  oledm_command(SSD1680_BORDER_WAVEFORM_CONTROL, err);
  command_data();
  // VBD=VCOM, VBD_LEVEL=VSS, GS_transition=FollowLUT, GS_transition=LUT0
  oledm_command(0x80, err);
  oledm_stop(display);

  oledm_startCommands(err);
  oledm_command(SSD1680_DISPLAY_UPDATE_CONTROL2, err);
  command_data();
  oledm_command(0xC0, err);
  oledm_stop(display);

  oledm_startCommands(err);
  oledm_command(SSD1680_MASTER_ACTIVATION, err);
  oledm_stop(display);

  epaper_wait(display);
}

#define STARTUP_ATTEMPTS 3
void oledm_start(struct OLEDM* display) {
  error_t* err = &(display->error);
  oledm_ifaceInit();
  BUSY_DDR &= ~(1 << BUSY_PIN);
  BUSY_PORT &= ~(1 << BUSY_PIN);

  int startup_attempts = 0;
  while (1) {
    ++startup_attempts;
    epaper_wait(display);
    if ((*err != NO_ACK_ERROR) ||
        (startup_attempts >= STARTUP_ATTEMPTS)) {
      break;
    }

    // its an ack error and we still have startup attempts to spare
    epaper_reset();
  }

  if (*err) {
    // failed to start up
    return;
  }


  oledm_startCommands(err);
  oledm_command(SSD1680_SW_RESET, err);
  oledm_stop(display);
  epaper_wait(display);

  oledm_startCommands(err);
  oledm_command(SSD1680_DRIVER_OUTPUT, err);
  command_data();
  oledm_command(0x27, err);  // 0x127 -> 295 (display width) 
  oledm_command(0x01, err);
  oledm_command(0x00, err);  // Scan down and to the right
  oledm_stop(display);

  oledm_startCommands(err);
  oledm_command(SSD1680_DATA_ENTRY_MODE, err);
  command_data();
  oledm_command(
      display->option_bits & OLEDM_ROTATE_180 ? 0x05 : 0x06, err);
  oledm_stop(display);

  oledm_startCommands(err);
  oledm_command(SSD1680_DISPLAY_UPDATE_CONTROL1, err);
  command_data();
  oledm_command(
      display->option_bits & OLEDM_WHITE_ON_BLACK ? 0x00 : 0x88,
      err);
  oledm_command(0x80, err);
  oledm_stop(display);

  oledm_set_memory_bounds(
      display,
      0,
      0,
      DISPLAY_MEMORY_COLUMNS - 1,
      DISPLAY_MEMORY_ROWS - 1);

  epaper_set_full_mode(display);
}

static void set_memory_pointer(
    struct OLEDM* display, column_t column, uint8_t row) {
  error_t* err = &(display->error);

  oledm_startCommands(err);
  oledm_command(SSD1680_SET_X_ADDRESS_COUNTER, err);
  command_data();
  oledm_command(
      display->option_bits & OLEDM_ROTATE_180 ?
          row :
          DISPLAY_MEMORY_ROWS - row - 1,
      err);
  oledm_stop(display);

  oledm_startCommands(err);
  oledm_command(SSD1680_SET_Y_ADDRESS_COUNTER, err);
  command_data();
  const column_t mcol =
    display->option_bits & OLEDM_ROTATE_180 ?
      DISPLAY_MEMORY_COLUMNS - column - 1 :
      column;
  oledm_command(mcol, err);
  oledm_command(mcol >> 8, err);
  oledm_stop(display);

  epaper_wait(display);
}

void oledm_set_memory_bounds(
    struct OLEDM* display,
    column_t left_column,
    uint8_t top_row,
    column_t right_column,
    uint8_t bottom_row) {

  error_t* err = &(display->error);

  oledm_startCommands(err);
  oledm_command(SSD1680_SET_X_START_END, err);
  command_data();
  if (display->option_bits & OLEDM_ROTATE_180) {
    oledm_command(top_row, err);
    oledm_command(bottom_row, err);
  } else {
    oledm_command(DISPLAY_MEMORY_ROWS - top_row - 1, err);
    oledm_command(DISPLAY_MEMORY_ROWS - bottom_row - 1, err);
  }
  oledm_stop(display);

  oledm_startCommands(err);
  oledm_command(SSD1680_SET_Y_START_END, err);
  command_data();

  const column_t lc =
    display->option_bits & OLEDM_ROTATE_180 ?
        DISPLAY_MEMORY_COLUMNS - left_column - 1 :
        left_column;

  oledm_command(lc & 0xFF, err);
  oledm_command(lc >> 8, err);

  const column_t rc =
    display->option_bits & OLEDM_ROTATE_180 ?
        DISPLAY_MEMORY_COLUMNS - right_column - 1 :
        right_column;

  oledm_command(rc & 0xFF, err);
  oledm_command(rc >> 8, err);

  oledm_stop(display);

  set_memory_pointer(display, left_column, top_row);
}

void oledm_start_pixels(struct OLEDM* display) {
  error_t* err = &(display->error);
  oledm_startCommands(err);
  oledm_command(
      display->option_bits & OLEDM_WRITE_COLOR_RAM ?
      SSD1680_WRITE_RAM_RED :
      SSD1680_WRITE_RAM_BW, err);
  command_data();
}

void epaper_start_color_pixels(struct OLEDM* display) {
  error_t* err = &(display->error);
  oledm_startCommands(err);
  oledm_command(SSD1680_WRITE_RAM_RED, err);
  command_data();
}

void oledm_write_pixels(struct OLEDM* display, uint8_t byte) {
  oledm_ifaceWriteData(reverse_byte(byte), &(display->error));
}

void oledm_clear(struct OLEDM* display, uint8_t byte) {
  byte = reverse_byte(byte);
  oledm_set_memory_bounds(
      display, 0, 0, display->memory_columns - 1, display->memory_rows - 1);
  oledm_start_pixels(display);
  const uint16_t num_bytes = display->memory_rows * display->memory_columns;
  for (uint16_t i=0; i < num_bytes; ++i) {
    // call direct to avoid the double bit reverse
    oledm_ifaceWriteData(byte, &(display->error));
  }
  oledm_stop(display);
}

static void swap_buffers(struct OLEDM* display, uint8_t partial) {
  error_t* err = &(display->error);

  oledm_startCommands(err);
  oledm_command(SSD1680_DISPLAY_UPDATE_CONTROL2, err);
  command_data();
  oledm_command(partial ? 0x0F : 0xC7, err);
  oledm_stop(display);

  oledm_startCommands(err);
  oledm_command(SSD1680_MASTER_ACTIVATION, err);
  oledm_stop(display);
}

void epaper_swap_buffers(
    struct OLEDM* display, EpaperSleepMode sleep_mode) {
  swap_buffers(display, 0);
  epaper_wait(display);
  epaper_sleep_mode(display, sleep_mode);
}

void epaper_swap_buffers_no_wait(struct OLEDM* display) {
  swap_buffers(display, 0);
}

void epaper_update_partial(
    struct OLEDM* display, EpaperSleepMode sleep_mode) {
  swap_buffers(display, 1);
  epaper_wait(display);
  epaper_sleep_mode(display, sleep_mode);
}

void epaper_update_partial_no_wait(struct OLEDM* display) {
  swap_buffers(display, 1);
}

void epaper_sleep_mode(struct OLEDM* display, EpaperSleepMode sleep_mode) {
  if (sleep_mode == SLEEP_MODE_OFF) {
    return;
  }
  error_t* err = &(display->error);

  oledm_startCommands(err);
  oledm_command(SSD1680_DEEP_SLEEP_MODE, err);
  command_data();
  oledm_command(sleep_mode & 0x03, err);
  oledm_stop(display);
}
