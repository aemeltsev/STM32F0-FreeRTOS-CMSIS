#include "stm32f0xx.h"
//#include "usart.h"
//#include "util.h"
#include "button.h"

static __IO uint32_t timing_delay_count;

void delay_ms(uint32_t us);
void timing_delay_decrem(void);

int main(void)
{
    SysTick_Config(8000);
    init_button_led();
    uint8_t button_state = 0;

    while (1)
    {
        button_to_led(&button_state);
    }
}

void ClockInitHSI(void)
{
    /*
    HSI - it's already enabled after reset.
    Set the PLL parameters in RCC->CFGR
    enable PLL in RCC->CR
    wait for PLL stabiized
    set Flash wait states
    Switch to PLL clock
    https://community.st.com/t5/stm32-mcus-products/configure-system-clock-using-registers-for-stm32f030k6t6/td-p/132791
    */
    // Enable HSI oscillator and wait for it to be ready
    RCC->CR |= RCC_CR_HSION;
    while ((RCC->CR & RCC_CR_HSIRDY) == 0);

    // Configure PLL
    RCC->CFGR &= ~RCC_CFGR_PLLSRC; // Clear PLL source
    RCC->CFGR |= RCC_CFGR_PLLSRC_HSI_DIV2; // Set PLL source to HSI/2
    
    RCC->CFGR &= ~RCC_CFGR_PLLMUL; // Clear PLL multiplication factor
    RCC->CFGR |= RCC_CFGR_PLLMUL12; // Set PLL multiplication factor to 12

    // Enable PLL
    RCC->CR |= RCC_CR_PLLON;
    while ((RCC->CR & RCC_CR_PLLRDY) == 0);

    // Configure AHB and APB1 prescalers
    RCC->CFGR &= ~RCC_CFGR_HPRE; // Clear AHB prescaler
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1; // Set AHB prescaler to 1
    
    RCC->CFGR &= ~RCC_CFGR_PPRE; // Clear APB1 prescaler
    RCC->CFGR |= RCC_CFGR_PPRE_DIV1; // Set APB1 prescaler to 1

    // Configure USART1 and I2C1 clocks
    RCC->CFGR3 &= ~RCC_CFGR3_USART1SW; // Clear USART1 clock source
    RCC->CFGR3 |= RCC_CFGR3_USART1SW_PCLK; // Set USART1 clock source to PCLK
    
    RCC->CFGR3 &= ~RCC_CFGR3_I2C1SW; // Clear I2C1 clock source
    RCC->CFGR3 |= RCC_CFGR3_I2C1SW_HSI; // Set I2C1 clock source to HSI

    // Select the PLL as the system clock source and wait for it to be switched
    RCC->CFGR &= ~RCC_CFGR_SW; // Clear SW bits
    RCC->CFGR |= RCC_CFGR_SW_PLL; // Set SW to PLL
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

    // Enable SYSCFG clock
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
}

void delay_ms(uint32_t ms)
{
    timing_delay_count = ms;
    while (timing_delay_count != 0);
}

void timing_delay_decrem(void)
{
    if (timing_delay_count != 0x00)
    {
        timing_delay_count--;
    }
}

void SysTick_Handler(void)
{
    timing_delay_decrem();
}