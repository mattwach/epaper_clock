#include "nmea_decoder.h"

#ifndef DISABLE_INTERRUPTS
  #include <avr/interrupt.h>
  #define DISABLE_INTERRUPTS cli
  #define ENABLE_INTERRUPTS sei
#endif

static inline char get_char(const struct NMEA_decoder* gps, uint16_t offset) {
  return gps->buffer[(gps->buffer_head + offset) & NMEA_BUFFER_MASK];
}

static inline void set_char(struct NMEA_decoder* gps, uint16_t offset, char c) {
  gps->buffer[(gps->buffer_head + offset) & NMEA_BUFFER_MASK] = c;
}

static inline char hex_char(uint8_t val) {
  if (val < 10) {
    return '0' + val;
  }
  return 'A' + val - 10;
}

// Finds the next index and updates gpd structure.  Returns 1 if sucessful
static uint8_t next_index(struct NMEA_decoder* gps) {
  ++gps->index;

  while (gps->index_offset < gps->buffer_size) {
    const char c = get_char(gps, gps->index_offset);
    if (c == '\r') {
      break;
    }
    ++gps->index_offset;
    if (gps->index_offset >= gps->buffer_size) {
      break;
    }

    if (c == ',') {
      return 1;
    }
  }

  // if we get here, then the process failed
  gps->last_error = NMEA_BAD_INDEX;
  gps->index = 0;
  gps->index_offset = 0;
  return 0;
}

// Updates gps to point to index and returns '1', if sucessful.
static uint8_t find_index(struct NMEA_decoder* gps, uint8_t index) {
  if (gps->index > index) {
    gps->index = 0;
    gps->index_offset = 0;
  }

  while (gps->index < index) {
    if (!next_index(gps)) {
      return 0;
    }
  }

  return 1;
}

static uint8_t find_end_of_digits(
    struct NMEA_decoder* gps,
    int16_t* char_idx) {
  const int16_t initial_idx = (*char_idx);

  while (1) {
    if ((*char_idx) >= gps->buffer_size) {
      if ((*char_idx) > initial_idx) {
        --(*char_idx);
        break;
      }
      return 0;
    }
    const char c = get_char(gps, (*char_idx));
    if (c < '0' || c > '9') {
      switch (c) {
        case '-':
          gps->last_error = NMEA_NEGATIVE_NUMBER;
          break;
        case ',':
        case '.':
        case '\r':
        case '*':
          if ((*char_idx) == initial_idx) {
            gps->last_error = NMEA_EMPTY_FIELD;
          }
          break;
        default:
          gps->last_error = NMEA_NON_NUMERIC_CHARACTER;
          break;
      }
      if ((*char_idx) == gps->index_offset) {
        return 0;
      }
      --(*char_idx);
      break;
    }
    ++(*char_idx);
  }
  return 1;
}

static uint32_t parse_u32_from_end(
    struct NMEA_decoder* gps,
    int16_t start_idx,
    int16_t char_idx,
    uint32_t* multiplier) {
  uint32_t val = 0;
  uint32_t local_mutiplier;
  if (!multiplier) {
    multiplier = &local_mutiplier;
  }
  *multiplier = 1;
  for (; char_idx >= start_idx; --char_idx, (*multiplier) *= 10) {
    const char c = get_char(gps, char_idx) - '0';
    const uint32_t old_val = val;
    val += (*multiplier) * c;
    if ((*multiplier) == 1000000000) {
      if (char_idx > start_idx || c > 4) {
        // will overflow
        gps->last_error = NMEA_NUMBER_OVERFLOW;
        return 0;
      }
      if (old_val > val) {
        // overflow
        gps->last_error = NMEA_NUMBER_OVERFLOW;
        return 0;
      }
    }
  }

  return val;
}

static uint32_t parse_u32(
    struct NMEA_decoder* gps, int16_t char_idx, uint32_t* multiplier) {
  if (!find_end_of_digits(gps, &char_idx)) {
    return 0;
  }
  return parse_u32_from_end(gps, gps->index_offset, char_idx, multiplier);
}

