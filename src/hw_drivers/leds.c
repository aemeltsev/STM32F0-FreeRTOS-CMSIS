#include "leds.h"

void leds_init()
{
    //TODO
}

void error_state(bool state)
{
    if (state){
        GPIOC->BSRR = GPIO_BSRR_BS_13;
    } else{
        GPIOC->BSRR = GPIO_BSRR_BR_13;
    }
}

void link_state(bool state)
{
    if (state){
        GPIOB->BSRR = GPIO_BSRR_BS_9;
    } else{
        GPIOB->BSRR = GPIO_BSRR_BR_9;
    }
}
