#ifndef BUTTON_H
#define BUTTON_H

/**
 *******************************************
 * @file    button.h
 * @author  Anton Emeltsev
 * @version 1.0
 * @date	4-February-2025
 * @brief   This header file part of Cortex-M0, M0+ lib
 *******************************************
 */
#include "stm32f0xx.h"
#include "util.h"

extern void delay_ms(uint32_t ms);

void init_button_led(void);
void button_to_led(uint8_t *button_state);
void init_buttons_for_bin_sum(void);
void init_buttons_for_sel_led(void);

#endif