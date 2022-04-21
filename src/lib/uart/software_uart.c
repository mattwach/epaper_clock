#include "software_uart.h"
#include <avr/io.h>
#include <avr/interrupt.h>

// Assume F_CPU is 16000000 for now
//
// Strategy, use an 8-bit timer to cover the span of a 10-bit word (8N1)
// For best resolution, the time required to output 10 bits shoulds be
// as clock to 256 timer ticks as possible without exceeding that value
//
// We then have a state machind that works as follows
//
// - Initial state is SERIAL_IDLE.
// - On first transistion, INT0 is called
//    - state becomes SERIAL_COLLECT
//    - reset and start timer 0
// - On each INT0 call, we check timer 0 to see how many bits the transisition
//   represents and shift in that many bits, too many results in SERIAL_ERROR state
// - At some point, timer0 will trigger.  At this point
//   - We assert that SERIAL_ERROR is false
//   - We assert that the state is high (1)
//   - We shift in the required number of 1's on the high side
//   - We call the byte receive callback function
//   - We reset the state to idle and stop timer 0

// Timer setup.
// 9600 baud represents 0.1042 ms per bit.
//
// We want to stop in the middle of the supposed stop bit
// which is bit 9.5  This occurs around 0.9896 ms
//
// This we sill select be x64 as 
//   16 Mhz is 0.0625 us per cycle
//   * x64 makes it 4 us per cycle
//   * 256 make is 1.024 ms within a 256 count.
//
// Thus the per bit count is 102.4 us / 4 us ~ 26 bits
// and the 9.5 bit count is 247
#define TIMER0_CLOCK_SELECT 0x03  // clk/256
#define TIMER0_CLOCK_DIVIDE 64  // must correlate with TIMER0_CLOCK_SELECT
#define BAUD_RATE 9600
#define CLOCKS_PER_BIT (F_CPU / TIMER0_CLOCK_DIVIDE / BAUD_RATE) 
#define CLOCKS_TO_STOP_BIT (F_CPU * 95 / TIMER0_CLOCK_DIVIDE / BAUD_RATE / 10)

#define INT0_DDR DDRD
#define INT0_PORT PORTD
#define INT0_INPUT PIND
#define INT0_PIN 2

#define SOFTWARE_UART_BUFFER_MASK (SOFTWARE_UART_BUFFER_SIZE - 1)

typedef enum {
  SERIAL_IDLE = 0,
  SERIAL_COLLECT = 1,
  SERIAL_ERROR = 2,
} SerialState;

struct SoftwareUART {
  void (*byte_received)(void);
  SerialState state;
  SerialError last_error;
  uint8_t bits_collected;
  uint8_t byte;
  uint8_t last_timer_check;
  uint8_t buffer_head;
  uint8_t buffer_tail;
  uint8_t buffer[SOFTWARE_UART_BUFFER_SIZE];
};

struct SoftwareUART suart;

void software_uart_init(void (*byte_received)(void)) {
  suart.byte_received = byte_received;
  suart.state = SERIAL_IDLE;
  suart.buffer_head = 0;
  suart.buffer_tail = 0;

  TCCR0B = 0;  // disable the timer if it happens to be running.
  TCNT0 = 0; // reset the counter (possibly unneeded)
  OCR0A = CLOCKS_TO_STOP_BIT;  // This never changes
  TIMSK0 |= (1 << OCIE0A); // Interrupt on the stop bit (if the timer was
                           // running, which it currently is not.)

  // These are generally unneeded as this is the chip's default state
  INT0_DDR &= ~(1 << INT0_PIN);  // Set as a input
  INT0_PORT &= ~(1 << INT0_PIN);  // pullup disabled
  software_uart_enable();
}


void software_uart_enable(void) {
  // interrupt on any transition
  EICRA |= (1 << ISC00);  // Any logic change on INT0 generates an interrupt.
  EIMSK |= (1 << INT0);  // Enable the interrupt
}

