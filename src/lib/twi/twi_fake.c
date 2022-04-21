#include "twi_fake.h"

uint8_t twi_log[16384];
uint16_t twi_logidx;
uint8_t twi_err[1024];
uint16_t twi_erridx;
uint16_t twi_errcnt;
uint8_t* twi_read_data;
uint16_t twi_bytes_read;

#define LOG(v) twi_log[twi_logidx++] = (v)

static void check_err(error_t* err) {
    if (twi_erridx >= twi_errcnt) {
        return;
    }
    if (twi_err[twi_erridx]) {
        *err = twi_err[twi_erridx];
    }
    ++twi_erridx;
}

void twi_init(void) {
    LOG(TWI_INIT);
}

void twi_reinit(void) {
    LOG(TWI_REINIT);
}

void twi_stop(error_t* err) {
    if (*err) { return; } 
    LOG(TWI_STOP);
    check_err(err);
}

void twi_startWrite(uint8_t address, error_t* err) {
    if (*err) { return; } 
    LOG(TWI_START_WRITE);
    LOG(address);
    check_err(err);
}

void twi_writeNoStop(uint8_t byte, error_t* err) {
    if (*err) { return; } 
    LOG(TWI_WRITE_NO_STOP);   
    LOG(byte);
    check_err(err);
}
void twi_readNoStop(
    uint8_t address,
    uint8_t* data,
    uint8_t length,
    error_t* err) {
    if (*err) { return; } 
    LOG(TWI_READ_NO_STOP);   
    LOG(address);
    LOG(length);
    if (twi_read_data) {
      memcpy(data, twi_read_data + twi_bytes_read, length);
      twi_bytes_read += length;
    }
    check_err(err);
 }
