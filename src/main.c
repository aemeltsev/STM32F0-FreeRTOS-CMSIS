#include "stm32f0xx.h"
#include "usart.h"
#include "util.h"
#include "button.h"

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
    // SystemCoreClock / 1000 — количество тиков для 1 мс
    // Регулятор LOAD определяет период
    SysTick->LOAD = (ticks  / 1000) - 1;
    
    // Сброс текущего значения
    SysTick->VAL = 0;
    
    // Настройка управляющего регистра:
    // BIT 2: CLKSOURCE = 1 (процессорная частота)
    // BIT 1: TICKINT = 1 (разрешить прерывание)
    // BIT 0: ENABLE = 1 (запустить таймер)
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

// --- Вариант 1: Максимальная скорость (48 МГц) ---
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
    // Перед настройкой PLL должен быть выключен
    if (RCC->CR & RCC_CR_PLLON) {
        RCC->CR &= ~RCC_CR_PLLON;
        while (RCC->CR & RCC_CR_PLLRDY);
    }
    // Источник PLL = HSI/2 (4МГц). Множитель = 12. Итого 48МГц.
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

    // 7. Конфигурация тактирования периферии (после переключения на PLL)
    
    // Явно указываем USART1 использовать системную частоту (48МГц)
    RCC->CFGR3 &= ~RCC_CFGR3_USART1SW; // Clear USART1 clock source
    RCC->CFGR3 |= RCC_CFGR3_USART1SW_SYSCLK; // Set USART1 clock source to PCLK
    
    // Явно указываем I2C1 использовать системную частоту (48МГц)
    // Это ОБЯЗАТЕЛЬНО для тайминга 0x10805E89
    RCC->CFGR3 &= ~RCC_CFGR3_I2C1SW; // Clear I2C1 clock source
    RCC->CFGR3 |= RCC_CFGR3_I2C1SW_SYSCLK; // Set I2C1 clock source to HSI

    // Enable SYSCFG clock
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    /*
    // Для I2C1 крайне рекомендую использовать SYSCLK (48MHz) 
    // Это позволит точнее настроить тайминги 100/400 кГц
    RCC->CFGR3 = (RCC->CFGR3 & ~RCC_CFGR3_I2C1SW) | RCC_CFGR3_I2C1SW_SYSCLK;
    */

    // 8. Обновляем SystemCoreClock (стандартный способ CMSIS)
    SystemCoreClockUpdate(); 
    return 1; // Успех
}

// --- Вариант 2: Средняя скорость (24 МГц) ---
uint8_t ClockInitHSI24MHz(void)
{
    uint32_t timeout = 0xFFFF;

    // 1. Enable HSI oscillator and wait for it to be ready
    RCC->CR |= RCC_CR_HSION;
    while (((RCC->CR & RCC_CR_HSIRDY) == 0) && --timeout);
    if (timeout == 0) return 0; // Ошибка: HSI не запустился

    // 2. Настройка Flash: 0 wait state (допустимо до 24МГц), включаем Prefetch
    FLASH->ACR |= FLASH_ACR_PRFTBE;
    FLASH->ACR &= ~FLASH_ACR_LATENCY; 

    // 3. Настройка PLL - Выключаем PLL перед настройкой (обязательно!)
    if (RCC->CR & RCC_CR_PLLON) {
        RCC->CR &= ~RCC_CR_PLLON;
        while (RCC->CR & RCC_CR_PLLRDY); // Ждем полной остановки
    }
    
    // 5. Configure PLL - Источник PLL = HSI/2 (4МГц). Множитель = 6. Итого 24МГц.
    RCC->CFGR &= ~RCC_CFGR_PLLSRC; // Clear PLL source
    RCC->CFGR |= RCC_CFGR_PLLSRC_HSI_DIV2; // Set PLL source to HSI/2
    
    RCC->CFGR &= ~RCC_CFGR_PLLMUL; // Clear PLL multiplication factor
    RCC->CFGR |= RCC_CFGR_PLLMUL6; // Set PLL multiplication factor to 6
    
    // 6. Включаем PLL и ждем стабилизации
    RCC->CR |= RCC_CR_PLLON;
    timeout = 0xFFFF;
    while (!(RCC->CR & RCC_CR_PLLRDY) && --timeout);
    if (timeout == 0) return 0;

    // 5. Переключаем систему (SW) на источник PLL
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

    // 6. Обновляем глобальную переменную частоты
    SystemCoreClockUpdate();
    return 1; // Успех
}