void software_uart_disable(void) {
  EIMSK &= ~(1 << INT0);  // Disable the interrupt
  EICRA &= ~(1 << ISC00);  // Any logic change on INT0 generates an interrupt.
  EIFR |= (1 << INTF0); // Clear any pending interrupt
  suart.state = SERIAL_IDLE;
  suart.buffer_head = 0;
  suart.buffer_tail = 0;
}

uint8_t software_uart_read(uint8_t* c) {
  if (suart.buffer_head == suart.buffer_tail) {
    // buffer is empty
    return 0;
  }
  *c = suart.buffer[suart.buffer_head];
  suart.buffer_head = (suart.buffer_head + 1) & SOFTWARE_UART_BUFFER_MASK;
  return 1;
}

SerialError software_uart_last_error(void) {
  const SerialError last_error = suart.last_error;
  suart.last_error = SERIAL_OK;
  return last_error;
}

// Called when on the low to high AND high to low edges
ISR(INT0_vect) {
  uint8_t i, bit_count;
  const uint8_t timer0 = TCNT0;

  switch (suart.state) {
    case SERIAL_IDLE:
      // We expect this to be a start bit...

      // Start the timer.  Even if there is an unexpected state (error),
      // the timer can clear the problem for potential future success.
      TCNT0 = 0;
      TCCR0B = TIMER0_CLOCK_SELECT; 
      suart.bits_collected = 0;  // how many bits collected so far
      suart.byte = 0;  // the byte we are building
      suart.last_timer_check = 0;  // used to determined bit counts

      if (INT0_INPUT & (1 << INT0_PIN)) {
        // Start bit should be low
        suart.last_error = SERIAL_ERROR_BAD_START_BIT;
        // for this case the start bit has the wrong polarity so try staying in idle
        suart.state = SERIAL_ERROR;
      } else {
        suart.state = SERIAL_COLLECT;
      }
      break;
   case SERIAL_COLLECT:
      // look at timer to determine the bit count
      bit_count = (timer0 - suart.last_timer_check + (CLOCKS_PER_BIT / 2)) /
        CLOCKS_PER_BIT;
      suart.last_timer_check = timer0;

      if (bit_count < 1) {
        suart.last_error = SERIAL_ERROR_BIT_TOO_NARROW;
        suart.state = SERIAL_ERROR;
        break;
      }

      suart.bits_collected += bit_count;
      if (suart.bits_collected > 9) {
        suart.last_error = SERIAL_ERROR_TOO_MANY_BITS;
        suart.state = SERIAL_ERROR;
      }

      // do we shift in 1's or zeros?
      // a subtle thing to thing about - the current state is the INVERSE
      // of where we were in the past.
      if (INT0_INPUT & (1 << INT0_PIN)) {
        // We ARE high, so we WERE low
        // shift in zeros
        suart.byte >>= bit_count;
      } else {
        // We ARE low, so we WERE high
        // shift in ones
        for (i = 0; i < bit_count; ++i) {
          suart.byte = (suart.byte >> 1) | 0x80;
        }
      }
      break;
   default:
      // in an error state, do nothing
      break;
  }
}

ISR(TIMER0_COMPA_vect) {
  // disable the timer
  TCCR0B = 0;
  if (suart.state != SERIAL_COLLECT) {
    // Something went wrong.  Reset for the next byte.
    suart.state = SERIAL_IDLE;
    return;
  }

  suart.state = SERIAL_IDLE;

  if (((suart.buffer_tail + 1) & SOFTWARE_UART_BUFFER_MASK) == suart.buffer_head) {
    suart.last_error = SERIAL_ERROR_BUFFER_FULL;
    return;
  }

  // Only proceed if the stop bit is high like it should be
  if (INT0_INPUT & (1 << INT0_PIN)) {
    // it possible that bits preceeding the stop bit were
    // high (thus no transistion).  Fill those in.
    for (int i=suart.bits_collected; i<9; ++i) {
      suart.byte = (suart.byte >> 1) | 0x80;
    }

    suart.buffer[suart.buffer_tail] = suart.byte;
    suart.buffer_tail = (suart.buffer_tail + 1) & SOFTWARE_UART_BUFFER_MASK;

    if (suart.byte_received) {
      suart.byte_received();
    }
  }
}

