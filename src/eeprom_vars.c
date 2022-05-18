#include "eeprom_vars.h"

#include <avr/eeprom.h>

// Calculates the checksum.  Option bits are shifted to make more
// use of the overall byte (as UTC offset ranges from -12 to +14)
static inline uint8_t calc_eeprom_checksum(const struct EEPromVars* eeprom) {
  return eeprom->utc_offset ^ (eeprom->option_bits << 4);
}

// Loads the eeprom memory block into the provided structure.
void load_eeprom(struct EEPromVars* eeprom) {
  eeprom_read_block(eeprom, (uint8_t*)(0x00), sizeof(struct EEPromVars));
  const uint8_t checksum = calc_eeprom_checksum(eeprom);
  if ((checksum != eeprom->checksum) ||
      (eeprom->utc_offset < -12) ||
      (eeprom->utc_offset > 14) ||
      (eeprom->option_bits > OPTION_MASK)) {
    // Data is uninitialized or corrupted
    eeprom->utc_offset = 0;
    eeprom->option_bits = 0x00;
  }
}

// Updates the checksum of eeprom and saves it.
void save_eeprom(struct EEPromVars* eeprom) {
  eeprom->checksum = calc_eeprom_checksum(eeprom);
  eeprom_update_block(eeprom, (uint8_t*)(0x00), sizeof(struct EEPromVars));
}

