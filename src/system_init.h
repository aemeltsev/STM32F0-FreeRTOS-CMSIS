#ifndef SYSTEM_INIT_H
#define SYSTEM_INIT_H

#include "stm32f0xx.h"

uint8_t clock_init_hsi_48MHz(void);
uint8_t clock_init_hsi_24MHz(void);
uint8_t clock_init_hsi_8MHz(void);
uint8_t clock_init_hse_48MHz(void);
uint8_t clock_init_hse_32MHz(void);
uint8_t clock_init_hse_8MHz(void);

// Configures all physical GPIO pins for buzzer, error and link leds, spi1, spi2, uart2(modbus, RS-485), i2c1(eeprom)
void system_gpio_init(void);

#endif // SYSTEM_INIT_H