// Common source between ssd1306.c and sh1106.c

// The following functions could be implemented by either TWI or SPI,
// depending on if oledm_i2c.o or oledm_spi.o is linked in.
extern void oledm_ifaceInit(void);
extern void oledm_ifaceWriteData(uint8_t data, error_t* err);
extern void oledm_startCommands(error_t* err);
extern void oledm_command(uint8_t cmd, error_t* err);

void oledm_set_bounds(
    struct OLEDM* display,
    column_t left_column,
    uint8_t top_row,
    column_t right_column,
    uint8_t bottom_row) {
  // Top row and bottom row need to be adjusted by start_line
  // Examples:
  //  If start_line = 0 then row 4 is row 4
  //  Is start_line = 1 then row 4 is actually row 5
  top_row += display->start_line + display->row_offset;
  bottom_row += display->start_line + display->row_offset;

  // apply column offsets as well
  left_column += display->column_offset;
  right_column += display->column_offset;

  oledm_set_memory_bounds(display, left_column, top_row, right_column, bottom_row);
};

