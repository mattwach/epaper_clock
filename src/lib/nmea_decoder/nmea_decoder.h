#ifndef NMEA_DECODER
#define NMEA_DECODER

#include <stdint.h>

// This is a NMEA string decoder which aims to be as light on resource
// use as possible.  This should allow the library to be used with
// even resource-constrained chips, such as the ATTiny series.
//
// Here is a typical NMEA string
// $GPGGA,181908.00,3404.7041778,N,07044.3966270,W,4,13,1.00,495.144,M,29.200,M,0.10,0000*40
//
// Tokens are separated by commas.  The string ends with a checksum, followed
// by a <CR><LF> combination. The first token is the message type and indicates
// what the fields that follow mean.
//
// The way this library saves on resources is by breaking processing down into
// fine-grained functions.  You call only the functions you need and, with
// appropriate compiler settings, only the functions you call will end up in
// the final called binary.
//
// For example, let's use the API to extract latitude and longitude from the
// example string above:
//
// #define NEMA_BUFFER_SIZE 1024
// struct NMEA_decoder gps;
// float latitude;
// float longitude;
//
// void main() {
//   nmea_init(&gps);
//   while (1) {
//     // get_serial_byte is a placeholder and will vary according to
//     // the library you use
//     char c = get_serial_byte();
//     nmea_encode(&gps, c);
//     for (; gps.ready; nmea_command_done(&gps)) {
//       gps.last_error = 0;
//       if (nmea_is_gpgga(&gps)) {
//         latitude = nmea_gga_latitude(&gps);
//         longitude = nmea_gga_longitude(&gps);
//         if (gps.last_error) {
//           // Something failed to parse, probably don't have a GPS lock yet.
//         }
//       }
//     }
//   }
// }
//
// Notes on the above:
//   - NMEA_BUFFER_SIZE should be set to contain all of the data that you
//     might see in a single GPS dump.  The size depends on how you configured
//     your GPS unit (what messages you activated).  A fast processor might be
//     able to clear the buffer between bytes sent (or if you set the baud
//     rate low enough), but it's generally safest to reserve enough space
//     to hold an entire dump from the unit.
//   - nmea_encode() would often be called in an interrupt handler.
//   - the gps_ready could be called in an interrupt handler but usually
//     it would be called in the main program loop, it depends on your code
//     architecture.
//   - nmea_command_done() needs to be called to free up buffer space for
//     a new command and should be called until gps.ready turns false.
//   - Calling the _get_ functions in the same order as index order (or the
//     order listed in this header file) is the most efficient approach.
//     Using other orders will have correct results but will waste extra
//     cycles searching through the data.
//
// Error Handling:
//   Error checking is mandatory for most GPS applications because GPS is
//   not reliable - it can take minutes to get a lock and sometimes a
//   lock never happens.  This library handles errors by setting the
//   last_error field in the NMEA_decoder structure.  This field is only
//   changed if there is an error, otherwise it is left alone.  Thus
//   you can call a set of parse function and check last_error afterwords
//   to know that one of these commands failed.
//
//   You could also check last_error after every parse call if you want
//   to know exactly which parese failed but this is generally overkill
//   as entire groups of parsing function will be expected to fail if
//   thier is no  GPS lock (latitude and longitude will both fail, no
//   need to check them individually).
//
//   Also, make sure to set last_error to zero before parsing, otherwise
//   errors will continue to persist, even when a lock is obtained.
//
// If NMEA_BUFFER_SIZE is exceeded, then all calls to nmea_encode() will
// drop the character (no place to put it), resulting in lost messages.

// NMEA_BUFFER_SIZE must be a power of 2
#ifndef NMEA_BUFFER_SIZE
#error "Please define NMEA_BUFFER_SIZE"
#endif

#if NMEA_BUFFER_SIZE > 65536
#error "Please set NMEA_BUFFER_SIZE < 65536"
#endif

#define NMEA_BUFFER_MASK (NMEA_BUFFER_SIZE - 1)

