#include "stm32f0xx.h"
#include "usart.h"
#include "util.h"
#include "button.h"

extern uint32_t SystemCoreClock;

uint8_t ClockInitHSI48MHz(void);
uint8_t ClockInitHSI24MHz(void);
uint8_t ClockInitHSI8MHz(void);
uint8_t ClockInitHSE48MHz(void);
uint8_t ClockInitHSE32MHz(void);

volatile uint32_t timing_delay_count;
void delay_ms(uint32_t ms);

int main(void)
{
    // 1. First we need configure clock
    ClockInitHSI8MHz();
    // 2. After call any ClockInit... functios need reinitialise SysTick
    SysTick_Init(SystemCoreClock);

    // 3. Initialise other peripheral
    init_button_led();
    uint8_t button_state = 0;

    while (1)
    {
        button_to_led(&button_state);
    }
}

void SysTick_Init(uint32_t ticks)
{
    // SystemCoreClock / 1000 — the number of ticks for 1 ms
    // The LOAD controller determines the period
    // If we passed 1000 as the "desired interrupt frequency in Hz"
    // then we need to divide the system frequency by this number
    if(ticks == 1000) {
        SysTick->LOAD = (SystemCoreClock / 1000) - 1;
    } else {
        // Otherwise, we consider that the finished processor frequency has been transferred
        SysTick->LOAD = (ticks / 1000) - 1;
    }
    
    // Reset current value
    SysTick->VAL = 0;
    
    // Control register settings:
    // BIT 2: CLKSOURCE = 1 (processor frequency)
    // BIT 1: TICKINT = 1 (enable interrupt)
    // BIT 0: ENABLE = 1 (start timer)
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | 
                    SysTick_CTRL_TICKINT_Msk   | 
                    SysTick_CTRL_ENABLE_Msk;
}

void delay_ms(uint32_t ms)
{
    timing_delay_count = ms;
    while (timing_delay_count != 0);
}

void SysTick_Handler(void)
{
    if (timing_delay_count != 0x00) {
        timing_delay_count--;
    }
}

// --- Variant One: Maximum speed (48 MHz) from internal oscillator ---
uint8_t ClockInitHSI48MHz(void)
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
    // 1. Enable HSI oscillator and wait for it to be ready
    uint32_t timeout = 0xFFFF;
    RCC->CR |= RCC_CR_HSION;
    while ((RCC->CR & RCC_CR_HSIRDY) == 0){
        if(timeout == 0) return 0;
        --timeout;
    }

    // 4. FLASH CONFIGURATION (Critically Important!)
    // Set Wait State to 1 and enable Prefetch
    FLASH->ACR = FLASH_ACR_LATENCY | FLASH_ACR_PRFTBE;

    // 2. Configure PLL
    // Before configure PLL have got to disable
    if (RCC->CR & RCC_CR_PLLON) {
        RCC->CR &= ~RCC_CR_PLLON;
        while (RCC->CR & RCC_CR_PLLRDY);
    }
    // Source PLL = HSI/2 (4MHz). Multiplier = 12. Result 48MHz.
    RCC->CFGR &= ~RCC_CFGR_PLLSRC; // Clear PLL source
    RCC->CFGR |= RCC_CFGR_PLLSRC_HSI_DIV2; // Set PLL source to HSI/2
    
    RCC->CFGR &= ~RCC_CFGR_PLLMUL; // Clear PLL multiplication factor
    RCC->CFGR |= RCC_CFGR_PLLMUL12; // Set PLL multiplication factor to 12

    // 3. Enable PLL
    RCC->CR |= RCC_CR_PLLON;
    timeout = 0xFFFF;
    while ((RCC->CR & RCC_CR_PLLRDY) == 0)
    {
        if(timeout == 0) return 0;
        --timeout;
    }

    // 5. Configure AHB and APB1 prescalers (AHB = 48MHz, APB = 48MHz)
    RCC->CFGR &= ~RCC_CFGR_HPRE; // Clear AHB prescaler
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1; // Set AHB prescaler to 1
    
    RCC->CFGR &= ~RCC_CFGR_PPRE; // Clear APB1 prescaler
    RCC->CFGR |= RCC_CFGR_PPRE_DIV1; // Set APB1 prescaler to 1

    // 6. Select the PLL as the system clock source and wait for it to be switched
    RCC->CFGR &= ~RCC_CFGR_SW; // Clear SW bits
    RCC->CFGR |= RCC_CFGR_SW_PLL; // Set SW to PLL
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

    // 7. Configure clocks for peripherals (after switch on PLL)
    
    // We explicitly tell USART1 to use the system frequencies (48 MHz)
    RCC->CFGR3 &= ~RCC_CFGR3_USART1SW; // Clear USART1 clock source
    RCC->CFGR3 |= RCC_CFGR3_USART1SW_SYSCLK; // Set USART1 clock source to PCLK
    
    // We explicitly tell I2C1 to use the system frequency (48 MHz)
    // This VERY IMPORTANT for timing 0x10805E89
    RCC->CFGR3 &= ~RCC_CFGR3_I2C1SW; // Clear I2C1 clock source
    RCC->CFGR3 |= RCC_CFGR3_I2C1SW_SYSCLK; // Set I2C1 clock source to HSI

    // Enable SYSCFG clock
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    /*
    // For I2C1, recommend using SYSCLK (48MHz) 
    // This will allow you to more accurately adjust the 100/400 kHz timings.
    RCC->CFGR3 = (RCC->CFGR3 & ~RCC_CFGR3_I2C1SW) | RCC_CFGR3_I2C1SW_SYSCLK;
    */

    // 8. Update SystemCoreClock (standart method CMSIS)
    SystemCoreClockUpdate(); 
    return 1; // Success
}