static uint16_t parse_u16(struct NMEA_decoder* gps, int16_t char_idx) {
  if (!find_end_of_digits(gps, &char_idx)) {
    return 0;
  }
  uint16_t val = 0;
  uint16_t multiplier = 1;
  for (; char_idx >= gps->index_offset; --char_idx, multiplier *= 10) {
    const char c = get_char(gps, char_idx) - '0';
    const uint16_t old_val = val;
    val += multiplier * c;
    if (multiplier == 10000) {
      if (char_idx > gps->index_offset || c > 6) {
        // will overflow
        gps->last_error = NMEA_NUMBER_OVERFLOW;
        return 0;
      }
      if (old_val > val) {
        // overflow
        gps->last_error = NMEA_NUMBER_OVERFLOW;
        return 0;
      }
    }
  }

  return val;
}

static uint8_t parse_u8(struct NMEA_decoder* gps, int16_t char_idx) {
  if (!find_end_of_digits(gps, &char_idx)) {
    return 0;
  }
  uint8_t val = 0;
  uint8_t multiplier = 1;
  for (; char_idx >= gps->index_offset; --char_idx, multiplier *= 10) {
    const char c = get_char(gps, char_idx) - '0';
    const uint8_t old_val = val;
    val += multiplier * c;
    if (multiplier == 100) {
      if (char_idx > gps->index_offset || c > 2) {
        // will overflow
        gps->last_error = NMEA_NUMBER_OVERFLOW;
        return 0;
      }
      if (val < old_val) {
        // overflow
        gps->last_error = NMEA_NUMBER_OVERFLOW;
        return 0;
      }
    }
  }

  return val;
}

static void split_fraction(
    struct NMEA_decoder* gps,
    int16_t char_idx,
    uint32_t* int_val,
    uint32_t* frac_val,
    uint32_t* frac_multiplier,
    uint8_t* is_negative) {
  *is_negative = 0;
  *int_val = 0;
  *frac_val = 0;
  *frac_multiplier = 1;

  if (char_idx < gps->buffer_size && get_char(gps, char_idx) == '-') {
    *is_negative = 1;
    ++char_idx;
  }
  const uint16_t int_start_idx = char_idx;

  if (char_idx < gps->buffer_size && get_char(gps, char_idx) != '.') {
    if (!find_end_of_digits(gps, &char_idx)) {
      return;
    }
    *int_val = parse_u32_from_end(gps, int_start_idx, char_idx, 0);
  } else {
    --char_idx;  // position ahead of the dot for frac_idx
  }

  int16_t frac_idx = char_idx + 1;

  if (frac_idx < (gps->buffer_size - 1) && get_char(gps, frac_idx) == '.') {
    ++frac_idx; // skip over the .
    const uint16_t frac_start_idx = frac_idx;
    if (find_end_of_digits(gps, &frac_idx)) {
      *frac_val = parse_u32_from_end(
          gps, frac_start_idx, frac_idx, frac_multiplier);
    }
  }
}

static float parse_float(struct NMEA_decoder* gps, int16_t char_idx) {
  uint32_t int_val;
  uint32_t frac_val;
  uint32_t frac_multiplier;
  uint8_t is_negative;

  split_fraction(
      gps, char_idx, &int_val, &frac_val, &frac_multiplier, &is_negative);

  if (is_negative) {
    return -(float)(int_val) - ((float)frac_val / (float)frac_multiplier);
  }
  return (float)(int_val) + ((float)frac_val / (float)frac_multiplier);
}

static double parse_double(struct NMEA_decoder* gps, int16_t char_idx) {
  uint32_t int_val;
  uint32_t frac_val;
  uint32_t frac_multiplier;
  uint8_t is_negative;

  split_fraction(
      gps, char_idx, &int_val, &frac_val, &frac_multiplier, &is_negative);

  if (is_negative) {
    return -(double)(int_val) - ((double)frac_val / (double)frac_multiplier);
  }
  return (double)(int_val) + ((double)frac_val / (double)frac_multiplier);
}

static void convert_to_dms(
    struct NMEA_decoder* gps,
    uint8_t index,
    uint8_t* degrees,
    uint8_t* minutes,
    float* seconds,
    char* direction) {
  double raw_val = nmea_parse_double(gps, index) / 100.0;
  *direction = nmea_parse_alpha(gps, index + 1);

  if (gps->last_error) {
    return;
  }

  *degrees = (uint8_t)(raw_val);
  raw_val -= (double)(*degrees);
  raw_val *= 100.0;
  *minutes = (uint8_t)(raw_val);
  raw_val -= (double)(*minutes);
  *seconds = (float)(raw_val * 60.0);
}

