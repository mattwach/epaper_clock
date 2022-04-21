#ifndef EEPROM_VARS_H
#define EEPROM_VARS_H

#include <inttypes.h>

// Bit field
#define OPTION_USE_24H_TIME 0x01
#define OPTION_USE_METRIC 0x02
#define OPTION_DARK_MODE 0x04
#define OPTION_MASK 0x07

struct EEPromVars {
  uint8_t utc_offset;
  uint8_t option_bits;
  uint8_t checksum;
};

// loads EEProm data.  Loads with default values if the eeprmo data is
// uninitialized or corrupted
void load_eeprom(struct EEPromVars* eeprom);

// Saves data to eeprom
void save_eeprom(struct EEPromVars* eeprom);

#endif
