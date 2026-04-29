#include "step.h"

#ifdef USE_STEPPER_INTERRUPT
volatile uint32_t StepsToMove = 0;
volatile uint8_t  IsMoving = 0;
#endif

void stepper_init(void)
{
    // 1. Turn on clock for GPIOA and TIM1
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

    // 2. Configure PA8 (DIR) and PA10 (EN) as an output
    GPIOA->MODER &= ~(GPIO_MODER_MODER8 | GPIO_MODER_MODER10);
    GPIOA->MODER |= (GPIO_MODER_MODER8_0 | GPIO_MODER_MODER10_0);

    // 3. Configure PA9 (STEP) as an alternate function (TIM1_CH2)
    GPIOA->MODER &= ~GPIO_MODER_MODER9;
    GPIOA->MODER |= GPIO_MODER_MODER9_1; // AF mode
    GPIOA->AFR[1] |= (2 << GPIO_AFRH_AFSEL9_Pos); // AF2 for TIM1 on PA9

    // 4. Configure TIM1 timer
    TIM1->PSC = 48 - 1;       // The frequency of timer 1 MHz (for system frequency 48 MHz)
    TIM1->ARR = 1000 - 1;     // The frequency of steps is 1 kHz (1000 mcs for the period)
    TIM1->CCR2 = 500;         // Pulse duration 50% (500 µs)
    
    // Setting PWM mode 1 on channel 2 + Preload
    TIM1->CCMR1 |= (TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2PE);
    TIM1->CCER |= TIM_CCER_CC2E; // Enable channel 2 output
    TIM1->CR1 |= TIM_CR1_ARPE;   // ARR buffering
    
    #ifdef USE_STEPPER_INTERRUPT
    // Enable Update Interrupt (UIE)
    TIM1->DIER |= TIM_DIER_UIE;

    // Setting NVIC
    NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
    NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn, 0); 
    #endif

    // TIM1 Specific: Main Output Enable (BDTR)
    TIM1->BDTR |= TIM_BDTR_MOE;
}

void stepper_enable(uint8_t state)
{
    if (state) 
        GPIOA->BSRR = GPIO_BSRR_BR_10; // LOW = Driver is active
    else 
        GPIOA->BSRR = GPIO_BSRR_BS_10; // HIGH = Turn off
}

void stepper_set_dir(uint8_t dir)
{
    if (dir) 
        GPIOA->BSRR = GPIO_BSRR_BS_8;  // Clockwise
    else 
        GPIOA->BSRR = GPIO_BSRR_BR_8;  // Counterclockwise
}

void stepper_start(uint16_t period)
{
    if (period < 2) period = 2; // Protection against overfrequency
    TIM1->ARR = period - 1;     // It is more correct to subtract 1 according to the STM32 specification
    TIM1->CCR2 = period / 2; 
    TIM1->CNT = 0;
    TIM1->CR1 |= TIM_CR1_CEN;
}

void stepper_stop(void)
{
    TIM1->CR1 &= ~TIM_CR1_CEN; // Stop
}

void stepper_update_speed(uint16_t period)
{
    TIM1->ARR = period - 1;
    TIM1->CCR2 = period / 2;
}

#ifdef USE_STEPPER_INTERRUPT

void TIM1_BRK_UP_TRG_COM_IRQHandler(void)
{
    // We check that the interrupt was caused by the Update event
    if (TIM1->SR & TIM_SR_UIF) {
        TIM1->SR &= ~TIM_SR_UIF; // Clearing the interrupt flag

        if (StepsToMove > 0) {
            StepsToMove--;
        } else {
            // Steps are finished - stop
            TIM1->CR1 &= ~TIM_CR1_CEN; 
            IsMoving = 0;
        }
    }
}

void Stepper_Move(uint32_t steps, uint16_t period)
{
    if (steps == 0) return;

    StepsToMove = steps - 1; // -1, since the current cycle has already begun
    IsMoving = 1;

    TIM1->ARR = period - 1;
    TIM1->CCR2 = period / 2;
    TIM1->CNT = 0;
    TIM1->CR1 |= TIM_CR1_CEN; // Running
}
#endif