static void parse_time(
    struct NMEA_decoder* gps,
    uint8_t index,
    uint8_t* hours,
    uint8_t* minutes,
    uint8_t* seconds) {
  uint32_t timestamp = nmea_parse_u32(gps, index);
  *seconds = (uint8_t)(timestamp % 100);
  timestamp /= 100;
  *minutes = (uint8_t)(timestamp % 100);
  timestamp /= 100;
  *hours = (uint8_t)timestamp;
}

static float convert_to_metric(
    struct NMEA_decoder* gps, uint8_t index, char pos_dir, char neg_dir) {
  float raw_val = nmea_parse_float(gps, index) / 100.0;
  char direction = nmea_parse_alpha(gps, index + 1);
  uint8_t degrees = (uint8_t)raw_val;
  float minutes = (raw_val - (float)degrees) * 100.0;
  if (gps->last_error) {
    return 0.0;
  }
  float val = (float)degrees + minutes / 60.0;
  if (direction == pos_dir) {
    return val;
  }
  if (direction == neg_dir) {
    return -val;
  }
  gps->last_error = NMEA_INVALID_DIRECTION;
  return 0.0;
}

static double convert_to_metric_double(
    struct NMEA_decoder* gps, uint8_t index, char pos_dir, char neg_dir) {
  double raw_val = nmea_parse_double(gps, index) / 100.0;
  char direction = nmea_parse_alpha(gps, index + 1);
  uint8_t degrees = (uint8_t)raw_val;
  double minutes = (raw_val - (double)degrees) * 100.0;
  if (gps->last_error) {
    return 0.0;
  }
  double val = (double)degrees + minutes / 60.0;
  if (direction == pos_dir) {
    return val;
  }
  if (direction == neg_dir) {
    return -val;
  }
  gps->last_error = NMEA_INVALID_DIRECTION;
  return 0.0;
}

static inline float get_latitude(struct NMEA_decoder* gps, uint8_t index) {
  return convert_to_metric(gps, index, 'N', 'S');
}

static inline double get_latitude_double(struct NMEA_decoder* gps, uint8_t index) {
  return convert_to_metric_double(gps, index, 'N', 'S');
}

static inline float get_longitude(struct NMEA_decoder* gps, uint8_t index) {
  return convert_to_metric(gps, index, 'E', 'W');
}

static inline double get_longitude_double(struct NMEA_decoder* gps, uint8_t index) {
  return convert_to_metric_double(gps, index, 'E', 'W');
}

void nmea_init(struct NMEA_decoder* gps) {
  gps->buffer_head = 0;
  gps->buffer_size = 0;
  gps->ready = 0;
  gps->last_error = 0;
  gps->index = 0;
  gps->index_offset = 0;
}

uint8_t nmea_encode(struct NMEA_decoder* gps, char c) {
  if (gps->buffer_size >= NMEA_BUFFER_SIZE) {
    return 0;
  }
  set_char(gps, gps->buffer_size, c);
  if (c == '\n' &&
      gps->buffer_size >= 2 &&
      get_char(gps, gps->buffer_size - 1) == '\r') {
    ++gps->ready;
  }
  ++gps->buffer_size;
  return 1;
}

uint8_t nmea_is_command_type(struct NMEA_decoder* gps, const char* cmd) {
  if (gps->buffer_size < 7) {
    gps->last_error = NMEA_MESSAGE_TOO_SHORT;
    return 0;
  }

  if (get_char(gps, 0) != '$' || get_char(gps, 1) != 'G') {
    gps->last_error = NMEA_INVALID_MESSAGE_START;
    return 0;
  }

  // Check for a cmd match, while accumulating checksum
  uint8_t index = 3;
  uint8_t checksum = get_char(gps, 1) ^ get_char(gps, 2);
  for (; index < gps->buffer_size; ++index) {
    const char c = get_char(gps, index);
    if (c == '*' || c == ',') {
      if (cmd[index - 3]) {
        // cmd is longer than the buffer string
        return 0;
      }
      break;
    }
    checksum ^= c;
    if (c != cmd[index - 3]) {
      // mismatch
      return 0;
    }
  }

  // continue to accumulate checksum
  const uint16_t buffer_minus_checksum = gps->buffer_size - 2;
  for (; index < buffer_minus_checksum; ++index) {
    const char c = get_char(gps, index);
    if (c == '\r' || c == '\n') {
      // truncated
      break;
    }
    if (c == '*') {
      // validate the checksum matches
      if (get_char(gps, index + 1) != hex_char(checksum >> 4) ||
          get_char(gps, index + 2) != hex_char(checksum & 0x0F)) {
        gps->last_error = NMEA_CHECKSUM_ERROR;
        return 0;
      }
      return 1;
    }
    checksum ^= c;
  }

  // reached the end of the buffer before encountering a '*'
  gps->last_error = NMEA_MESSAGE_TRUNCATED;
  return 0;
}


