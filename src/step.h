#ifndef STEP_H
#define STEP_H

#include <stdint.h>
#include "stm32f0xx.h"

void stepper_init(void);
void stepper_enable(uint8_t state);
void stepper_set_dir(uint8_t dir);
void stepper_start(uint16_t period);
void stepper_stop(void);
void stepper_update_speed(uint16_t period);

#endif //STEP_H