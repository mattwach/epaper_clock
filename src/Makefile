# define to attiny85 or atmega328p
MCU?=atmega328p

# This is the Low-power stand alone chip configuration.
CLOCK_MODE ?= USE_32K_CRYSTAL
UART_MODE ?= HARDWARE_UART
F_CPU ?= 8000000

# This is the easy-to-build firmware that is based on an Ardino Nano
#CLOCK_MODE ?= USE_CPU_CRYSTAL
#UART_MODE ?= SOFTWARE_UART
#F_CPU ?= 16000000

# Uncomment to activate debug via the UART TX (9600 baud)
#DEBUG_CFLAG := -DDEBUG

CFLAGS := \
    -DSSD1680 \
    -DTWI_FREQ=100000 \
    -DNMEA_BUFFER_SIZE=512 \
    -D$(CLOCK_MODE) \
    -D$(UART_MODE) \
    $(DEBUG_CFLAG) \

# If you get the error, array subscript 0 is outside array bounds
# then uncomment the line below (it has to do with using GCC >= 12)
#CFLAGS += --param=min-pagesize=0

ROOT=.
include $(ROOT)/vars.mak

FILES := \
  main.o \
  buttons.o \
  clock_number_font.o \
  detail_numbers_font.o \
  display.o \
  labels_font.o \
  eeprom_vars.o \
  gps.o \
  gps_stats_font.o \
  menu.o \
  pressure_graph.o \
  pressure_font.o \
  sun_moon_icons_dark.o \
  sun_moon_icons_light.o \
  uart.o \
  $(ROOT_LIB)/data/stream_u16_to_u8.o \
  $(ROOT_LIB)/lowpower/lowpower.o \
  $(ROOT_LIB)/nmea_decoder/nmea_decoder.o \
  $(ROOT_LIB)/oledm/graph_display.o \
  $(ROOT_LIB)/oledm/ssd1680.o \
  $(ROOT_LIB)/oledm/oledm_spi.o \
  $(ROOT_LIB)/oledm/text.o \
  $(ROOT_LIB)/pstr/pstr.o \
  $(ROOT_LIB)/spi/spi.o \
  $(ROOT_LIB)/twi/twi.o \
  $(ROOT_LIB)/weather/ms8607.o \

include $(ROOT)/rules.mak