typedef enum 
{ 
  NMEA_OK = 0,
  NMEA_BAD_INDEX = 1,
  NMEA_NON_NUMERIC_CHARACTER = 2,
  NMEA_NEGATIVE_NUMBER = 3,
  NMEA_EMPTY_FIELD = 4,
  NMEA_NUMBER_OVERFLOW = 5,
  NMEA_CHECKSUM_ERROR = 6,
  NMEA_MESSAGE_TRUNCATED = 7,
  NMEA_INVALID_MESSAGE_START = 8,
  NMEA_MESSAGE_TOO_SHORT = 9,
  NMEA_INVALID_DIRECTION = 10,
  NMEA_NON_ALPHA_CHARACTER = 11,
  NMEA_VALUE_OUT_OF_RANGE = 12,
} NMEA_error;

typedef enum {
  NMEA_QUALITY_NO_FIX = 0,
  NMEA_QUALITY_SINGLE_POINT = 1,
  NMEA_QUALITY_PSEUDORANGE_DIFF = 2,
  NMEA_QUALITY_RTK_FIXED_AMBIGUITY = 4,
  NMEA_QUALITY_RTK_FLOATING_AMBIGUITY = 5,
  NMEA_QUALITY_DEAD_RECKONING = 6,
  NMEA_QUALITY_MANUAL_INPUT = 7,
  NMEA_QUALITY_SIMULATED = 8,
  NMEA_QUALITY_WAAS = 9,
} NMEA_GPS_quality;

struct NMEA_decoder {
  char buffer[NMEA_BUFFER_SIZE];
  volatile uint16_t buffer_head;  // indicates where the first byte of the next message
  volatile uint16_t buffer_size;  // indicates how many bytes are in the buffer
  volatile uint8_t ready;         // indicates how many commands are available to parse
  NMEA_error last_error; // Set only when an error occurs
  // Internal tracking
  uint8_t index;
  uint16_t index_offset;
};

void nmea_init(struct NMEA_decoder* gps);
// enocode returns zero if the encode failed.  Currently, this will only
// happen if the gps buffer is full.
uint8_t nmea_encode(struct NMEA_decoder* gps, char c);
void nmea_command_done(struct NMEA_decoder* gps);

// Generic parsing, prefer specific parsing below for better readability
uint8_t nmea_is_command_type(struct NMEA_decoder* gps, const char* cmd);
// Returns 'N', 'L', 'P', or '\0' corresponding to NMEA sattelite type.
char nmea_parse_sat_type(struct NMEA_decoder* gps);
char nmea_parse_alpha(struct NMEA_decoder* gps, uint8_t index);
uint32_t nmea_parse_u32(struct NMEA_decoder* gps, uint8_t index);
uint16_t nmea_parse_u16(struct NMEA_decoder* gps, uint8_t index);
uint8_t nmea_parse_u8(struct NMEA_decoder* gps, uint8_t index);
float nmea_parse_float(struct NMEA_decoder* gps, uint8_t index);
double nmea_parse_double(struct NMEA_decoder* gps, uint8_t index);

//
// $GxGGA - time. 3D location, undulation.
// (Note other messages also contain some of this data)
//

static inline uint8_t nmea_is_gga(struct NMEA_decoder* gps) {
  return nmea_is_command_type(gps, "GGA");
}
// returns the current UTC time in HHMMSS format.  i.e. 184532 -> 18:45 32
static inline uint32_t nmea_gga_timestamp(struct NMEA_decoder* gps) {
  return nmea_parse_u32(gps, 1);
}
void nmea_gga_time(
    struct NMEA_decoder* gps,
    uint8_t* hours,
    uint8_t* minutes,
    uint8_t* seconds);
float nmea_gga_latitude(struct NMEA_decoder* gps);
double nmea_gga_latitude_double(struct NMEA_decoder* gps);
void nmea_gga_latitude_dms(
    struct NMEA_decoder* gps,
    uint8_t* degrees,
    uint8_t* minutes,
    float* seconds,
    char* direction);
float nmea_gga_longitude(struct NMEA_decoder* gps);
double nmea_gga_longitude_double(struct NMEA_decoder* gps);
void nmea_gga_longitude_dms(
    struct NMEA_decoder* gps,
    uint8_t* degrees,
    uint8_t* minutes,
    float* seconds,
    char* direction);
