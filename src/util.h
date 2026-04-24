/**
 *******************************************
 * @file    util.h
 * @author  Anton Emeltsev
 * @version 1.0
 * @date	5-February-2025
 * @brief   This header file part of Cortex-M0, M0+ lib
 *******************************************
 */
#include <stdint.h>
#include <stdlib.h>

uint8_t bin_to_dec(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
void dec_to_bin(uint8_t num, uint8_t *out);
void u16_to_bytes(uint16_t val, uint8_t* buf);
uint16_t bytes_to_u16(uint8_t* buf);
uint8_t calculate_checksum(uint8_t* data, uint16_t len);
void byte_to_hex_str(uint8_t byte, char* out_str);
int32_t clamp(int32_t val, int32_t min, int32_t max);
int32_t apply_deadzone(int32_t val, int32_t threshold);
uint8_t pack_bits(uint8_t *bits);

uint16_t swap_uint16(uint16_t val);
uint32_t swap_uint32(uint32_t val);

long map(long x, long in_min, long in_max, long out_min, long out_max);
uint16_t fast_avg(uint16_t new_val, uint16_t old_avg, uint8_t shift);