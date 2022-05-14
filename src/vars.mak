# Assumes that probrams are nested two deep.  Ifg this is not true,
# the caller can override this var before including this file.
ROOT ?= ../..
ROOT_LIB ?= $(ROOT)/lib

OS=$(shell uname)
AVR_PATH ?= /usr/bin
AVR_DUDE ?= /usr/bin/avrdude
AVR_DUDE_CONF ?= $(ROOT)/avrdude.conf

MCU ?= atmega328p

ifneq ($(strip $(shell lsusb | grep 1781:0c9f)),)
  PROGRAMMER ?= usbtiny
  BAUD ?=
  DISABLE_AUTO_ERASE ?=
  F_CPU ?= 16000000
  SERIAL_DEV ?=
else ifneq ($(strip $(shell lsusb | grep 16c0:05dc)),)
  PROGRAMMER ?= usbasp
  BAUD ?=
  DISABLE_AUTO_ERASE ?=
  F_CPU ?= 16000000
  SERIAL_DEV ?=
else ifeq ($(MCU),attiny85)
  PROGRAMMER ?= stk500v1
  BAUD ?= -b19200
  DISABLE_AUTO_ERASE ?=
  F_CPU ?= 1000000
  SERIAL_DEV ?= -P$(shell ls /dev/ttyUSB* | head -1)
else
  PROGRAMMER ?= arduino
  BAUD ?= -b57600
  DISABLE_AUTO_ERASE ?= -D
  F_CPU ?= 16000000
  SERIAL_DEV ?= -P$(shell ls /dev/ttyUSB* | head -1)
endif

FLAGS ?= \
  -mmcu=$(MCU) \
  -g \
  -Os \
  -fdata-sections \
  -ffunction-sections \
  -std=c11 \
  -Wall \
  -Werror \

BASE_CFLAGS ?= \
  $(FLAGS) \
  -c \
  -DF_CPU=$(F_CPU) \

LFLAGS ?= \
  $(FLAGS) \
  -Wl,--gc-sections \

INCLUDES ?= \
  -I$(ROOT_LIB)


FILES ?= \
  main.o