char nmea_parse_sat_type(struct NMEA_decoder* gps) {
  if (gps->buffer_size < 3) {
    gps->last_error = NMEA_MESSAGE_TOO_SHORT;
    return 0;
  }
  return get_char(gps, 2);
}

void nmea_command_done(struct NMEA_decoder* gps) {
  if (gps->ready == 0) {
    // No command was waiting
    return;
  }

  // look for the end of the buffer or the start of the next command
  char last_c = get_char(gps, 0);
  while (gps->buffer_size > 0 && last_c != '\n') {
      DISABLE_INTERRUPTS();
      last_c = get_char(gps, 0);
      --gps->buffer_size;
      gps->buffer_head = (gps->buffer_head + 1) & NMEA_BUFFER_MASK;
      ENABLE_INTERRUPTS();
  }

  DISABLE_INTERRUPTS();
  --gps->ready;
  ENABLE_INTERRUPTS();
  gps->index = 0;
  gps->index_offset = 0;
}

uint32_t nmea_parse_u32(struct NMEA_decoder* gps, uint8_t index) {
  if (!find_index(gps, index)) {
    return 0;
  }
  return parse_u32(gps, gps->index_offset, 0);
}

uint16_t nmea_parse_u16(struct NMEA_decoder* gps, uint8_t index) {
  if (!find_index(gps, index)) {
    return 0;
  }
  return parse_u16(gps, gps->index_offset);
}

uint8_t nmea_parse_u8(struct NMEA_decoder* gps, uint8_t index) {
  if (!find_index(gps, index)) {
    return 0;
  }
  return parse_u8(gps, gps->index_offset);
}

float nmea_parse_float(struct NMEA_decoder* gps, uint8_t index) {
  if (!find_index(gps, index)) {
    return 0;
  }
  return parse_float(gps, gps->index_offset);
}

double nmea_parse_double(struct NMEA_decoder* gps, uint8_t index) {
  if (!find_index(gps, index)) {
    return 0;
  }
  return parse_double(gps, gps->index_offset);
}

char nmea_parse_alpha(struct NMEA_decoder* gps, uint8_t index) {
  if (!find_index(gps, index)) {
    return 0;
  }
  const char c = get_char(gps, gps->index_offset);
  if (c < 'A' || c > 'Z') {
    switch (c) {
      case ',':
      case '\r':
      case '\n':
      case '*':
        gps->last_error = NMEA_EMPTY_FIELD;
        break;
      default:
        gps->last_error = NMEA_NON_ALPHA_CHARACTER;
        break;
    }

    return 0;
  }

  return c;
}

void nmea_gga_time(
    struct NMEA_decoder* gps,
    uint8_t* hours,
    uint8_t* minutes,
    uint8_t* seconds) {
  parse_time(gps, 1, hours, minutes, seconds);
}

float nmea_gga_latitude(struct NMEA_decoder* gps) {
  return get_latitude(gps, 2);
}

double nmea_gga_latitude_double(struct NMEA_decoder* gps) {
  return get_latitude_double(gps, 2);
}

void nmea_gga_latitude_dms(
    struct NMEA_decoder* gps,
    uint8_t* degrees,
    uint8_t* minutes,
    float* seconds,
    char* direction) {
  convert_to_dms(
      gps,
      2,
      degrees,
      minutes,
      seconds,
      direction);
}

float nmea_gga_longitude(struct NMEA_decoder* gps) {
  return get_longitude(gps, 4);
}

double nmea_gga_longitude_double(struct NMEA_decoder* gps) {
  return get_longitude_double(gps, 4);
}

void nmea_gga_longitude_dms(
    struct NMEA_decoder* gps,
    uint8_t* degrees,
    uint8_t* minutes,
    float* seconds,
    char* direction) {
  convert_to_dms(
      gps,
      4,
      degrees,
      minutes,
      seconds,
      direction);
}

NMEA_GPS_quality nmea_gga_quality(struct NMEA_decoder* gps) {
  uint32_t quality = nmea_parse_u32(gps, 6);
  if (quality > NMEA_QUALITY_WAAS) {
    gps->last_error = NMEA_VALUE_OUT_OF_RANGE;
  }
  return (NMEA_GPS_quality)quality;
}

