#if defined(HARDWARE_UART)
#include <uart/uart.c>
#elif defined(SOFTWARE_UART)
#include <uart/software_uart.c>

#ifdef DEBUG
#include <uart/uart.c>
#endif

#else
#error Please define HARDWARE_UART or SOFTWARE_UART
#endif
