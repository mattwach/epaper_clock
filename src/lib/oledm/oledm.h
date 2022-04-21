#ifndef OLEDM_OLEDM_H
#define OLEDM_OLEDM_H

#include <inttypes.h>
#include <error_codes.h>

// OLED Driver
//
// Low-level interface
// Supports ATMega328P and ATTiny85
// Supports I2C and SPI
// Supports SSD1306 (monochrome), SH1106 (monochrome) and SSD1708 (color)
//
// General usage:
//
// void main() {
//   OLEDM display;
//
//   oledm_init(&display);
//   oledm_clear(&display, 0x00);
//
//   // Lets draw a filled rect
//   const uint8_t x = 10;
//   const uint8_t y = 1;
//   const uint8_t w = 30;
//   const uint8_t h = 2;  // 2*8 = 16 pixels
//   oledm_set_bounds(&display, x, y, x+w, y+h);
//   uint8_t i=0;
//   const uint8_t count = w * h;
//   oledm_start_pixels(&display);
//   for (; i < count; ++i) {
//     oledm_write_pixels(&display, 0xFF);  // 8 white vertical pixels
//   }
//   oledm_stop(&display);
//
//   if (display.error) {
//     // handle error as needed
//   }
// }
//
// When using the SSD1708 color driver, define OLEDM_COLOR16 and use
//   oledm_set_bounds16(&display, x, y, x+w, y+h);  // rows are single pixels
//   oledm_write_pixels16(&display, 0xFFFF);  // 1 white pixel
//
//   You can also use monochrome commands with the SSD1708 you can choose to
//   define OLEDM_EMULATE_MONOCHROME and use the oledm_set_bounds, oledm_write_pixels
//   or libraries (e.g. text.h) that use those calls.  When using monochrome calls,
//   you can set display->fg_color, display->bg_color to change colors, and can set
//   display->mono_row_offset to shift the row up and down
//
//   Mixing monochrome and color functions is possible but take care.  Making
//   and calls to color commands (e.g. oledm_write_pixels16) will cause the
//   mono_* fields in the OLEDM structure to be invalidated.  A call to
//   oledm_set_memory_bounds will correct this and should be used between
//   using color functions and emulated monochrome ones.  Alternatively,
//   you can try and correct the OLEDM mono_ fields yourself if you feel
//   that works best for your situation.

typedef uint8_t bool_t;

#ifdef SSD1680
typedef unsigned short column_t;
#else
typedef unsigned char column_t;
#endif

// State object
struct OLEDM {
  // Error status.  Check this when convienent.  If it's not zero,
  // commands other than oledm_init() will return immediately without
  // doing anything.  This allows a more casual error-checking strategy at
  // the client-level (less code).
  error_t error;

  // Visible display dimensions.  Some libraries might need these for fitting
  // information, such as where to word-wrap.
  column_t visible_columns;
  uint8_t visible_rows;

  // Memory display dimensions.  Libraries that might want to render
  // off-screen may need these
  column_t memory_columns;
  uint8_t memory_rows;

  // Offset values, used to correctly center smaller displays.
  // For example, if using a 64x32 display, you might need an
  // column offset of 64 and a row offset of 32.  Or maybe not.
  // It depends on what part of memory the manufacturer chose
  // to connect to live pixels.  Just try different values and
  // see.
  column_t column_offset;
  uint8_t row_offset;

  // current startline.  Somewhere between zero (init) and
  // DISPLAY_MEMORY_ROWS - 1.  This value is used to implement scrolling
  // but could also be used for double buffering on small displays
  // For example, on a 64x32 a oledm_vscroll(4) would have the
  // practical effect of a buffer-swap.  You just purposely set the
  // row_offset to the off-screen data for this case.
  uint8_t start_line;

  // Option bits.  These are needed to change the initialization
  // sequence that certain displays need
  uint8_t option_bits;

#ifdef OLEDM_COLOR16
  // Put color only fields here.
#endif
#ifdef OLEDM_EMULATE_MONOCHROME
  // If this is set, extra tracking data needs to be added to the
  // structure.

  // User-supplied foreground and background colors
  uint16_t fg_color;
  uint16_t bg_color;
  // With monochrome displayes, the rows are fixed at multiple of 8
  // but the emulator is not bound by this limitation.  Thus
  // mono_row_offset can be set from -7 to 7 to shift the row
  int8_t mono_row_offset;

  // State tracking
  uint8_t mono_row;
  column_t mono_column;
  uint8_t mono_top_row;
  uint8_t mono_bottom_row;
  column_t mono_left_column;
  column_t mono_right_column;
#endif
};

