#ifndef LEDS_H
#define LEDS_H
#include <stdint.h>
#include <stdbool.h>
#include "stm32f0xx.h"

void leds_init();

void error_state(bool state);

void link_state(bool state);

#endif // LEDS_H