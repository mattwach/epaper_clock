#ifndef MENU_H
#define MENU_H

#include "eeprom_vars.h"
#include <inttypes.h>
#include <time.h>


// Initialize internal state
void menu_init(time_t current_y2k, struct EEPromVars* eeprom);

// Called whenever the menu should be shown.
//
// Returns true if we are in "menu mode" and navigating menus
// button_pressed is a bitmap as defined in buttons.h
uint8_t update_menu(uint8_t button_pressed, time_t current_y2k, const struct EEPromVars* eeprom);

#endif