// --- Вариант 3: Экономичный режим (8 МГц без PLL) ---
uint8_t ClockInitHSI8MHz(void)
{
    uint32_t timeout = 0xFFFF;

    // 1. Enable HSI oscillator and wait for it to be ready
    RCC->CR |= RCC_CR_HSION;
    while (((RCC->CR & RCC_CR_HSIRDY) == 0) && --timeout);
    if (timeout == 0) return 0; // Ошибка: HSI не запустился

    // 2. Сброс прескалеров шин AHB, APB1 в дефолтное состояние (делитель 1)
    // Это гарантирует, что периферия не работает на "хвостах" старых настроек
    RCC->CFGR &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE);

    // 3. Настройка Flash: для 8 МГц достаточно Latency 0
    // Но важно сбросить буфер претча (Prefetch), если он был включен ранее
    FLASH->ACR &= ~(FLASH_ACR_LATENCY | FLASH_ACR_PRFTBE);

    // 4. Переключение на HSI напрямую
    RCC->CFGR &= ~RCC_CFGR_SW;
    while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);
    
    // 5. Выключаем PLL для экономии энергии
    RCC->CR &= ~RCC_CR_PLLON; 

    // 6. Обновляем SystemCoreClock (стандартный способ CMSIS)
    SystemCoreClockUpdate(); 
    return 1; // Успех
}

// --- Вариант 4: HSE -> PLL -> 48 МГц (Кварц 8 МГц) ---
uint8_t ClockInitHSE48MHz(void)
{
    uint32_t timeout;

    // 1. Включаем HSE и ждем готовности
    RCC->CR |= RCC_CR_HSEON;
    //Ждем успешного запуска или окончания тайм-аута
    for (timeout = 0; ; timeout++)
    {
        //Если успешно запустилось, то выходим из цикла
        if (RCC->CR & RCC_CR_HSERDY){
        // Ожидаем, пока HSE не стабилизируется
            break;
        }
        //Если не запустилось, то отключаем все, что включили и возвращаем ошибку
        if (timeout > 0x1000){
            RCC->CR &= ~(RCC_CR_HSEON); //Останавливаем HSE
            return 1;
        }
    }

    // 2. Настройка Flash (1 wait state для 48 МГц)
    // Настройка частоты доступа к FLASH-памяти
    FLASH->ACR &= ~FLASH_ACR_LATENCY; // Сброс битов LATENCY
    FLASH->ACR = FLASH_ACR_LATENCY | FLASH_ACR_PRFTBE;

    // 3. Настройка PLL
    // Выключаем PLL перед изменением параметров (на всякий случай)
    RCC->CR &= ~RCC_CR_PLLON;
    while(RCC->CR & RCC_CR_PLLRDY);

    // Очищаем биты источника, предделителя (PREDIV) и множителя
    RCC->CFGR &= ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLMUL);
    
    // Источник PLL = HSE (через PREDIV)
    RCC->CFGR |= RCC_CFGR_PLLSRC_HSE_PREDIV; 
    
    // Убеждаемся, что PREDIV равен 1 (т.е. вход 8 МГц не делится)
    RCC->CFGR2 &= ~RCC_CFGR2_PREDIV;
    RCC->CFGR2 |= RCC_CFGR2_PREDIV_DIV1;

    // Множитель x6 (8 МГц * 6 = 48 МГц)
    RCC->CFGR |= RCC_CFGR_PLLMUL6;

    // 4. Включаем PLL и ждем стабилизации
    RCC->CR |= RCC_CR_PLLON;
    //Ждем успешного запуска или окончания тайм-аута
    for(timeout=0; ; timeout++)
    {
        //Если успешно запустилось, то выходим из цикла
        if (RCC->CR & RCC_CR_PLLRDY) {
            // Ожидаем, пока PLL не стабилизируется
            break;
        }
        
        //Если по каким-то причинам не запустился PLL, то отключаем все, что включили и возвращаем ошибку
        if(timeout > 0x1000)
        {
            RCC->CR &= ~(RCC_CR_HSEON); //Останавливаем HSE
            RCC->CR &= ~(RCC_CR_PLLON); //Останавливаем PLL
            return 2;
        }
    }

    // 5. Настройка делителей шин (AHB=1, APB=1)
    RCC->CFGR &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE);
    RCC->CFGR |= (RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE_DIV1);

    // 6. Переключение системы на PLL
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    timeout=0;
    while (((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) && timeout < 0xFFFF) {
        timeout++;
    }

    // 7. Обновляем глобальную переменную частоты
    SystemCoreClockUpdate();
    
    return 0; // УСПЕХ
}