NMEA_GPS_quality nmea_gga_quality(struct NMEA_decoder* gps);
static inline uint16_t nmea_gga_num_satellites_in_use(struct NMEA_decoder* gps) {
  return nmea_parse_u16(gps, 7);
}

static inline float nmea_gga_hdop(struct NMEA_decoder* gps) {
  return nmea_parse_float(gps, 8);
}
static inline float nmea_gga_altitude(struct NMEA_decoder* gps) {
  return nmea_parse_float(gps, 9);
}
static inline char nmea_gga_altitude_units(struct NMEA_decoder* gps) {
  return nmea_parse_alpha(gps, 10);
}
static inline float nmea_gga_undulation(struct NMEA_decoder* gps) {
  return nmea_parse_float(gps, 11);
}
static inline char nmea_gga_undulation_units(struct NMEA_decoder* gps) {
  return nmea_parse_alpha(gps, 12);
}
static inline float nmea_gga_age(struct NMEA_decoder* gps) {
  return nmea_parse_float(gps, 13);
}
static inline uint16_t nmea_gga_stn_id(struct NMEA_decoder* gps) {
  return nmea_parse_u16(gps, 14);
}

//
// $GPGSA - "dillusion of precision" and active satellites
//

static inline uint8_t nmea_is_gsa(struct NMEA_decoder* gps) {
  return nmea_is_command_type(gps, "GSA");
}
static inline char nmea_gsa_mode_ma(struct NMEA_decoder* gps) {
  return nmea_parse_alpha(gps, 1);
}
static inline uint8_t nmea_gsa_mode_123(struct NMEA_decoder* gps) {
  return nmea_parse_u8(gps, 2);
}
// provide an array with at least 12 slots
void nmea_gsa_satellite_prn(
    struct NMEA_decoder* gps,
    uint8_t prn_numbers[],
    uint8_t* num_satellites);
static inline float nmea_gsa_pdop(struct NMEA_decoder* gps) {
  return nmea_parse_float(gps, 15);
}
static inline float nmea_gsa_hdop(struct NMEA_decoder* gps) {
  return nmea_parse_float(gps, 16);
}
static inline float nmea_gsa_vdop(struct NMEA_decoder* gps) {
  return nmea_parse_float(gps, 17);
}

//
// $GPGSV - Satellites in view
//

struct SatelliteInView {
  uint8_t prn;
  uint8_t elevation;
  uint16_t azimuth;
  uint8_t snr;
};
static inline uint8_t nmea_is_gsv(struct NMEA_decoder* gps) {
  return nmea_is_command_type(gps, "GSV");
}
static inline uint8_t nmea_gsv_num_msgs(struct NMEA_decoder* gps) {
  return nmea_parse_u8(gps, 1);
}
static inline uint8_t nmea_gsv_msg_num(struct NMEA_decoder* gps) {
  return nmea_parse_u8(gps, 2);
}
static inline uint16_t nmea_gsv_num_sats(struct NMEA_decoder* gps) {
  return nmea_parse_u8(gps, 3);
}
// provide an array of 4 SatelliteInView objects (the maximum per statement).
void nmea_gsv_satellites(
    struct NMEA_decoder* gps,
    struct SatelliteInView sat[],
    uint8_t* num_found);

//
// $GPGLL - 2D Position and UTC time.
//
static inline uint8_t nmea_is_gll(struct NMEA_decoder* gps) {
  return nmea_is_command_type(gps, "GLL");
}
// returns the current UTC time in HHMMSS format.  i.e. 184532 -> 18:45 32
static inline uint32_t nmea_gll_timestamp(struct NMEA_decoder* gps) {
  return nmea_parse_u32(gps, 5);
}
void nmea_gll_time(
    struct NMEA_decoder* gps,
    uint8_t* hours,
    uint8_t* minutes,
    uint8_t* seconds);
float nmea_gll_latitude(struct NMEA_decoder* gps);
double nmea_gll_latitude_double(struct NMEA_decoder* gps);
void nmea_gll_latitude_dms(
    struct NMEA_decoder* gps,
    uint8_t* degrees,
    uint8_t* minutes,
    float* seconds,
    char* direction);
