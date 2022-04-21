#include <test/unit_test.h>
#include "twi.h"

void test_init(void) {
  twi_init();
  twi_init(); // This should do nothing

  // SDA and SCL set as pullups
  assert_reg_equal(0x00, DDRC);
  assert_reg_equal(0x30, PORTC);
  assert_reg_equal(0x00, TWSR);
  assert_reg_equal(8, TWBR);
}

static void common_init(void) {
  twi_reinit();
  reset_history();
}

void test_startWrite_OK(void)  {
  error_t error = 0;
  common_init();
  sequence(TWSR, 0x08);  // Indicates that TWSR start was successful
  sequence(TWSR, 0x18); // Indicates that the Write was ACKed

  twi_startWrite(0x7B, &error);

  assert_int_equal(0, error);
  assert_history(TWDR, 0xF6); // 0x7B << 1 (address write)
  // send start, send data and two wait-for-changes
  assert_history(TWCR,
    0xA4, 0xA4,  // start bit + WAIT_FOR_TWI
    0x84, 0x84   // send data (address) + WAIT_FOR_TWI
  );
}

void test_startWrite_Restart_OK(void)  {
  error_t error = 0;
  common_init();
  sequence(TWSR, 0x10);  // Indicates that TWSR restart was successful
  sequence(TWSR, 0x18); // Indicates that the Write was ACKed

  twi_startWrite(0x7B, &error);

  assert_int_equal(0, error);
  assert_reg_activity(TWDR);  // These are covered in test_startWrite_OK
  assert_reg_activity(TWCR);
}

void test_startWrite_preerror(void)  {
  error_t error = 1;
  common_init();

  twi_startWrite(0x7B, &error);

  assert_int_equal(1, error);
}

void test_startWrite_Start_Not_OK(void)  {
  error_t error = 0;
  common_init();
  // Just leave TWSR at zero.
  sequence(TWSR, 0x00);  // Invalid start write code

  twi_startWrite(0x7B, &error);

  assert_int_equal(TWI_MISSING_START_CON_ERROR, error);
  assert_history(TWCR, 0xA4, 0xA4);  // Confirm there is no attempt to write
}

void test_startWrite_NACK(void)  {
  error_t error = 0;
  common_init();
  sequence(TWSR, 0x10);  // Indicates that TWSR restart was successful
  sequence(TWSR, 0x20); // Indicates that the Write was NACKed

  twi_startWrite(0x7B, &error);

  assert_int_equal(TWI_NO_ACK_ERROR, error);
  assert_reg_activity(TWDR);  // These are covered in test_startWrite_OK
  assert_reg_activity(TWCR);
}

void test_startWrite_ArbLost(void)  {
  error_t error = 0;
  common_init();
  sequence(TWSR, 0x10);  // Indicates that TWSR restart was successful
  sequence(TWSR, 0x38); // Indicates that arbitration was lost

  twi_startWrite(0x7B, &error);

  assert_int_equal(TWI_ARB_LOST_ERROR, error);
  assert_reg_activity(TWDR);  // These are covered in test_startWrite_OK
  assert_reg_activity(TWCR);
}

void test_startWrite_Internal_Error(void)  {
  error_t error = 0;
  common_init();
  sequence(TWSR, 0x10);  // Indicates that TWSR restart was successful
  sequence(TWSR, 0x00); // Unexpected response

  twi_startWrite(0x7B, &error);

  assert_int_equal(TWI_INTERNAL_ERROR, error);
  assert_reg_activity(TWDR);  // These are covered in test_startWrite_OK
  assert_reg_activity(TWCR);
}

void test_writeNoStop_OK(void)  {
  error_t error = 0;
  common_init();
  sequence(TWSR, 0x28); // data ack

  twi_writeNoStop(0xAB, &error);

  assert_int_equal(0, error);
  assert_history(TWCR, 0x84, 0x84);  // Set byte and wait for send
  assert_history(TWDR, 0xAB);
}

void test_writeNoStop_preerror(void)  {
  error_t error = 1;
  common_init();

  twi_writeNoStop(0xFF, &error);
  
  assert_int_equal(1, error);
}

void test_writeNoStop_NACK(void)  {
  error_t error = 0;
  common_init();
  sequence(TWSR, 0x30); // data NACK

  twi_writeNoStop(0xAB, &error);

  assert_int_equal(TWI_NO_ACK_ERROR, error);
  assert_reg_activity(TWCR);  // already checked in test_writeNoStop_OK
  assert_reg_activity(TWDR);
}

void test_writeNoStop_Internal_Error(void)  {
  error_t error = 0;
  common_init();
  sequence(TWSR, 0x00); // unexpected response

  twi_writeNoStop(0xAB, &error);

  assert_int_equal(TWI_INTERNAL_ERROR, error);
  assert_reg_activity(TWCR);  // already checked in test_writeNoStop_OK
  assert_reg_activity(TWDR);
}

