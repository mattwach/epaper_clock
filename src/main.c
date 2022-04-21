// Epaper Clock with Temperature / Humidity and Pressure
//
// Offers different setups:
//
// USE_CPU_CRYSTAL + SOFTWARE_UART:
//  Easier build with a nano.  Intended for use with wall power.
//  A good choice when prototyping and experimenting.
//
// USE_32K_CRYSTAL + HARDWARE_UART:
//  Uses 32k crystal with Atmega328p "standalone" (although a
//  properly stripped pro mini could work).  Should run about a
//  year on AAA batteries.
//
// See schematic/ directory for hardware details (free kicad software
// is needed to view the files).

#include <lowpower/lowpower.h>
#include <port/port.h>
#include <twi/twi.h>
#include <uart/uart.h>
#include <weather/ms8607.h>

#include <avr/interrupt.h>
#include <util/delay.h>
#include <time.h>

#include "buttons.h"
#include "display.h"
#include "eeprom_vars.h"
#include "gps.h"
#include "menu.h"

// Clock drift correction
// If your clock runs too fast or too slow, then you can enable these
//#define CORRECT_CLOCK_DRIFT
// number of seconds that a second should be added or removed
//#define CLOCK_DRIFT_SECONDS_PER_CORRECT 1800
// define this if thwe clock is too slow, otherwise leave it commented out
//#define CLOCK_DRIFT_TOO_SLOW

// When using 32k crystal + hardware UART, the HEARTBEAT flashes for 1ms every
// second and tells us that the clock is working and the code is not hung up.
//
// CPU crystal + software uart flashes the LED in the main loop instead of the
// interrupt so that timing-critical software uart interrupts are not delayed (even
// 1ms of delay is a problem).  This means a more erratic update but it does
// settle down to abuot 1 update per second after the GPS locks.
#define HEARTBEAT_LED_DDR DDRD
#define HEARTBEAT_LED_PORT PORTD
#define HEARTBEAT_LED_PIN 5

#if defined(USE_32K_CRYSTAL) && defined(USE_CPU_CRYSTAL)
  #error Define USE_32K_CRYSTAL or USE_CPU_CRYSTAL, not both.
#endif

#if defined(HARDWARE_UART) && defined(SOFTWARE_UART)
  #error Define HARDWARE_UART or SOFTWARE_UART, not both.
#endif

#if defined(USE_32K_CRYSTAL) && defined(SOFTWARE_UART)
  #error SORTWARE_UART and USE_32K_CRYSTAL are not tested to work together.  Remove this error from main.c at your own risk.
#endif

// Pressure/Humidity/Temperature (PHT) sensor calibration data
struct MS8607 ms8607;
// Seconds since Jan 1, 2000.  avr time.h functions use this format.
// Don't access this value directly because the interrupt handler increments it.
// Instead use snapshot_time_y2k which will disable interrupts and grab a snapshot of
// the value.
volatile time_t current_time_y2k;

// Stores non-volatile settings, like UTC time offset and 12/24h preference.
struct EEPromVars eeprom;

// This is needed to allow the LCD to come up after a low power state (which
// turns off SPI and leaves things in a bit of a mess).
void oledm_ifaceInit(void);

// A simple boolean that is set after the GPS has locks a position, it will stay
// true even if the GPS loses it's lock (in which case we assume the location
// can not changed).
uint8_t position_set_trigger;

// A flag that says if we are in the setting menu or not
uint8_t menu_mode;

#ifdef CORRECT_CLOCK_DRIFT
time_t next_clock_drift_correction;
#endif

// Call this to get the current time.  Avoid accessing current_time_y2k
// directly becuase there is a risk of the interrupt handler updating
// the time while in the middle of reading it.
static inline time_t snapshot_time_y2k(void) {
  cli();
  const time_t snapshot = current_time_y2k;
  sei();
  return snapshot;
}

static inline void heartbeat_on(void) {
  HEARTBEAT_LED_PORT |= (1 << HEARTBEAT_LED_PIN);
}

static inline void heartbeat_off(void) {
  HEARTBEAT_LED_PORT &= ~(1 << HEARTBEAT_LED_PIN);
}

static void heartbeat(void) {
  // Turning on the heartbeat LED for 1ms uses almost no power and due to
  // photoluminescence / persistance of vision, the flash can still be easily
  // detected by the human eye.  Without a "heartbeat" it's really hard to tell
  // if the unit is powered on and working properly.
  heartbeat_on();
  _delay_ms(1);
  heartbeat_off();
}

