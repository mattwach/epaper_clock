#include "spi.h"

#include <test/unit_test.h>

void test_init_mosi(void) {
    spi_initMasterFreq(SPI_USE_MOSI, 1000000);
    assert_reg_equal(0x51, SPCR);  // SPE | MSTR | SPR0 (1Mhz)
    assert_reg_equal(0x28, DDRB);  // SCK | USE_MOSI
    assert_reg_equal(0, SPSR);  // 1Mhz 
}

void test_init_miso(void) {
    sequence(DDRB, 0x10);  // Set MISO to make sure it's cleared
    spi_initMasterFreq(SPI_USE_MISO, 1000000);
    assert_reg_equal(0x51, SPCR);  // SPE | MSTR | SPR0 (1Mhz)
    assert_reg_equal(0x20, DDRB);  // SCK | USE_MISO(LOW)
    assert_reg_equal(0, SPSR);  // 1Mhz 
}

void test_init_invert(void) {
    spi_initMasterFreq(SPI_INVERT_CLOCK, 1000000);
    assert_reg_equal(0x59, SPCR);  // SPE | MSTR | SPR0 (1Mhz) | CPOL
    assert_reg_equal(0x20, DDRB);  // SCK | USE_MISO(LOW)
    assert_reg_equal(0, SPSR);  // 1Mhz 
}

void test_init_phase(void) {
    spi_initMasterFreq(SPI_PHASE, 1000000);
    assert_reg_equal(0x55, SPCR);  // SPE | MSTR | SPR0 (1Mhz) | CPHA
    assert_reg_equal(0x20, DDRB);  // SCK | USE_MISO(LOW)
    assert_reg_equal(0, SPSR);  // 1Mhz 
}

void test_init_lsb(void) {
    spi_initMasterFreq(SPI_LSB, 1000000);
    assert_reg_equal(0x71, SPCR);  // SPE | MSTR | DORD | SPR0 (1Mhz)
    assert_reg_equal(0x20, DDRB);  // SCK | USE_MISO(LOW)
    assert_reg_equal(0, SPSR);  // 1Mhz 
}

void test_init_freqs(void) {
    spi_initMasterFreq(0, 8000001);  // / 2
    assert_reg_equal(0x01, SPSR);
    assert_reg_equal(0x50, SPCR);

    spi_initMasterFreq(0, 4500000);  // / 4
    assert_reg_equal(0x00, SPSR);
    assert_reg_equal(0x50, SPCR);

    spi_initMasterFreq(0, 2900000);  // / 8
    assert_reg_equal(0x01, SPSR);
    assert_reg_equal(0x51, SPCR);

    spi_initMasterFreq(0, 1900000);  // / 16
    assert_reg_equal(0x00, SPSR);
    assert_reg_equal(0x51, SPCR);

    spi_initMasterFreq(0, 520000);  // / 32
    assert_reg_equal(0x01, SPSR);
    assert_reg_equal(0x52, SPCR);

    spi_initMasterFreq(0, 260000);  // / 64
    assert_reg_equal(0x00, SPSR);
    assert_reg_equal(0x52, SPCR);

    spi_initMasterFreq(0, 0);  // / 128
    assert_reg_equal(0x00, SPSR);
    assert_reg_equal(0x53, SPCR);

    assert_reg_activity(DDRB); // Not tested here
}

void test_syncWrite(void) {
    sequence(SPSR, 0x80);
    spi_syncWrite(0xAB);
    assert_reg_equal(0xAB, SPDR);
    assert_history(SPSR, 0x80);
}

void test_syncRead(void) {
    sequence(SPSR, 0x80);
    sequence(SPDR, 0xAB);
    const uint8_t val = spi_syncRead();
    assert_int_equal(0xAB, val);
    assert_reg_activity(SPDR);
    assert_history(SPSR, 0x80);
}

void test_syncTransact(void) {
    sequence(SPSR, 0x80);
    sequence(SPDR, 0x00);  // the write
    sequence(SPDR, 0xBC);  // read back
    const uint8_t val = spi_syncTransact(0xAB);
    assert_int_equal(0xBC, val);
    assert_history(SPDR, 0xAB, 0xBC);
    assert_history(SPSR, 0x80);
}

int main(void) {
    test(test_init_mosi);
    test(test_init_miso);
    test(test_init_invert);
    test(test_init_phase);
    test(test_init_lsb);
    test(test_init_freqs);

    test(test_syncWrite);
    test(test_syncRead);
    test(test_syncTransact);
    return 0;
}
