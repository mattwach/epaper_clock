#include "ms8607.h"
#include <twi/twi.h>
#include <util/delay.h>
#include <debug/debug.h>
#include <string.h>

#define PT_I2C_ADDRESS 0x76
typedef enum {
  PT_ADC_READ = 0x00,
  PT_RESET = 0x1E,
  PT_CONVERT_PRESSURE_BASE = 0x40,
  PT_CONVERT_TEMPERATURE_BASE = 0x50,
  PT_PROM_ADDRESS_0 = 0xA0,
} PTI2CCommands;

#define HUM_I2C_ADDRESS 0x40
typedef enum {
  HUM_MEASURE_WITH_HOLD = 0xE5,
  HUM_WRITE_USER_REGISTER = 0xE6,
  HUM_READ_USER_REGISTER = 0xE7,
  HUM_MEASURE_NO_HOLD = 0xF5,
  HUM_RESET = 0xFE,
} HumI2CCommands;

static void reset(error_t* err) {
  twi_startWrite(PT_I2C_ADDRESS, err);
  twi_writeWithStop(PT_RESET, err);

  twi_startWrite(HUM_I2C_ADDRESS, err);
  twi_writeWithStop(HUM_RESET, err);

  // From the data sheet (p15) "The reset takes less than 15ms"
  _delay_ms(15);
  DEBUG_U8("ms8607_reset", *err);
}

// copied from util/crc4.c
// n_prom must be an array of size 8
#define N_PROM_ARRAY_SIZE 8
static uint8_t crc4_PT_mod(uint16_t n_prom[]) {
  uint16_t n_rem=0; // crc remainder
  n_prom[0]=((n_prom[0]) & 0x0FFF); // CRC byte is replaced by 0
  n_prom[7]=0; // Subsidiary value, set to 0
  for (uint8_t cnt = 0; cnt < (N_PROM_ARRAY_SIZE * 2); cnt++) { // choose LSB or MSB
    if (cnt & 1) {
      n_rem ^= (n_prom[cnt>>1]) & 0x00FF;
    } else {
      n_rem ^= (n_prom[cnt>>1]>>8);
    }
    for (uint8_t n_bit = 8; n_bit > 0; n_bit--) {
      if (n_rem & (0x8000)) {
        n_rem = (n_rem << 1) ^ 0x3000;
      }
      else {
        n_rem = (n_rem << 1);
      }
    }
  }
  n_rem= ((n_rem >> 12) & 0x000F); // final 4-bit remainder is CRC code
  DEBUG_U8("crc4_PT_mod", n_rem);
  return (uint8_t)(n_rem);
}


static void validate_pt_checksum(struct MS8607* ms8607) {
  const struct PTCalibrationValues* pt_cal = &(ms8607->pt_cal);
  const uint8_t read_crc = pt_cal->crc >> 12;
  DEBUG_U8("read_crc", read_crc);
  uint16_t n_prom[N_PROM_ARRAY_SIZE];
  memcpy(n_prom, pt_cal, sizeof(struct PTCalibrationValues));
  const uint8_t calc_crc = crc4_PT_mod(n_prom);
  if (read_crc != calc_crc) {
    ms8607->err = MS8607_BAD_PT_CHECKSUM;
  }
  DEBUG_U8("validate_pt_checksum", ms8607->err);
}


static void read_pt_calibration_values(struct MS8607* ms8607) {
  uint16_t* raw_values = (uint16_t*)(&ms8607->pt_cal);  
  const uint8_t value_count = sizeof(struct PTCalibrationValues) / sizeof(uint16_t);
  uint8_t read_buffer[2];
  error_t* err = &(ms8607->err);
  for (uint8_t i=0; (ms8607->err == 0) && (i < value_count); ++i) {
    twi_startWrite(PT_I2C_ADDRESS, err);
    twi_writeNoStop(PT_PROM_ADDRESS_0 + (i * 2), err);
    twi_readWithStop(PT_I2C_ADDRESS, read_buffer, 2, err);
    raw_values[i] = (read_buffer[0] << 8) | read_buffer[1];
    DEBUG_U16("PROM", raw_values[i]);
  }

  if (*err == 0) {
    validate_pt_checksum(ms8607);
  }
  DEBUG_U8("read_pt_calibration_values", ms8607->err);
}