// --- Variand Two: Medium speed (24 MHz) from internal oscillator ---
uint8_t ClockInitHSI24MHz(void)
{
    uint32_t timeout = 0xFFFF;

    // 1. Enable HSI oscillator and wait for it to be ready
    RCC->CR |= RCC_CR_HSION;
    while (((RCC->CR & RCC_CR_HSIRDY) == 0) && --timeout);
    if (timeout == 0) return 0; // Error: HSI does not running

    // 2. Configure Flash: 0 wait state (up to 24 MHz permissible), enable Prefetch
    FLASH->ACR |= FLASH_ACR_PRFTBE;
    FLASH->ACR &= ~FLASH_ACR_LATENCY; 

    // 3. Configure PLL - Enable PLL before configure (IMPORTANT!)
    if (RCC->CR & RCC_CR_PLLON) {
        RCC->CR &= ~RCC_CR_PLLON;
        while (RCC->CR & RCC_CR_PLLRDY); // Wait for full stop
    }
    
    // 5. Configure PLL - Source PLL = HSI/2 (4MHz). Multiplier = 6. Total 24MHz.
    RCC->CFGR &= ~RCC_CFGR_PLLSRC; // Clear PLL source
    RCC->CFGR |= RCC_CFGR_PLLSRC_HSI_DIV2; // Set PLL source to HSI/2
    
    RCC->CFGR &= ~RCC_CFGR_PLLMUL; // Clear PLL multiplication factor
    RCC->CFGR |= RCC_CFGR_PLLMUL6; // Set PLL multiplication factor to 6
    
    // 6. Disable PULL and wait for stabilise
    RCC->CR |= RCC_CR_PLLON;
    timeout = 0xFFFF;
    while (!(RCC->CR & RCC_CR_PLLRDY) && --timeout);
    if (timeout == 0) return 0;

    // 5. Switch system (SW) on PLL source
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

    // 6. Update global frequency variable
    SystemCoreClockUpdate();
    return 1; // Success
}

// --- Variant Three: Economic mode (8 MHz without PLL) ---
uint8_t ClockInitHSI8MHz(void)
{
    uint32_t timeout = 0xFFFF;

    // 1. Enable HSI oscillator and wait for it to be ready
    RCC->CR |= RCC_CR_HSION;
    while (((RCC->CR & RCC_CR_HSIRDY) == 0) && --timeout);
    if (timeout == 0) return 0; // Error: HSI is not running
    
    // 2. Reset the AHB and APB1 bus prescalers to their default state (divider 1)
    // This ensures that the peripherals don't operate at the "tails" of the old settings.
    RCC->CFGR &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE);

    // 3. Configure Flash: for 8 MHz enough Latency 0
    // But it's important to reset the buffer (Prefetch), if it turn on before
    FLASH->ACR &= ~(FLASH_ACR_LATENCY | FLASH_ACR_PRFTBE);

    // 4. Switch to HSI straight
    RCC->CFGR &= ~RCC_CFGR_SW;
    while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);
    
    // 5. Disable PLL for power economy
    RCC->CR &= ~RCC_CR_PLLON; 

    // 6. Update SystemCoreClock value (CMSIS standard method)
    SystemCoreClockUpdate(); 
    return 1; // Success
}

