#include "stm32f0xx.h"
#include "util.h"

extern void delay_ms(uint32_t ms);

void init_button_led(void);
void button_to_led(uint8_t *button_state);
void init_buttons_for_bin_sum(void);
void init_buttons_for_sel_led(void);