void ms8607_init(struct MS8607* ms8607) {
  ms8607->err = 0;
  ms8607->temperature_resolution = OSR_4096;
  ms8607->pressure_resolution = OSR_4096;
  twi_init();
  reset(&(ms8607->err));

  if (ms8607->err == 0) {
    read_pt_calibration_values(ms8607);
  }

  DEBUG_U8("ms8607_init", ms8607->err);
}

void ms8607_humidity_settings(
    struct MS8607* ms8607,
    OSRResolution resolution,
    bool_t use_chip_heater) {
  error_t* err = &(ms8607->err);

  DEBUG_U8("ms8607_humidity_settings_res", resolution);
  DEBUG_U8("  chip_heat", use_chip_heater);

  uint8_t user_register = 0x00;
  twi_startWrite(HUM_I2C_ADDRESS, err);
  twi_writeNoStop(HUM_READ_USER_REGISTER, err);
  twi_readWithStop(HUM_I2C_ADDRESS, &user_register, 1, err);

  DEBUG_U8("  old_user_reg", user_register);
  // clear bit 0, 2, 7 -> 0111 1010 -> 0x7A
  user_register &= 0x7A;

  switch (resolution) {
    case OSR_256:
      user_register |= 0x81;  // 11
      break;
    case OSR_1024:
      user_register |= 0x80;  // 10
      break;
    case OSR_2048:
      user_register |= 0x01;  // 01
      break;
    case OSR_4096:
      // leave at 00
      break;
    default:
      *err = MS8607_INVALID_OSR;
      DEBUG_STR("  invalid osr");
  }

  if (use_chip_heater) {
    user_register |= 0x04;  // Turn on chip heater
  }

  DEBUG_U8("  new_user_reg", user_register);
  twi_startWrite(HUM_I2C_ADDRESS, err);
  twi_writeNoStop(HUM_WRITE_USER_REGISTER, err);
  twi_writeWithStop(user_register, err);

  DEBUG_U8("ms8607_humidity_settings", *err);
}

// Waits for converstion to be complete, then returns the adc value
#define READ_TIMEOUT_MS 100
static int32_t read_pt(uint8_t command, error_t* err) {
  uint8_t adc_buffer[3];

  twi_startWrite(PT_I2C_ADDRESS, err);
  twi_writeWithStop(command, err);

  for (uint8_t i=0; (*err == 0) && (i < READ_TIMEOUT_MS); ++i) {
    _delay_ms(1);
    twi_startWrite(PT_I2C_ADDRESS, err);
    twi_writeWithStop(PT_ADC_READ, err);
    if (*err == 0) {
      DEBUG_U8("adc ready after", i+1);
      break;
    } else if (*err == TWI_NO_ACK_ERROR) {
      // Conversion is not yet ready.
      *err = 0;
    } else {
      break;
    }
  }

  twi_readWithStop(PT_I2C_ADDRESS, adc_buffer, 3, err);
  const uint32_t adc_value = (*err == 0) ?
      (uint32_t)adc_buffer[0] << 16 |
      (uint32_t)adc_buffer[1] << 8 |
      (uint32_t)adc_buffer[2] : 0;

  DEBUG_U32("adc_value", adc_value);
  return (int32_t)adc_value;
}

static void check_hum_crc(uint16_t hum_value, uint8_t crc, error_t* err) {
  uint32_t polynom = 0x988000; // x^8 + x^5 + x^4 + 1
  uint32_t msb = 0x800000;
  uint32_t mask = 0xFF8000;
  uint32_t calc_crc = (uint32_t)hum_value << 8;

  while (msb != 0x80) {
    if (calc_crc & msb)
      calc_crc = ((calc_crc ^ polynom) & mask) | (calc_crc & ~mask);

    msb >>= 1;
    mask >>= 1;
    polynom >>= 1;
  }
  DEBUG_U8("hum_crc", crc);
  DEBUG_U8("calc_crc", calc_crc);
  if (calc_crc != crc) {
    *err = MS8607_BAD_HUM_CHECKSUM;
  }
}

