#ifndef LowPower_h
#define LowPower_h

enum period_t
{
	SLEEP_15MS,
	SLEEP_30MS,
	SLEEP_60MS,
	SLEEP_120MS,
	SLEEP_250MS,
	SLEEP_500MS,
	SLEEP_1S,
	SLEEP_2S,
	SLEEP_4S,
	SLEEP_8S,
	SLEEP_FOREVER
};

enum bod_t
{
	BOD_OFF,
	BOD_ON
};

enum adc_t
{
	ADC_OFF,
	ADC_ON
};

enum timer5_t
{
	TIMER5_OFF,
	TIMER5_ON
};

enum timer4_t
{
	TIMER4_OFF,
	TIMER4_ON
};

enum timer3_t
{
	TIMER3_OFF,
	TIMER3_ON
};

enum timer2_t
{
	TIMER2_OFF,
	TIMER2_ON
};

enum timer1_t
{
	TIMER1_OFF,
	TIMER1_ON
};

enum timer0_t
{
	TIMER0_OFF,
	TIMER0_ON
};

enum spi_t
{
	SPI_OFF,
	SPI_ON
};

enum usart0_t
{
	USART0_OFF,
	USART0_ON
};

enum usart1_t
{
	USART1_OFF,
	USART1_ON
};

enum usart2_t
{
	USART2_OFF,
	USART2_ON
};

enum usart3_t
{
	USART3_OFF,
	USART3_ON
};

enum twi_t
{
	TWI_OFF,
	TWI_ON
};

enum usb_t
{
	USB_OFF,
	USB_ON
};

#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega168__) || defined (__AVR_ATmega1284P__)
	void	lowpower_idle(enum period_t period, enum adc_t adc, enum timer2_t timer2,
							enum timer1_t timer1, enum timer0_t timer0, enum spi_t spi,
										enum usart0_t usart0, enum twi_t twi);
#elif defined __AVR_ATmega2560__
	void	lowpower_idle(enum period_t period, enum adc_t adc, enum timer5_t timer5,
							enum timer4_t timer4, enum timer3_t timer3, enum timer2_t timer2,
									enum timer1_t timer1, enum timer0_t timer0, enum spi_t spi,
										enum usart3_t usart3, enum usart2_t usart2, enum usart1_t usart1,
							enum usart0_t usart0, enum twi_t twi);
#elif defined __AVR_ATmega32U4__
	void	lowpower_idle(enum period_t period, enum adc_t adc, enum timer4_t timer4, enum timer3_t timer3,
							enum timer1_t timer1, enum timer0_t timer0, enum spi_t spi,
										enum usart1_t usart1, enum twi_t twi, enum usb_t usb);
#elif defined __AVR_ATtiny85__
	void	lowpower_idle(enum period_t period, enum adc_t adc,
							enum timer1_t timer1, enum timer0_t timer0);
#else
	#error "Please ensure chosen MCU is either 168, 328P, 32U4, 1284P, 1280, 2560 or tiny85."
#endif
void	lowpower_adcNoiseReduction(enum period_t period, enum adc_t adc, enum timer2_t timer2);
void	lowpower_powerDown(enum period_t period, enum adc_t adc, enum bod_t bod);
void	lowpower_powerSave(enum period_t period, enum adc_t adc, enum bod_t bod, enum timer2_t timer2);
void	lowpower_powerStandby(enum period_t period, enum adc_t adc, enum bod_t bod);
#if !defined __AVR_ATtiny85__
	void	lowpower_powerExtStandby(enum period_t period, enum adc_t adc, enum bod_t bod, enum timer2_t timer2);
#endif

#endif
