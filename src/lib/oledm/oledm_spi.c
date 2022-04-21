#include <spi/spi.h>
#include <error_codes.h>
#include "oledm.h"
#include "oledm_spi.h"
#include <util/delay.h>

void oledm_ifaceInit(void) {
  CS_DDR |= (1 << CS_PIN);
  CS_PORT |= (1 << CS_PIN);

  DC_DDR |= (1 << DC_PIN);
  DC_PORT |= (1 << DC_PIN);

  RES_DDR |= (1 << RES_PIN);
  RES_PORT &= ~(1 << RES_PIN);
  _delay_ms(2);
  RES_PORT |= (1 << RES_PIN);
  spi_initMasterFreq(SPI_USE_MOSI, SPI_FREQUENCY);
}

void oledm_startCommands(error_t* err) {
  DC_PORT &= ~(1 << DC_PIN);
  CS_PORT &= ~(1 << CS_PIN);
}

#ifndef SSD1680
void oledm_start_pixels(struct OLEDM* display) {
  CS_PORT &= ~(1 << CS_PIN);
}
#endif

void oledm_command(uint8_t cmd, error_t* err) {
  spi_syncWrite(cmd);
}

void oledm_ifaceWriteData(uint8_t data, error_t* err) {
  spi_syncWrite(data);
}

void oledm_stop(struct OLEDM* display) {
  CS_PORT |= (1 << CS_PIN);
  DC_PORT |= (1 << DC_PIN);
}