float nmea_gll_longitude(struct NMEA_decoder* gps);
double nmea_gll_longitude_double(struct NMEA_decoder* gps);
void nmea_gll_longitude_dms(
    struct NMEA_decoder* gps,
    uint8_t* degrees,
    uint8_t* minutes,
    float* seconds,
    char* direction);
static inline char nmea_gll_data_status(struct NMEA_decoder* gps) {
  return nmea_parse_alpha(gps, 6);
}
static inline char nmea_gll_mode_ind(struct NMEA_decoder* gps) {
  return nmea_parse_alpha(gps, 7);
}

//
// $GPVTD - Track make good and ground speed
//

static inline uint8_t nmea_is_vtg(struct NMEA_decoder* gps) {
  return nmea_is_command_type(gps, "VTG");
}
static inline float nmea_vtg_track_true(struct NMEA_decoder* gps) {
  return nmea_parse_float(gps, 1);
}
static inline float nmea_vtg_track_mag(struct NMEA_decoder* gps) {
  return nmea_parse_float(gps, 3);
}
static inline float nmea_vtg_speed_knots(struct NMEA_decoder* gps) {
  return nmea_parse_float(gps, 5);
}
static inline float nmea_vtg_speed_km(struct NMEA_decoder* gps) {
  return nmea_parse_float(gps, 7);
}
static inline char nmea_vtg_mode_ind(struct NMEA_decoder* gps) {
  return nmea_parse_alpha(gps, 9);
}

//
// $GPRMC - time, date, position, track made good, speed
// Note: other messages also contain much of this data
//
static inline uint8_t nmea_is_rmc(struct NMEA_decoder* gps) {
  return nmea_is_command_type(gps, "RMC");
}
// returns the current UTC time in HHMMSS format.  i.e. 184532 -> 18:45 32
static inline uint32_t nmea_rmc_timestamp(struct NMEA_decoder* gps) {
  return nmea_parse_u32(gps, 1);
}
static inline void nmea_rmc_time(
    struct NMEA_decoder* gps,
    uint8_t* hours,
    uint8_t* minutes,
    uint8_t* seconds) {
  return nmea_gga_time(gps, hours, minutes, seconds);
}
static inline char nmea_rmc_pos_status(struct NMEA_decoder* gps) {
  return nmea_parse_alpha(gps, 2);
}
float nmea_rmc_latitude(struct NMEA_decoder* gps);
double nmea_rmc_latitude_double(struct NMEA_decoder* gps);
void nmea_rmc_latitude_dms(
    struct NMEA_decoder* gps,
    uint8_t* degrees,
    uint8_t* minutes,
    float* seconds,
    char* direction);
float nmea_rmc_longitude(struct NMEA_decoder* gps);
double nmea_rmc_longitude_double(struct NMEA_decoder* gps);
void nmea_rmc_longitude_dms(
    struct NMEA_decoder* gps,
    uint8_t* degrees,
    uint8_t* minutes,
    float* seconds,
    char* direction);
static inline float nmea_rmc_speed_knots(struct NMEA_decoder* gps) {
  return nmea_parse_float(gps, 7);
}
static inline float nmea_rmc_track_true(struct NMEA_decoder* gps) {
  return nmea_parse_float(gps, 8);
}
// DDMMYY
static inline uint32_t nmea_rmc_datestamp(struct NMEA_decoder* gps) {
  return nmea_parse_u32(gps, 9);
}
void nmea_rmc_date(
    struct NMEA_decoder* gps, uint8_t* day, uint8_t* month, uint8_t* year);
static inline float nmea_rmc_mag_var(struct NMEA_decoder* gps) {
  return nmea_parse_float(gps, 10);
}
static inline char nmea_rmc_var_dir(struct NMEA_decoder* gps) {
  return nmea_parse_alpha(gps, 11);
}
static inline char nmea_rmc_mode_ind(struct NMEA_decoder* gps) {
  return nmea_parse_alpha(gps, 12);
}

#endif