static uint16_t read_humidity(error_t* err) {
  uint8_t buffer[3];

  twi_startWrite(HUM_I2C_ADDRESS, err);
  twi_writeWithStop(HUM_MEASURE_NO_HOLD, err);

  for (uint8_t i=0; (*err == 0) && (i < READ_TIMEOUT_MS); ++i) {
    _delay_ms(1);
    twi_readWithStop(HUM_I2C_ADDRESS, buffer, 3, err);
    if (*err == 0) {
      DEBUG_U8("hum measure ready after", i+1);
      break;
    } else if (*err == TWI_NO_ACK_ERROR) {
      // Not yet ready.
      *err = 0;
    } else {
      break;
    }
  }

  const uint16_t hum_value = (*err == 0) ?
      (uint32_t)buffer[0] << 8 | (uint32_t)buffer[1] :
      0;

  if (*err == 0) {
    check_hum_crc(hum_value, buffer[2], err);
  }
  

  DEBUG_U32("hum_value", hum_value);
  return (int32_t)hum_value;
}

void ms8607_read_values(
    struct MS8607* ms8607,
    int16_t* temp_cc,
    uint32_t* pressure_pa,
    uint16_t* humidity_cpct) {
  error_t* err = &(ms8607->err);
  const struct PTCalibrationValues* pt_cal = &(ms8607->pt_cal);

  const int32_t raw_temp = read_pt(
      PT_CONVERT_TEMPERATURE_BASE | ms8607->temperature_resolution, err);
  const int32_t raw_pressure =
    pressure_pa ? read_pt(
        PT_CONVERT_PRESSURE_BASE | ms8607->pressure_resolution, err) : 0;
  const uint16_t raw_humidity =
    humidity_cpct ? read_humidity(err) : 0;


  // calculate temperature according to data sheet
  const int64_t dt = raw_temp - ((int32_t)pt_cal->tref << 8);
  int16_t local_temp_cc = (int16_t)(2000 + ((dt * (int64_t)pt_cal->tempsens) >> 23));  

  // non-linear temperature compensation via datasheet guidance.
  const int16_t t2 = (local_temp_cc >= 2000) ?
    (int16_t)((5 * dt * dt) >> 38) :
    (int16_t)((3 * dt * dt) >> 33);

  if ((*err == 0) && temp_cc) {
    *temp_cc = local_temp_cc - t2;
  }

  if ((*err == 0) && pressure_pa) {
    int32_t off2 = 0;
    int32_t sens2 = 0;

    // second order corrections
    if (local_temp_cc < 2000) {
      const int32_t low_temp_sq =
        ((int32_t)local_temp_cc - 2000) * ((int32_t)local_temp_cc - 2000);
      off2 = (61 * low_temp_sq) >> 4;
      sens2 = (29 * low_temp_sq) >> 4;

      if (local_temp_cc < -1500) {
        const int32_t very_low_temp_sq = 
          ((int32_t)local_temp_cc + 1500) * ((int32_t)local_temp_cc + 1500); 
        off2 += 17 * very_low_temp_sq; 
        sens2 += 9 * very_low_temp_sq;
      }
    }

    const int64_t off =
      ((int64_t)pt_cal->off << 17) +
      (((int64_t)pt_cal->tco * dt) >> 6) - off2;

    const int64_t sens =
      ((int64_t)pt_cal->sens << 16) +
      (((int64_t)pt_cal->tcs * dt) >> 7) - sens2;

    (*pressure_pa) = (uint32_t)(
        ((((int64_t)raw_pressure * sens) >> 21) - off) >> 15
    );
  }

  if ((*err == 0) && humidity_cpct) {
    int32_t local_hum = ((12500 * (int32_t)raw_humidity) >> 16) - 600;
    if (local_hum > 10000) {
      local_hum = 10000;
    } else if (local_hum < 0) {
      local_hum = 0;
    }

    // first order temperature compensation
    local_hum -= (2000 - local_temp_cc + t2) * 18 / 100;

    *humidity_cpct = (uint16_t)local_hum;
  }

  DEBUG_U8("ms8607_read_values err", *err);
}

