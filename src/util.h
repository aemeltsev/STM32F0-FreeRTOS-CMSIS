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