void nmea_gsa_satellite_prn(
    struct NMEA_decoder* gps,
    uint8_t prn_numbers[],
    uint8_t* num_satellites) {
  *num_satellites = 0;
  uint8_t i = 0;
  for (; i < 12; ++i) {
    uint8_t prn = nmea_parse_u8(gps, 3 + i);
    if (gps->last_error == 0) {
      if (prn >= 33 && prn <= 64) {
        // SBAS offset
        prn += 87;
      }
      prn_numbers[*num_satellites] = prn;
      *num_satellites += 1;
    } else if (gps->last_error == NMEA_EMPTY_FIELD) {
      gps->last_error = 0;
    } else {
      *num_satellites = 0;
      return;
    }
  }
}

void nmea_gsv_satellites(
    struct NMEA_decoder* gps,
    struct SatelliteInView sat[],
    uint8_t* num_found) {
  if (gps->last_error) {
    return;
  }
  *num_found = 0;
  while (*num_found < 4) {
    uint8_t offset = 4 + (*num_found) * 4;
    sat[*num_found].prn = nmea_parse_u8(gps, offset);
    sat[*num_found].elevation = nmea_parse_u8(gps, offset + 1);
    sat[*num_found].azimuth = nmea_parse_u16(gps, offset + 2);
    sat[*num_found].snr = nmea_parse_u16(gps, offset + 3);

    if (gps->last_error == NMEA_BAD_INDEX) {
      // This error is acceptable because messages can be truncated
      gps->last_error = 0;
      return;
    }

    if (gps->last_error == NMEA_EMPTY_FIELD) {
      // Sometimes fields are not populated
      gps->last_error = 0;
    }

    *num_found += 1;
  }
}

void nmea_gll_time(
    struct NMEA_decoder* gps,
    uint8_t* hours,
    uint8_t* minutes,
    uint8_t* seconds) {
  parse_time(gps, 5, hours, minutes, seconds);
}

float nmea_gll_latitude(struct NMEA_decoder* gps) {
  return get_latitude(gps, 1);
}

double nmea_gll_latitude_double(struct NMEA_decoder* gps) {
  return get_latitude_double(gps, 1);
}

void nmea_gll_latitude_dms(
    struct NMEA_decoder* gps,
    uint8_t* degrees,
    uint8_t* minutes,
    float* seconds,
    char* direction) {
  convert_to_dms(
      gps,
      1,
      degrees,
      minutes,
      seconds,
      direction);
}

float nmea_gll_longitude(struct NMEA_decoder* gps) {
  return get_longitude(gps, 3);
}

double nmea_gll_longitude_double(struct NMEA_decoder* gps) {
  return get_longitude_double(gps, 3);
}

void nmea_gll_longitude_dms(
    struct NMEA_decoder* gps,
    uint8_t* degrees,
    uint8_t* minutes,
    float* seconds,
    char* direction) {
  convert_to_dms(
      gps,
      3,
      degrees,
      minutes,
      seconds,
      direction);
}

//
// $GPRMC
//

float nmea_rmc_latitude(struct NMEA_decoder* gps) {
  return get_latitude(gps, 3);
}

double nmea_rmc_latitude_double(struct NMEA_decoder* gps) {
  return get_latitude_double(gps, 3);
}

void nmea_rmc_latitude_dms(
    struct NMEA_decoder* gps,
    uint8_t* degrees,
    uint8_t* minutes,
    float* seconds,
    char* direction) {
  convert_to_dms(
      gps,
      3,
      degrees,
      minutes,
      seconds,
      direction);
}

float nmea_rmc_longitude(struct NMEA_decoder* gps) {
  return get_longitude(gps, 5);
}

double nmea_rmc_longitude_double(struct NMEA_decoder* gps) {
  return get_longitude_double(gps, 5);
}

void nmea_rmc_longitude_dms(
    struct NMEA_decoder* gps,
    uint8_t* degrees,
    uint8_t* minutes,
    float* seconds,
    char* direction) {
  convert_to_dms(
      gps,
      5,
      degrees,
      minutes,
      seconds,
      direction);
}

void nmea_rmc_date(
    struct NMEA_decoder* gps, uint8_t* day, uint8_t* month, uint8_t* year) {
  uint32_t datestamp = nmea_rmc_datestamp(gps);
  *year = (uint8_t)(datestamp % 100);
  datestamp /= 100;
  *month = (uint8_t)(datestamp % 100);
  datestamp /= 100;
  *day = (uint8_t)(datestamp);
}
