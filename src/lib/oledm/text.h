// Library for displaying text
//
// Basic usage example:
//
// #include <oledm/oledm.h>
// #include <oledm/text.h>
// #include <oledm/fonts/terminus6x8.h>
// 
// struct OLEDM display;
// struct Text text;
// 
// void main(void) {
//   oledm_init(&display);
//   oledm_clear(&display, 0x00);
//   text_init(&text, terminus6x8, &display);
//   text.column = text.cr_column = 8;  // indent a bit
//   text_print_fixed(&text, "Hello");
//   text_print_fixed(&text, " world!");  // will go to next line
//   text_cr(&text);  // next line
//   text_print_fixed(&text, "Bonjour le monde!");
// }

#ifndef SSD1306_TEXT_H
#define SSD1306_TEXT_H

#include <inttypes.h>
#include "oledm.h"

// This is a bit field, 0x01, 0x02, 0x04, 0x08, 0x10...
#define TEXT_OPTION_INVERTED 0x01

struct Text {
    const uint8_t* font;    // Pointer to some font data
    struct OLEDM* display;  // Pointer to display object
    column_t column;           // Current column to start printing text from
    uint8_t row;              // Current row to start printing text from
    uint8_t options;          // Can add TEXT_OPTION_* flags here
};

struct FontASCII {
  uint8_t id[4];       // Should be set to 'FAS1'
  uint8_t first_char;  // Minimum supported ASCII character
  uint8_t last_char;   // Maximum supported ASCII character, inclusive
  uint8_t width;       // Column width of each character.  FUTURE: This could
                       // be set to zero for variable width fonts
  uint8_t height;      // row width of each character where a row is 8 pixels

  // First data byte.  For a fixed font, ach character will take 
  // width * height bytes to display. Character are directly concatenated
  // to one another in the data array.
  //
  // FUTURE: variable width fots might start with a table here that gives
  // the byte offset of each character.  e.g.  Say you have i=3, j=5, k=6 for widths
  // and those are the only three supported characters.  You would see
  //
  // 0x0003, <- j start offset
  // 0x0008, <- k start offset
  // 0x000E, <- end of column
  // 0x00, ... <- i data, j, data, k data similar to fixed width
  uint8_t data[];     
};

struct VariableFont {
  uint8_t id[4];       // Should be set to 'VAR1'
  uint8_t num_chars;   // number of characters
  uint8_t height;      // row width of each character where a row is 8 pixels

  // Now for a lookup table.  Format is
  // uint8_t char_idx
  // uint8_t char_width
  // uint16_t offset
  // ...
  //
  // Where offset is the number of bytes from the start of data[]
  // char_idx must be in order to support binary search

  // then comes all of the data bytes.  These are stored in a simple
  // but effective RLE format of the following pattern:
  // l1, b  Length of sequence, sequence byte (l1 must be <128)
  // l2 | 0x80, b1, b2, b3 ...  Length of non repeat followed by bytes
  //
  // example:
  // 0x05 0xFF 0x83 0x01 0x02 0x03
  //
  // Would result in
  // 0xFF 0xFF 0xFF 0xFF 0xFF 0x01 0x02 0x03
  //
  // The pattern is effective due to all of the whitespace and repeating
  // patterns in a typical font.  Think about 'H', '-', '.' etc...
  uint8_t data[];     
};

// Use this to initialize a text object or change fonts in an existing one
void text_init(struct Text* text, const void* font, struct OLEDM* display);

// outputs a string.  text must be initialized with a fixed
// font or this function will set an error in text->display
// and do nothing.
void text_strLen(struct Text* text, const char* str, uint8_t length);
void text_str(struct Text* text, const char* str);
// Pascal string.  Useful in combination with the pstr/pstr.h library,
// for printing formatted integers, etc.
static inline void text_pstr(struct Text* text, const uint8_t* pstr) {
  text_strLen(text, (const char*)(pstr + 1), *pstr);
}
// Single character
static inline void text_char(struct Text* text, char c) {
  text_strLen(text, &c, 1);
}

// Clears from the current column to the end of the line.
// Sets text->column to the maximum available column
// The number of rows cleared is based on the height of the current font
void text_clear_row(struct Text* text);


// Verifies the given data has an expected id.  Sets text->display->error
// if not
void text_verifyFont(struct Text* text);

#endif  // SSD1360_TEXT_H