// --- Вариант 5: HSE -> PLL -> 32 МГц (Кварц 8 МГц) ---
uint8_t ClockInitHSE32MHz(void)
{
    uint32_t timeout = 0xFFFF;

    // 1. Включаем HSE (обязательно, так как он источник для PLL)
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY) && --timeout);
    if (timeout == 0) return 1;

    // 2. Настройка Flash (1 wait state для 32 МГц)
    // Настройка частоты доступа к FLASH-памяти
    FLASH->ACR &= ~FLASH_ACR_LATENCY; // Сброс битов LATENCY
    FLASH->ACR = FLASH_ACR_LATENCY | FLASH_ACR_PRFTBE;

    // 3. Настройка PLL: Источник HSE, PREDIV /1, MUL x4 (8 * 4 = 32)
    // Перед настройкой PLL должен быть выключен
    RCC->CR &= ~RCC_CR_PLLON;
    while(RCC->CR & RCC_CR_PLLRDY);

        // Очищаем биты источника, предделителя (PREDIV) и множителя
    RCC->CFGR &= ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLMUL);

    // Источник PLL = HSE (через PREDIV)
    RCC->CFGR |= RCC_CFGR_PLLSRC_HSE_PREDIV; 
    
    // Убеждаемся, что PREDIV равен 1 (т.е. вход 8 МГц не делится)
    RCC->CFGR2 &= ~RCC_CFGR2_PREDIV;
    RCC->CFGR2 |= RCC_CFGR2_PREDIV_DIV1;

    // Множитель x6 (8 МГц * 4 = 32 МГц)
    RCC->CFGR |= RCC_CFGR_PLLMUL4;

    // 4. Включаем PLL и ждем стабилизации
    RCC->CR |= RCC_CR_PLLON;
    timeout = 0xFFFF;
    while (!(RCC->CR & RCC_CR_PLLRDY) && --timeout);
    if (timeout == 0) return 2;

    // 5. Переключение системы на PLL
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

    // 6. Обновляем переменную частоты
    SystemCoreClockUpdate();
    
    return 0; // Успех
}

// --- Вариант 6: Прямой HSE 8 МГц (Без PLL) ---
uint8_t ClockInitHSE8MHz(void)
{
    uint32_t timeout = 0xFFFF;

    // 1. Включаем HSE (обязательно, так как он источник для PLL)
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY) && --timeout);
    if (timeout == 0) return 1;
    
    // 2. Настройка Flash: для 8 МГц задержки не нужны (0 Wait State)
    // Но полезно сбросить настройки на дефолтные
    FLASH->ACR &= ~(FLASH_ACR_LATENCY | FLASH_ACR_PRFTBE);
    
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_HSE;

    // 5. Ожидаем подтверждения переключения (SWS)
    timeout = 0xFFFF;
    while(((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSE) && --timeout);
    if (timeout == 0) return 2; // Ошибка переключения
    
    // 6. Выключаем PLL (не нужен для этого режима, экономим ток)
    RCC->CR &= ~RCC_CR_PLLON;
    while (RCC->CR & RCC_CR_PLLRDY); // Ждем остановки, если был включен
    
    // 7. Обновляем глобальную переменную частоты (запишет 8000000)
    SystemCoreClockUpdate();
    
    return 0; // Успех
}