// Option bit definitions
#define OLEDM_COM_LEFT_RIGHT_REMAP 0x01  // SETCOMPINS bit 5
#define OLEDM_ALTERNATE_COM_CONFIG 0x02  // SETCOMPINS bit 4
#define OLEDM_WHITE_ON_BLACK       0x04  // Used by epaper display
#define OLEDM_ROTATE_180           0x08  // Rotates display 180 degrees (if supported)
#define OLEDM_WRITE_COLOR_RAM      0x10  // Use color ram on epaper displays

// Initializes the display structure and display.  In a generic way.
//  The process is:
//    OLEDM display;
//    oledm_basic_init(&display);
//    display.visible_columns = 64;  // Optional, depending on prefrence
//    oledm_start(&display);
//
//  Alternately, choose an init from ssd1306_configurations.h or make
//  a custom one...
//    OLEDM display;
//    oledm_init_62x32_a(&display);
//    oledm_start(&display);
//
// Args:
//  display: object to initialize
void oledm_basic_init(struct OLEDM* display);

// Send initial commands that initializes and turns on the display
void oledm_start(struct OLEDM* display);

// Commands to turn off the display and turn it back on.
// Note that oledm_start already turns on the display so there
// is no need to call oledm_display_on() unless oledm_display_off()
// was called first.
void oledm_display_off(struct OLEDM* display);
void oledm_display_on(struct OLEDM* display);

// Used to scroll the screen vertically up or down by a number of
// rows. Sends a SET_STARTLINE command, along with setting the start
// line in the buffer so that future writes happen where you would
// expect. Note that the hardware supports per-pixel scrolling, but
// this library can not support setBounds this way, so we stick with
// multiple of 8.
//
// If you are using a color display, you can call oledm_vscroll16, which
// does not multiply by 8
void oledm_vscroll(struct OLEDM* display, int8_t rows);

// Clear the entire screen (include non-visible parts on smaller displays)
// byte is usually the byte to clear to but there are some exceptions
// for color displays or potentially hardware accelerated ones.
void oledm_clear(struct OLEDM* display, uint8_t byte);

// Start sending pixel data
void oledm_start_pixels(struct OLEDM* display);
// Stop sending pixels or a command
void oledm_stop(struct OLEDM* display);


// Set the display data window.  Written bytes will start at the upper
// right of this window and wrap to the left (similar to words on a page)
//
// Args:
//  display: initialized display object.
//  left_column: Column from 0-(DISPLAY_MEMORY_COLUMNS-1).  Zero is the leftmost column.
//  right_column: Column where wrapping will begin
//  top_row: row number from 0-(DISPLAY_MEMORY_ROWS-1).  Each row has
//    8 vertical pixels where the LSB is toward the top.  A zero value
//    is used to indicate the top of the display.
//  bottom_row: after this row, the data will wrap to the top right
//
// setBounds applies the current start_line and row/column offset.  This
// acounts for coordinate shifts on smaller displays (64x32) and
// when using scrolling.
//
// setMemoryBounds does not account for these things.  It's useful when
// these adjustments are a hinderance, such as when you want to change
// non-visible data without changing the start_line or if it want all
// data changed, whether visible or not.
//
// oledm_set_bounds, oledm_set_memory_bounds assume a monochrome OLED.
// When using a color OLED, they will go through emulation logic to
// try and work as they would with a monochrome OLED
void oledm_set_bounds(struct OLEDM* display,
  column_t left_column, uint8_t top_row,
  column_t right_column, uint8_t bottom_row);
void oledm_set_memory_bounds(struct OLEDM* display,
  column_t left_column, uint8_t top_row,
  column_t right_column, uint8_t bottom_row);

// Monochrome OLED pixel interface (emulated when using a color display)
void oledm_write_pixels(struct OLEDM* display, uint8_t byte);

#ifdef OLEDM_COLOR16
  #define rgb16(R, G, B) (((R) << 11) | ((G) << 5) | (B))
  // Come up with a color by index value.  This is useful when you
  // want a variety of colors but are not so particular on what they are.
  // Thus the intent is to have close indexes (0,1,2) represent fairly
  // different colors.
  uint16_t rgb16_by_index(uint8_t color_index);

  void oledm_write_pixel16(struct OLEDM* display, uint16_t pixel);
  void oledm_set_bounds16(struct OLEDM* display,
    column_t left_column, uint8_t top_row,
    column_t right_column, uint8_t bottom_row);
  void oledm_set_memory_bounds16(struct OLEDM* display,
    column_t left_column, uint8_t top_row,
    column_t right_column, uint8_t bottom_row);
  void oledm_vscroll16(struct OLEDM* display, int8_t rows);
#endif


#endif