void test_readNoStop_OK(void)  {
  error_t error = 0;
  uint8_t buffer[5];
  common_init();
  sequence(TWSR, 0x08);  // Indicates that TWSR start was successful
  sequence(TWSR, 0x40); // slar ack
  sequence(TWSR, 0x50); // 'H' Ack
  sequence(TWSR, 0x50); // 'e' Ack
  sequence(TWSR, 0x50); // 'l' Ack
  sequence(TWSR, 0x50); // 'l' Ack
  sequence(TWSR, 0x58); // 'o' NACK
  sequence(TWDR, DONT_CARE); // Padding for the address write
  sequence(TWDR, 0x48); // H
  sequence(TWDR, 0x65); // e
  sequence(TWDR, 0x6C); // l
  sequence(TWDR, 0x6C); // l
  sequence(TWDR, 0x6F); // o

  twi_readNoStop(0x7B, buffer, 5, &error);

  assert_int_equal(0, error);
  assert_buff_equal((uint8_t*)"Hello", buffer, 5);
  assert_history(TWCR,
    0xA4, 0xA4,  // Start condition + WAIT_FOR_TWI
    0x84, 0x84,  // Send data (address) + WAIT_FOR_TWI
    0xC4, 0xC4,  // ACK 'H' + WAIT_FOR_TWI
    0xC4, 0xC4,  // ACK 'e' + WAIT_FOR_TWI
    0xC4, 0xC4,  // ACK 'l' + WAIT_FOR_TWI
    0xC4, 0xC4,  // ACK 'l' + WAIT_FOR_TWI
    0x84, 0x84,  // Nack 'o' + WAIT_FOR_TWI
  );  // Set byte and wait for send

  assert_history(TWDR,
    0xF7, // 0x7B << 1 | 1 (address read)
    0x48, 0x65, 0x6C, 0x6C, 0x6F  // Hello
  );
}

void test_readNoStop_preerror(void)  {
  error_t error = 1;
  common_init();

  twi_readNoStop(0x7B, 0, 1, &error);
  
  assert_int_equal(1, error);
}

void test_readNoStop_Start_Not_OK(void)  {
  error_t error = 0;
  common_init();
  sequence(TWSR, 0x00);  // Invalid start code

  twi_readNoStop(0x7B, 0, 1, &error);

  assert_int_equal(TWI_MISSING_START_CON_ERROR, error);
  assert_history(TWCR, 0xA4, 0xA4);  // Confirm there is no attempt to write
}

void test_readNoStop_NACK(void)  {
  error_t error = 0;
  common_init();
  sequence(TWSR, 0x08);  // Indicates that TWSR start was successful
  sequence(TWSR, 0x48); // SLAR NACK

  twi_readNoStop(0x7B, 0, 1, &error);

  assert_int_equal(TWI_NO_ACK_ERROR, error);
  assert_reg_activity(TWCR);  // already checked in test_readNoStop_OK
  assert_reg_activity(TWDR);
}

void test_readNoStop_Internal_Error(void)  {
  error_t error = 0;
  common_init();
  sequence(TWSR, 0x08);  // Indicates that TWSR start was successful
  sequence(TWSR, 0x00); // unexpected response

  twi_readNoStop(0x7B, 0, 1, &error);

  assert_int_equal(TWI_INTERNAL_ERROR, error);
  assert_reg_activity(TWCR);  // already checked in test_readNoStop_OK
  assert_reg_activity(TWDR);
}

void test_stop(void) {
  common_init();
  error_t err = 0;

  twi_stop(&err);

  assert_int_equal(0, err);
  assert_history(TWCR, 0x94);
}

void test_stop_preerror(void) {
  common_init();
  error_t err = 1;

  twi_stop(&err);

  assert_int_equal(1, err);
}

int main(void) {
  test(test_init);

  test(test_startWrite_OK);
  test(test_startWrite_preerror);
  test(test_startWrite_Restart_OK);
  test(test_startWrite_Start_Not_OK);
  test(test_startWrite_NACK);
  test(test_startWrite_ArbLost);
  test(test_startWrite_Internal_Error);

  test(test_writeNoStop_OK);
  test(test_writeNoStop_preerror);
  test(test_writeNoStop_NACK);
  test(test_writeNoStop_Internal_Error);

  test(test_readNoStop_OK);
  test(test_readNoStop_preerror);
  test(test_readNoStop_Start_Not_OK);
  test(test_readNoStop_NACK);
  test(test_readNoStop_Internal_Error);

  test(test_stop);
  test(test_stop_preerror);

  return 0;
}


