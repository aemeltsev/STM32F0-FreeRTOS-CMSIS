#include "util.h"

uint8_t bin_to_dec(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    uint8_t result = (a << 3) | (b << 2) | (c << 1) | d;
    return result;
}

void dec_to_bin(uint8_t num, uint8_t *out)
{
  out[3] = num & 0x1;
  out[2] = (num >> 1) & 0x1;
  out[1] = (num >> 2) & 0x1;
  out[0] = (num >> 3) & 0x1;
}