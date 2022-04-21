#ifndef WEATHER_MS8607_H
#define WEATHER_MS8607_H

// MS8607 Temperature/Humidity/Pressure sensor
//
// Typical usage
//
// struct MS8607 ms8607;
//
// void main(void) {
//   int16_t temp_cc;
//   uint32_t pressure_pa;
//   uint8_t humidity_cpct;
//
//   ms8607_init(&ms8607);
//   // You can pass NULL for any measurement you do not need.
//   ms8607_read_values(&ms8607, &temp_cc, &pressure_pa, &humidity_cpct);
//
//   if (ms8607.err) {
//     // something went wrong above.  You can alse put error checking
//     // between commands if you want finer grained information.
//   } else {
//     // do something with the data.
//   }
// }
//

#include <error_codes.h>

// See datasheet
struct PTCalibrationValues {
  uint16_t crc;       // C0
  uint16_t sens;      // C1
  uint16_t off;       // C2
  uint16_t tcs;       // C3
  uint16_t tco;       // C4
  uint16_t tref;      // C5
  uint16_t tempsens;  // C6
};

typedef enum {
  OSR_256 = 0x00,
  OSR_512 = 0x02,
  OSR_1024 = 0x04,
  OSR_2048 = 0x06,
  OSR_4096 = 0x08,
  OSR_8192 = 0x0A
} OSRResolution;

struct MS8607 {
  struct PTCalibrationValues pt_cal;
  // default for both is OSR_4096.  Changing these
  // trades resolution for calculation speed and power usage.
  OSRResolution temperature_resolution;
  OSRResolution pressure_resolution;
  error_t err;
};

void ms8607_init(struct MS8607* ms8607);
void ms8607_humidity_settings(
    struct MS8607* ms8607,
    OSRResolution resolution,
    bool_t use_chip_heater);

// Extracts temperature in cC (1349 -> 13.49 C)
// pressure in Pascals
// and humidity in Percent * 100 (1549 -> 15.49%)
//
// Note that temp_cc or pressure_dapa can be set to
// NULL if you do not need the value.
//
// Setting pressure_dapa to NULL will speed up the operation.
// Setting humidity_cpct to NULL will speed up the operation.
// Setting temp_cc to NULL will NOT speed up the operation.
void ms8607_read_values(
    struct MS8607* ms8607,
    int16_t* temp_cc,
    uint32_t* pressure_pa,
    uint16_t* humidity_cpct);


#endif

