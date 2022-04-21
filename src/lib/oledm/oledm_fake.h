#ifndef LIB_OLEDM_FAKE_H
#define LIB_OLEDM_FAKE_H

#include "oledm.h"
#include <inttypes.h>

// Fake suppliment for OLEDM unit tests.
// The basic setup:

// Log codes
#define OLEDM_BASIC_INIT        0xA0
#define OLEDM_START             0xA1
#define OLEDM_DISPLAY_OFF       0xA2
#define OLEDM_DISPLAY_ON        0xA3
#define OLEDM_VSCROLL           0xA4
#define OLEDM_CLEAR             0xA5
#define OLEDM_START_PIXELS      0xA6
#define OLEDM_STOP              0xA7
#define OLEDM_SET_BOUNDS        0xA8
#define OLEDM_SET_MEMORY_BOUNDS 0xA9
#define OLEDM_WRITE_PIXELS      0xAA

extern uint32_t oledm_log[];
extern uint16_t oledm_logidx;

static inline void oledm_log_reset() {
    oledm_logidx = 0;
}

#endif
