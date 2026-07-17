#ifndef TIMER_DRV_H
#define TIMER_DRV_H

#include "stm32f0xx.h"
#include <stdint.h>

/**
 * @brief  Initializes the hardware TIM3 peripheral for Modbus T3.5 timeout detection.
 * @param  baudrate: The operational speed of the RS-485 bus (e.g., 9600).
 */
void tim3_init_modbus_timeout(uint32_t baudrate);

/**
 * @brief  Hardware Interrupt Service Routine (ISR) for TIM3.
 *         Automatically overrides the weak symbol defined in the startup assembly file.
 */
void TIM3_IRQHandler(void);

#endif //TIMER_DRV_H