// validate crc4 mutations

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <inttypes.h>

static void dump_values(uint16_t* n_prom) {
  for (uint8_t i=0; i < 8; ++i) {
    printf("n_prom[%d] = 0x%04X\n", i, n_prom[i]);
  }
  printf("\n");
}

unsigned char crc4_PT(uint16_t n_prom[]) // n_prom defined as 8x unsigned int (n_prom[8])
  {
  int cnt; // simple counter
  unsigned int n_rem=0; // crc remainder
  unsigned char n_bit;
  n_prom[0]=((n_prom[0]) & 0x0FFF); // CRC byte is replaced by 0
  n_prom[7]=0; // Subsidiary value, set to 0
  for (cnt = 0; cnt < 16; cnt++) // operation is performed on bytes
   { // choose LSB or MSB
   if (cnt%2==1) n_rem ^= (unsigned short) ((n_prom[cnt>>1]) & 0x00FF);
   else n_rem ^= (unsigned short) (n_prom[cnt>>1]>>8);
   for (n_bit = 8; n_bit > 0; n_bit--)
   {
   if (n_rem & (0x8000)) n_rem = (n_rem << 1) ^ 0x3000;
   else n_rem = (n_rem << 1);
   }
   }
  n_rem= ((n_rem >> 12) & 0x000F); // final 4-bit remainder is CRC code
  return (n_rem ^ 0x00);
  }

uint8_t crc4_PT_mod(uint16_t n_prom[]) {
  uint16_t n_rem=0; // crc remainder
  uint8_t n_bit;
  n_prom[0]=((n_prom[0]) & 0x0FFF); // CRC byte is replaced by 0
  n_prom[7]=0; // Subsidiary value, set to 0
  for (uint8_t cnt = 0; cnt < 16; cnt++) { // choose LSB or MSB
    if (cnt & 1) {
      n_rem ^= (n_prom[cnt>>1]) & 0x00FF;
    } else {
      n_rem ^= (n_prom[cnt>>1]>>8);
    }
    for (n_bit = 8; n_bit > 0; n_bit--) {
      if (n_rem & (0x8000)) {
        n_rem = (n_rem << 1) ^ 0x3000;
      }
      else {
        n_rem = (n_rem << 1);
      }
    }
  }
  n_rem= ((n_rem >> 12) & 0x000F); // final 4-bit remainder is CRC code
  return (uint8_t)n_rem;
}

int main(void) {
  srand(time(0));

  uint16_t n_prom[8];
  for (uint8_t i=0; i<8; ++i) {
    n_prom[i] = (uint16_t)rand();
  }
  dump_values(n_prom);
  printf("crc4_PT_mod: %X\n", crc4_PT_mod(n_prom)); 
  printf("crc4_PT: %X\n", crc4_PT(n_prom)); 
  return 0;
}