#if defined(USE_32K_CRYSTAL)
  static void timer_init(void) {
    ASSR = 0x20; // Enable the external 32k oscillator
    TCCR2B = 0x05; // Divide by 128 (for an overflow once per second)
    TIMSK2 = (1 << TOIE2);  // Interrupt on timer 2 overflow
  }
#elif defined(USE_CPU_CRYSTAL)
  static void timer_init(void) {
    OCR1A = (F_CPU >> 8);  // when to fire the interrupt (when using the / 256 prescaler)
    TIMSK1 = 1 << OCIE1A; // Interrupt match on counter 1
    TCCR1B = (1 << WGM12) | 0x04;  // OCR1A is top, / 256 count
  }
#else
  #error Please define either USE_32K_CRYSTAL or USE_CPU_CRYSTAL
#endif


// lowpower idle is a bit complicated due to all of the ifdefs,
// so it's movced to it's own separate function.
inline static void idle(void) {
  lowpower_idle(
      SLEEP_FOREVER,
      ADC_OFF,
#if defined(USE_32K_CRYSTAL)
      TIMER2_ON,
      TIMER1_OFF,
#elif defined(USE_CPU_CRYSTAL)
      TIMER2_OFF,
      TIMER1_ON,
#endif
#if defined(HARDWARE_UART)
      TIMER0_OFF,
#elif defined(SOFTWARE_UART)
      TIMER0_ON,  // Software UART needs timer 0 to function
#endif
      SPI_OFF,
#if defined(HARDWARE_UART) || defined(DEBUG)
      USART0_ON,
#elif defined(SOFTWARE_UART)
      USART0_OFF,
#endif
      TWI_OFF);
} 

static void wait_for_next_second(void) {
#ifdef SOFTWARE_UART
  // We can't blink in the interrupt handler so blink here instead
  heartbeat();
#endif

#if defined(USE_32K_CRYSTAL)
  if (menu_mode || gps_is_enabled()) {
    // GPS is enabled so we need to use the idle form of power save
    // to be able to receive the UART interrupts

    // In menu_mode we choose idle becuase the low-latency screen updates are
    // already using power and it reduces the number of overall system
    // states (e.g. we don't have to separately test menu+idle and
    // menu+powersave).  Also the menu state is expected to be a rare event.
    idle();
  } else {
    // GPS is powered down so we can use the ultra low power 32k oscillator
    // mode.
    lowpower_powerSave(SLEEP_FOREVER, ADC_OFF, BOD_OFF, TIMER2_ON);
  }
#elif defined(USE_CPU_CRYSTAL)
  // Always use idle mode when going with the cpu crystal
  // as power save would just lock things up.
  idle();
#else
  #error Please define either USE_32K_CRYSTAL or USE_CPU_CRYSTAL
#endif

  // check on every wait to provide relief to the GPS receive buffer, which may
  // not be large enough to endure several rounds of information (waiting too long
  // leads to new messages being lost until the buffer is processed)
  check_gps(&current_time_y2k);  // Using current_time_y2k on purpose so it can get updated
}


#if defined(USE_32K_CRYSTAL)
  static uint8_t timer_ticks(void) {
    return TCNT2;
  }
#elif defined(USE_CPU_CRYSTAL)
  static uint8_t timer_ticks(void) {
    // can't read TCNT1H directly because it is only updated when TCNT1L is read
    return (TCNT1 >> 8);
  }
#else
  #error Please define either USE_32K_CRYSTAL or USE_CPU_CRYSTAL
#endif

// one-time initialization function that calls several helpers
static void init(void) {
  // eventually, this will come from GPS
  menu_mode = 0;
  current_time_y2k = 694310400; // should be 1/1/2022
  set_system_time(current_time_y2k);
  position_set_trigger = 0;
  load_eeprom(&eeprom);
  set_zone(-((int32_t)eeprom.utc_offset) * ONE_HOUR);
  // Set unused pins as pullups
  // All PB pins are used
  // PC0, PC1, PC2, PC3 are unused
  PORTC |= (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);
  // PD1, PD2 are unused (PD2 is used by software uart but that will be
  // fixed in gps_init() below).
  PORTD |= (1 << 1) | (1 << 2);
  buttons_init(timer_ticks);
  oledm_ifaceInit();
  display_enable_spi();
  HEARTBEAT_LED_DDR |= (1 << HEARTBEAT_LED_PIN);
  _delay_ms(200);
  // If the UTC button was held on startup, never enable GPS.  This is
  // for power measurement
  gps_init(!select_button_is_pressed(), current_time_y2k);
  ms8607_init(&ms8607);
  timer_init();
  sei();  // enable global interrupts
  display_init();
}

