// Defines an error type and reserves error codes across all libraries

#ifndef ERROR_CODES_H
#define ERROR_CODES_H

#include <inttypes.h>

typedef uint8_t bool_t;
#define FALSE 0
#define TRUE 1

typedef uint8_t error_t;

// Generic codes
#define NO_ACK_ERROR 0x01

// TWI Codes
#define TWI_NO_ACK_ERROR 0x11             // The slave did not acknowledge  all data
#define TWI_MISSING_START_CON_ERROR 0x12  // Generated Start Condition not detected on bus
#define TWI_MISSING_STOP_CON_ERROR 0x13   // Generated Stop Condition not detected on bus
#define TWI_ARB_LOST_ERROR 0x14           // Bus arbitration failed (try again?)
#define TWI_INTERNAL_ERROR 0x15           // The code ended up in an unexpected state

// SSD1306 Codes
#define SSD1306_BAD_FONT_ID_ERROR 0x20
#define TEXT_INVALID_RLE_DATA 0x21

// MS8607 Codes
#define MS8607_BAD_PT_CHECKSUM 0x30
#define MS8607_BAD_HUM_CHECKSUM 0x31
#define MS8607_INVALID_OSR 0x32

#endif