// --- Variant Four: HSE -> PLL -> 48 MHz (Quartz for 8 MHz) ---
uint8_t ClockInitHSE48MHz(void)
{
    uint32_t timeout;

    // 1. Enable HSE and wait for it to be ready
    RCC->CR |= RCC_CR_HSEON;
    // We are waiting for a successful launch or the end of the timeout
    for (timeout = 0; ; timeout++)
    {
        // If it starts successfully, then we exit the loop.
        if (RCC->CR & RCC_CR_HSERDY){
        // Wait until HSE is stabilise
            break;
        }
        // If it doesn't start, turn off everything that turns on and return the error
        if (timeout > 0x1000){
            RCC->CR &= ~(RCC_CR_HSEON); // Stop HSE
            return 1;
        }
    }

    // 2. Flash Configuration (1 wait state for 48 MHz)
    // Configuring the flash memory access frequency
    FLASH->ACR &= ~FLASH_ACR_LATENCY; // Reset of LATENCY bits
    FLASH->ACR = FLASH_ACR_LATENCY | FLASH_ACR_PRFTBE;

    // 3. Configure PLL
    // Enable PLL before change parameters (just in case)
    RCC->CR &= ~RCC_CR_PLLON;
    while(RCC->CR & RCC_CR_PLLRDY);

    // Clear the source, prescaler (PREDIV), and multiplier bits
    RCC->CFGR &= ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLMUL);
    
    // Source of PLL = HSE (through PREDIV)
    RCC->CFGR |= RCC_CFGR_PLLSRC_HSE_PREDIV; 
    
    // We make sure that PREDIV is equal to 1 (i.e. the 8 MHz input is not taken into account)
    RCC->CFGR2 &= ~RCC_CFGR2_PREDIV;
    RCC->CFGR2 |= RCC_CFGR2_PREDIV_DIV1;

    // Multiplier x6 (8 MHz * 6 = 48 MHz)
    RCC->CFGR |= RCC_CFGR_PLLMUL6;

    // 4. Enable PLL and wait for a stabilise
    RCC->CR |= RCC_CR_PLLON;
    // Wait for a success run or end of time out
    for(timeout=0; ; timeout++)
    {
        // If it starts successfully, then we exit the loop.
        if (RCC->CR & RCC_CR_PLLRDY) {
            // Wait until PLL is stabilise
            break;
        }
        
        // If for some reason the PLL does not start, then we turn off everything that was turned on and return an error
        if(timeout > 0x1000)
        {
            RCC->CR &= ~(RCC_CR_HSEON); // Stop HSE
            RCC->CR &= ~(RCC_CR_PLLON); // Stop PLL
            return 2;
        }
    }

    // 5. Configure a bus divisor (AHB=1, APB=1)
    RCC->CFGR &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE);
    RCC->CFGR |= (RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE_DIV1);

    // 6. Switching the system to PLL
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    timeout=0;
    while (((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) && timeout < 0xFFFF) {
        timeout++;
    }

    // 7. Update SystemCoreClock value (CMSIS standard method)
    SystemCoreClockUpdate();
    
    return 0; // Success
}

// --- Variant Five: HSE -> PLL -> 32 MHz (Quartz for 8 MHz) ---
uint8_t ClockInitHSE32MHz(void)
{
    uint32_t timeout = 0xFFFF;

    // 1. Turn on HSE (required, as it is the source for PLL)
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY) && --timeout);
    if (timeout == 0) return 1;

    // 2. Flash Configuration (1 wait state for 32 MHz)
    // Configuring the flash memory access frequency
    FLASH->ACR &= ~FLASH_ACR_LATENCY; // Reset of LATENCY bits
    FLASH->ACR = FLASH_ACR_LATENCY | FLASH_ACR_PRFTBE;

    // 3. PLL setup: Source HSE, PREDIV /1, MUL x4 (8 * 4 = 32)
    // PLL must be disabled before setup
    RCC->CR &= ~RCC_CR_PLLON;
    while(RCC->CR & RCC_CR_PLLRDY);
    
    // Clean the source prescaler (PREDIV) and multiplier bits
    RCC->CFGR &= ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLMUL);

    // Source is PLL = HSE (through PREDIV)
    RCC->CFGR |= RCC_CFGR_PLLSRC_HSE_PREDIV; 
    
    // We make sure, that PREDIV is equal 1 (i.e. input 8 MHz is not divisible)
    RCC->CFGR2 &= ~RCC_CFGR2_PREDIV;
    RCC->CFGR2 |= RCC_CFGR2_PREDIV_DIV1;

    // Multiplier is x6 (8 MHz * 4 = 32 MHz)
    RCC->CFGR |= RCC_CFGR_PLLMUL4;

    // 4. Enable PLL and wait for a stabilise
    RCC->CR |= RCC_CR_PLLON;
    timeout = 0xFFFF;
    while (!(RCC->CR & RCC_CR_PLLRDY) && --timeout);
    if (timeout == 0) return 2;

    // 5. Switching the system to PLL
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

    // 6. Update SystemCoreClock value
    SystemCoreClockUpdate();
    
    return 0; // Success
}

// --- Variant Six: Straight HSE 8 MHz (Without PLL) ---
uint8_t ClockInitHSE8MHz(void)
{
    uint32_t timeout = 0xFFFF;

    // 1. Turn on HSE (required, as it is the source for PLL)
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY) && --timeout);
    if (timeout == 0) return 1;
    
    // 2. Flash settings: no delays are needed for 8 MHz (0 Wait State)
    // But it's useful to reset the settings to defaults
    FLASH->ACR &= ~(FLASH_ACR_LATENCY | FLASH_ACR_PRFTBE);
    
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_HSE;

    // 5. Waiting for switch confirmation (SWS)
    timeout = 0xFFFF;
    while(((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSE) && --timeout);
    if (timeout == 0) return 2; // Error switching
    
    // 6. Disable PLL (not need for this mode saving current)
    RCC->CR &= ~RCC_CR_PLLON;
    while (RCC->CR & RCC_CR_PLLRDY); // We wait for it to stop if it was turned on.
    
    // 7. Update SystemCoreClock value (to write 8000000 value)
    SystemCoreClockUpdate();
    
    return 0; // Success
}