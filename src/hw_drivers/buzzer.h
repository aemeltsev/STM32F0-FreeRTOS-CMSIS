#ifndef BUZZER_H
#define BUZZER_H
#include <stdint.h>
#include <stdbool.h>
#include "stm32f0xx.h"

// Pin initialise (call it from system_init.c or main.c)
void buzzer_init(void);

// Buzzer state control (true - on, false - off)
void buzzer_state(bool state);

#endif // BUZZER_H