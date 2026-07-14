#include "buzzer.h"

void buzzer_state(bool state){
    if(state){
        GPIOB->BSRR = GPIO_BSRR_BS_8;
    } else {
        GPIOB->BSRR = GPIO_BSRR_BR_8;
    }
}
