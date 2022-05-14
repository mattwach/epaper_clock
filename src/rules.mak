a.hex: clean a.out
		avr-objcopy -j .text -j .data -O ihex a.out a.hex
  
a.S: a.out
		avr-objdump -h -S a.out > $@

a.out: $(FILES)
		$(AVR_GCC) $(LFLAGS) $(FILES) 

%.o: %.c
		$(AVR_GCC) $(BASE_CFLAGS) $(CFLAGS) $(INCLUDES) -o $@ $?

%.o: %.S
		$(AVR_GCC) $(BASE_CFLAGS) $(CFLAGS) $(INCLUDES) -o $@ $?

upload: a.hex
		$(AVR_DUDE) \
				-C$(AVR_DUDE_CONF) \
				-v \
				-p$(MCU) \
				-c$(PROGRAMMER) \
				$(SERIAL_DEV) \
				$(BAUD) \
				$(DISABLE_AUTO_ERASE) \
				-Uflash:w:a.hex:i 

ifeq ($(MCU),atmega328p)
internal_1mhz:
		$(AVR_DUDE) \
				-p$(MCU) \
				-c$(PROGRAMMER) \
				$(SERIAL_DEV) \
				$(BAUD) \
				-Ulfuse:w:0x62:m

internal_8mhz:
		$(AVR_DUDE) \
				-p$(MCU) \
				-c$(PROGRAMMER) \
				$(SERIAL_DEV) \
				$(BAUD) \
				-Ulfuse:w:0xE2:m

external_16mhz:
		$(AVR_DUDE) \
				-p$(MCU) \
				-c$(PROGRAMMER) \
				$(SERIAL_DEV) \
				$(BAUD) \
				-Ulfuse:w:0xFF:m

disable_bod:
		$(AVR_DUDE) \
				-p$(MCU) \
				-c$(PROGRAMMER) \
				$(SERIAL_DEV) \
				$(BAUD) \
				-Uefuse:w:0xFF:m

enable_bod:
		$(AVR_DUDE) \
				-p$(MCU) \
				-c$(PROGRAMMER) \
				$(SERIAL_DEV) \
				$(BAUD) \
				-Uefuse:w:0xFD:m

endif

clean:
		rm -f a.S a.out a.hex $(FILES)

asm:
		$(AVR_GCC) -S $(CFLAGS) main.c
