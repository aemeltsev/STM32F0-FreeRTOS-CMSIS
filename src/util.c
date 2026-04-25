/**
 *******************************************
 * @file    util.c
 * @author  Anton Emeltsev
 * @version 1.0
 * @date	  5-February-2025
 * @brief   This header file part of Cortex-M0, M0+ lib
 *******************************************
 */
#include "util.h"

uint8_t bin_to_dec(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    uint8_t result = (a << 3) | (b << 2) | (c << 1) | d;
    return result;
}

void dec_to_bin(uint8_t num, uint8_t *out)
{
  if (out != NULL) {
    out[3] = num & 0x1;
    out[2] = (num >> 1) & 0x1;
    out[1] = (num >> 2) & 0x1;
    out[0] = (num >> 3) & 0x1;
  }
}

// Write a 16-bit number to a byte array (Big Endian)
void u16_to_bytes(uint16_t val, uint8_t* buf)
{
    buf[0] = (uint8_t)(val >> 8);
    buf[1] = (uint8_t)(val & 0xFF);
}

// Read a 16-bit number from an array
uint16_t bytes_to_u16(uint8_t* buf)
{
    return (uint16_t)((buf[0] << 8) | buf[1]);
}

uint8_t calculate_checksum(uint8_t* data, uint16_t len)
{
    uint8_t crc = 0xFF;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= data[i];
    }
    return crc;
}

void byte_to_hex_str(uint8_t byte, char* out_str)
{
    const char hex_chars[] = "0123456789ABCDEF";
    out_str[0] = hex_chars[(byte >> 4) & 0x0F];
    out_str[1] = hex_chars[byte & 0x0F];
    out_str[2] = '\0'; // End of string
}

// Range limitation
int32_t clamp(int32_t val, int32_t min, int32_t max)
{
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

// "Dead zone" for filtering out small noises
int32_t apply_deadzone(int32_t val, int32_t threshold)
{
    if (abs(val) < threshold) return 0;
    return val;
}

// Collects a byte from an array of states (e.g. 8 buttons)
uint8_t pack_bits(uint8_t *bits)
{
    uint8_t res = 0;
    for(int i = 0; i < 8; i++) {
        if(bits[i]) res |= (1 << i);
    }
    return res;
}

// Swap bytes (0xAABB -> 0xBBAA)
uint16_t swap_uint16(uint16_t val)
{
    return (val << 8) | (val >> 8);
}

// Expand 4 bytes
uint32_t swap_uint32(uint32_t val)
{
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0x00FF00FF);
    return (val << 16) | (val >> 16);
}

// Proportional transfer of a value from one range to another (analogous to Arduino)
long map(long x, long in_min, long in_max, long out_min, long out_max)
{
    // Equation: (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Fast filtering (exponential moving average)
uint16_t fast_avg(uint16_t new_val, uint16_t old_avg, uint8_t shift)
{
    // Equation: old_avg + (new_val - old_avg) / 2^shift
    // Allows you to simulate a low-pass filter
    return old_avg + ((int32_t)(new_val - old_avg) >> shift);
}

/*
This function takes a string of two characters (for example, "A5") 
and turns it into number 165. It takes into account both uppercase and lowercase letters.
*/
void hex_str_to_byte(const char* str, uint8_t* out)
{
  uint8_t result = 0;
  for (uint8_t i = 0; i < 2; i++)
  {
    char c = str[i];
    uint8_t value = 0;

    if (c >= '0' && c <= '9') value = c - '0';
    else if (c >= 'A' && c <= 'F') value = c - 'A' + 10;
    else if (c >= 'a' && c <= 'f') value = c - 'a' + 10;
    
    if (i == 0) result = value << 4; // high half-byte(nibble)
    else result |= value;            // low half-byte(nibble)
  }
  *out = result;
}

/*
Similar to itoa or sprintf(buf, "%u", val). 
The function writes the number to the buffer and adds a \0 line terminator at the end. 
The buffer must be at least 11 bytes (for the number 4,294,967,295 + null terminator).
*/
void u32_to_str(uint32_t val, char* buf)
{
  char temp[10];
  uint8_t i = 0;
  
  // Processing of zero
  if (val == 0) {
    *buf++ = '0';
    *buf = '\0';
    return;
  }
  
  // Parse the number from the end
  while (val > 0) {
    temp[i++] = (val % 10) + '0';
    val /= 10;
  }
  
  // Rewrite to the main buffer in the correct order
  while (i > 0) {
    *buf++ = temp[--i];
  }
  
  *buf = '\0'; // End the string
}

/*
If subtract 1 from the power of two, all the bits after the single "1" become ones
(for example, 1000 (8) becomes 0111 (7)). The & operation will give 0 in this case.
*/
uint8_t is_power_of_two(uint32_t n)
{
  // The number must be greater than 0 and pass the bit test
  return (n > 0) && ((n & (n - 1)) == 0);
}

/*
Often used when working with DMA 
or memory allocation where data must start at 
an address that is a multiple of 4, 8, or 16.
*/
uint32_t align_up(uint32_t val, uint32_t align)
{
  if (align == 0) return val;
  uint32_t remainder = val % align;
  if (remainder == 0) return val;
  return val + (align - remainder);
}

/*
If the alignment will always be 2, 4, 8, etc., 
this code is significantly faster since it doesn't use division.
*/
uint32_t align_up_fast(uint32_t val, uint32_t align)
{
    // Works only if align = 2, 4, 8, 16...
    return (val + (align - 1)) & ~(align - 1);
}