// Sample data from the Pressure/Humidity/Temperature sensor
// and update the epaper display with the latest information.
static void collect_data_and_update_display(uint8_t button_pressed, const time_t current_ytk) {
  if (menu_mode) {
    // menummode uses the higher-power eapaper fast-update
    // thus we do not disable SPI
    menu_mode = update_menu(button_pressed, current_ytk, &eeprom);
    button_pressed = 0;  // Don't carry button_pressed across mode changes
  } else if (button_pressed == OPTION_WAS_PRESSED) {
      menu_mode = 1;
      display_enable_spi();
      menu_init(current_ytk, &eeprom);
  }

  if (!menu_mode) {
    const struct GPSStats* gps_stats = gps_get_stats();
    struct DisplayInfo dinfo;
    dinfo.time_y2k = current_ytk;
    dinfo.position_was_set = gps_position_was_set();

    if (button_pressed == SELECT_WAS_PRESSED) {
      gps_stat_show_policy(
        (gps_stats->show_policy == GPS_STATS_HIDE) ?
          GPS_STATS_SHOW :
          GPS_STATS_HIDE);
    }

    display_enable_spi();
    ms8607_read_values(
        &ms8607, &(dinfo.temp_cc), &(dinfo.pressure_pa), &(dinfo.humidity_cpct));
    update_display(&dinfo, &eeprom, wait_for_next_second);
    display_disable_spi();
  }
}

// called when the chip is awoken from sleep, usually once per second
// but possibly more often, ofr example if a button is pressed or new
// GPS data is received.
static time_t loop(time_t next_update) {
  // Update if GPS says so or enough time has passed to advance to the next
  // minute
  const uint32_t current_ytk = snapshot_time_y2k();
  uint32_t difference = current_ytk > next_update ?
    current_ytk - next_update :
    next_update - current_ytk;
  const uint8_t position_was_set = gps_position_was_set();
  const uint8_t button_pressed = button_was_pressed();

  if ((difference >= 60) ||
      (current_ytk >= next_update) ||
      (position_set_trigger != position_was_set) ||
      button_pressed) {
    clear_button_press_state();
    collect_data_and_update_display(button_pressed, current_ytk);
    next_update = current_ytk + 60;
    const uint8_t r = next_update % 60;
    // accurate within one second as the GPS and 32k clock are not in sync
    if ((r > 0) && (r < 59)) {
      // This can happen when GPS sets the time.  Align the next update
      // with the next minute
      next_update -= r;
    }
    if (position_was_set) {
      // one time trigger when position is made available
#ifdef CORRECT_CLOCK_DRIFT
      if (!position_set_trigger) {
        next_clock_drift_correction = current_ytk + CLOCK_DRIFT_SECONDS_PER_CORRECT;
      } else if (current_ytk >= next_clock_drift_correction) {
        cli();
#ifdef CLOCK_DRIFT_TOO_SLOW
        ++current_time_y2k;
#else
        --current_time_y2k;
#endif
        sei();
        next_clock_drift_correction += CLOCK_DRIFT_SECONDS_PER_CORRECT;
      }
#endif
      position_set_trigger = position_was_set;
    }
  }

  return next_update;
}

// Code starts with this function
int main(void) {
  init();
  time_t next_update = snapshot_time_y2k();

  while (1) {
    wait_for_next_second();
    next_update = loop(next_update);
  }
}

// Called whenever Timer2 overflows from 0xff -> 0x00 which as-configured with
// a 32768Hz crystal, will happen once per second.
#if defined(USE_32K_CRYSTAL)
ISR(TIMER2_OVF_vect)
#elif defined(USE_CPU_CRYSTAL)
ISR(TIMER1_COMPA_vect)
#else
  #error Please define either USE_32K_CRYSTAL or USE_CPU_CRYSTAL
#endif
{
#ifdef HARDWARE_UART
  // The 1ms delay messes up the timing of the software uart
  heartbeat();
#endif
  ++current_time_y2k;
}


