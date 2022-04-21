// Unit tests for uart

#include <test/unit_test.h>
#include "uart.h"

void test_init(void) {
  uart_init(9600, ENABLE_TX);

  assert_reg_equal(0x20, UCSR0A);
  assert_reg_equal(0, UBRR0H);
  assert_reg_equal(103, UBRR0L);
  assert_reg_equal(0x08, UCSR0B);
  assert_reg_equal(0x06, UCSR0C);
}

void test_init_57600(void) {
  uart_init(57600, ENABLE_TX);

  assert_reg_equal(0, UBRR0H);
  assert_reg_equal(16, UBRR0L);
  assert_reg_activity(UCSR0A);
  assert_reg_activity(UCSR0B);
  assert_reg_activity(UCSR0C);
}

void test_init_rx(void) {
  uart_init(9600, ENABLE_RX);

  assert_reg_activity(UCSR0A);
  assert_reg_activity(UBRR0H);
  assert_reg_activity(UBRR0L);
  assert_reg_equal(0x10, UCSR0B);
  assert_reg_activity(UCSR0C);
}

void test_init_tx(void) {
  uart_init(9600, ENABLE_TX);

  assert_reg_activity(UCSR0A);
  assert_reg_activity(UBRR0H);
  assert_reg_activity(UBRR0L);
  assert_reg_equal(0x08, UCSR0B);
  assert_reg_activity(UCSR0C);
}

void test_init_rxi(void) {
  uart_init(9600, ENABLE_RX_INTERRUPT);

  assert_reg_activity(UCSR0A);
  assert_reg_activity(UBRR0H);
  assert_reg_activity(UBRR0L);
  assert_reg_equal(0x80, UCSR0B);
  assert_reg_activity(UCSR0C);
}

void test_init_txi(void) {
  uart_init(9600, ENABLE_TX_INTERRUPT);

  assert_reg_activity(UCSR0A);
  assert_reg_activity(UBRR0H);
  assert_reg_activity(UBRR0L);
  assert_reg_equal(0x40, UCSR0B);
  assert_reg_activity(UCSR0C);
}

void test_init_all(void) {
  uart_init(9600, ENABLE_RX | ENABLE_TX | ENABLE_RX_INTERRUPT | ENABLE_TX_INTERRUPT);

  assert_reg_activity(UCSR0A);
  assert_reg_activity(UBRR0H);
  assert_reg_activity(UBRR0L);
  assert_reg_equal(0xD8, UCSR0B);
  assert_reg_activity(UCSR0C);
}

static void common_init() {
  uart_init(9600, 0);
  reset_history();
}

void test_disable(void) {
  uart_disable();

  assert_reg_equal(0, UCSR0B);
  assert_reg_equal(0, UBRR0H);
  assert_reg_equal(0, UBRR0L);
}

void test_read_no_block(void) {
  common_init();
  uint8_t byte = 0;
  assert_int_equal(0, read_no_block(&byte));
  assert_reg_activity(UCSR0A);
}

void test_read_no_block_with_data(void) {
  common_init();
  sequence(UCSR0A, 0x80);
  sequence(UDR0, 'A');
  uint8_t byte = 0;
  assert_int_equal(1, read_no_block(&byte));
  assert_int_equal('A', byte);
  assert_reg_activity(UCSR0A);
  assert_reg_activity(UDR0);
}

void test_byte(void) {
  common_init();

  uart_byte('X');

  assert_history(UCSR0A, 0x20);
  assert_history(UDR0, 'X');
}

void test_bytes(void) {
  const char* data = "Hello";
  common_init();

  uart_bytes((uint8_t*)data, 5);

  assert_history_buff(UDR0, data, 5);
  assert_reg_activity(UCSR0A);
}

void test_bytes_zerolen(void) {
  common_init();
  uart_bytes(0, 0);
}

void test_bytes_twice(void) {
  common_init();

  uart_bytes((uint8_t*)"Hi ", 3);
  uart_bytes((uint8_t*)"There", 5);

  assert_history_str(UDR0, "Hi There");
  assert_reg_activity(UCSR0A);
}

void test_byteslong(void) {
  uint8_t data[255];
  int i=0;
  for (; i < sizeof(data); ++i) {
    data[i] = i;
  }
  common_init();

  uart_bytes((uint8_t*)data, sizeof(data));

  assert_history_buff(UDR0, data, sizeof(data));
  assert_reg_activity(UCSR0A);
}

void test_byteslongtwice(void) {
  uint8_t data[255];
  int i=0;
  for (; i < sizeof(data); ++i) {
    data[i] = i;
  }
  common_init();

  uart_bytes((uint8_t*)data, 111);
  uart_bytes((uint8_t*)data + 111, sizeof(data) - 111);

  assert_history_buff(UDR0, data, sizeof(data));
  assert_reg_activity(UCSR0A);
}

void test_str8(void) {
  const char* data = "Hello";
  common_init();

  uart_str(data);

  assert_history_str(UDR0, "Hello");
  assert_reg_activity(UCSR0A);
}

void test_str8_empty(void) {
  const char* data = "";
  common_init();

  uart_str(data);
}

void test_pstr(void) {
  uint8_t data[] = {5, 'H', 'e', 'l', 'l', 'o'};
  common_init();

  uart_pstr(data);

  assert_history_str(UDR0, "Hello");
  assert_reg_activity(UCSR0A);
}

void test_pstr_empty(void) {
  uint8_t data[] = {0};
  common_init();

  uart_pstr(data);
}

int main(void) {
  test(test_init);
  test(test_init_57600);
  test(test_init_all);
  test(test_init_rx);
  test(test_init_tx);
  test(test_init_rxi);
  test(test_init_txi);

  test(test_disable);

  test(test_read_no_block);
  test(test_read_no_block_with_data);

  test(test_byte);

  test(test_bytes);

  test(test_bytes_zerolen);
  test(test_bytes_twice);
  test(test_byteslong);
  test(test_byteslongtwice);

  test(test_str8);
  test(test_str8_empty);

  test(test_pstr);
  test(test_pstr_empty);

  return 0;
}
