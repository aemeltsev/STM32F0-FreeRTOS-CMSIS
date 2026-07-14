#include "system_init.h"

#define CLOCK_TIMEOUT    ((uint32_t)0x000FFFFF)

/**
 * @brief  Configures the System clock to 48MHz using internal HSI (8MHz) -> PLL x12 / 2
 * @return 0 = Success, 1 = Clock Configuration Timeout/Error
 */
uint8_t clock_init_hsi_48MHz(void)
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
    uint32_t timeout = CLOCK_TIMEOUT;
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
    timeout = CLOCK_TIMEOUT;
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

/**
 * @brief  Configures the System clock to 24MHz using internal HSI (8MHz) -> PLL x6 / 2
 * @return 0 = Success, 1 = Clock Configuration Timeout/Error
 */
uint8_t clock_init_hsi_24MHz(void)
{
    uint32_t timeout = CLOCK_TIMEOUT;

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
    timeout = CLOCK_TIMEOUT;
    while (!(RCC->CR & RCC_CR_PLLRDY) && --timeout);
    if (timeout == 0) return 0;

    // 5. Переключаем систему (SW) на источник PLL
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

    // 6. Обновляем глобальную переменную частоты
    SystemCoreClockUpdate();
    return 1; // Успех
}

/**
 * @brief  Bypasses the PLL and runs directly off the raw internal 8MHz HSI
 * @return 0 = Success, 1 = Clock Configuration Timeout/Error
 */
uint8_t clock_init_hsi_8MHz(void)
{
    uint32_t timeout = CLOCK_TIMEOUT;

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

/**
 * @brief  Configures System clock to 48MHz using External Crystal (Assumes an 8MHz HSE)
 *         Formula: HSE (8MHz) / PREDIV(1) -> 8MHz * PLLMUL(6) = 48MHz
 * @return 0 = Success, 1 = Clock Configuration Timeout/Error
 */
uint8_t clock_init_hse_48MHz(void)
{
    uint32_t timeout = CLOCK_TIMEOUT;

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

/**
 * @brief  Configures System clock to 32MHz using External Crystal (Assumes an 8MHz HSE)
 *         Formula: HSE (8MHz) / PREDIV(1) -> 8MHz * PLLMUL(4) = 32MHz
 * @return 0 = Success, 1 = Clock Configuration Timeout/Error
 */
uint8_t clock_init_hse_32MHz(void)
{
    uint32_t timeout = RCC_CR_HSEON;

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
    timeout = RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_PLLRDY) && --timeout);
    if (timeout == 0) return 2;

    // 5. Переключение системы на PLL
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

    // 6. Обновляем переменную частоты
    SystemCoreClockUpdate();
    
    return 0; // Успех
}

/**
 * @brief Configures System clock to 8MHz using External Crystal (Without PLL)
 * @return 0 = Success, 1 = Clock Configuration Timeout/Error
 */
uint8_t clock_init_hse_8MHz(void)
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
    timeout = RCC_CR_HSEON;
    while(((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSE) && --timeout);
    if (timeout == 0) return 2; // Ошибка переключения
    
    // 6. Выключаем PLL (не нужен для этого режима, экономим ток)
    RCC->CR &= ~RCC_CR_PLLON;
    while (RCC->CR & RCC_CR_PLLRDY); // Ждем остановки, если был включен
    
    // 7. Обновляем глобальную переменную частоты (запишет 8000000)
    SystemCoreClockUpdate();
    
    return 0; // Успех
}

/**
 * @brief  Configures all physical GPIO pins for buzzer, error/link LEDs, SPI1, SPI2, UART2, and I2C1
 */
void system_gpio_init(void)
{
    // 1. Enable system bus clocks for Ports A, B, and C
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN;

    // =========================================================================
    //  1. DIGITAL PERIPHERALS: BUZZER & LEDS (General Purpose Outputs)
    // =========================================================================
    // BUZZER    -> PB8
    // ERROR LED -> PC13 (Common choice for onboard user LEDs)
    // LINK LED  -> PB9

    // Configure PB8 (Buzzer) and PB9(Link) as GP output(01) and PP + PD - led
    GPIOB->MODER &= ~(GPIO_MODER_MODER8 | GPIO_MODER_MODER9); // Clear the mode bits for Pin 8, Pin 9
    GPIOB->MODER |= (GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0); // Set output mode for Pin 8, Pin 9
    GPIOB->OTYPER  &= ~(GPIO_OTYPER_OT_8 | GPIO_OTYPER_OT_9); // Set push-pull type for Pin 8, Pin 9
    GPIOB->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR8 | GPIO_OSPEEDER_OSPEEDR9); // Clear the speed bits for Pin 8, Pin 9
    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR8 | GPIO_PUPDR_PUPDR9); // Clear pull-up/pull-down register 
    GPIOB->PUPDR |= (GPIO_PUPDR_PUPDR8_0 | GPIO_PUPDR_PUPDR9_0); // Set Pull-up type for Pin 8, Pin 9
    GPIOB->BSRR = (GPIO_BSRR_BR_8 | GPIO_BSRR_BR_9); // Force LOW at start
    
    // Конфигурация PC13 (Error LED)
    GPIOC->MODER &= ~GPIO_MODER_MODER13;
    GPIOC->MODER |= GPIO_MODER_MODER13_0; // Set output mode for Pin 13
    GPIOC->OTYPER &= ~GPIO_OTYPER_OT_13; // Set push-pull type for Pin 13
    GPIOC->OSPEEDR &= ~GPIO_OSPEEDER_OSPEEDR13; // Clear the speed bits for Pin 13
    GPIOC->PUPDR &= ~GPIO_PUPDR_PUPDR13; // Clear pull-up/pull-down register 
    GPIOC->PUPDR |= GPIO_PUPDR_PUPDR13_0; // Set Pull-up type for Pin 13
    GPIOC->BSRR = GPIO_BSRR_BR_13; // Force LOW at start


    // =========================================================================
    //  2. HALF-DUPLEX SPI1 (Master Data exchange with 330R + SRV05 protection)
    // =========================================================================
    // PA4 -> SPI1_NSS / CS  (Output Mode + Pull-Up for tracking EXTI shifts)
    // PB2 -> SPI1_NSS / CS  (Output Mode + Pull-Up for tracking EXTI shifts)
    // PB1 -> SPI1_NSS / CS  (Output Mode + Pull-Up for tracking EXTI shifts)
    // PB0 -> SPI1_NSS / CS  (Output Mode + Pull-Up for tracking EXTI shifts)
    // PA5 -> SPI1_SCK       (Alternate Function AF0)
    // PA7 -> SPI1_MOSI      (Alternate Function AF0 - Single line data stream)

    // 1. Настройка SPI1: PA5 (SCK) и PA7 (MOSI) -> Alternate Function (Режим 10)
    GPIOA->MODER &= ~(GPIO_MODER_MODER5 | GPIO_MODER_MODER7);
    GPIOA->MODER |= GPIO_MODER_MODER5_1 | GPIO_MODER_MODER7_1;

    // Выбор AF0 для пинов PA5 и PA7 (в AFR регистре это нули пропишем явно для надежности)
    GPIOA->AFR[0] &= ~((0xFU << GPIO_AFRL_AFSEL5_Pos) | (0xFU << GPIO_AFRL_AFSEL7_Pos));
    GPIOA->AFR[0] |= ((0x0U << GPIO_AFRL_AFSEL5_Pos) | (0x0U << GPIO_AFRL_AFSEL7_Pos));

    // 2. Настройка Software CS: PA4 -> General Purpose Output (Режим 01)
    //                           PB2 -> General Purpose Output (Режим 01)
    //                           PB1 -> General Purpose Output (Режим 01)
    //                           PB0 -> General Purpose Output (Режим 01)
    GPIOB->MODER &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1 | GPIO_MODER_MODER2);
    GPIOB->MODER |= (GPIO_MODER_MODER0_0 | GPIO_MODER_MODER1_0 | GPIO_MODER_MODER2_0);
    GPIOB->OTYPER &= ~(GPIO_OTYPER_OT_0 | GPIO_OTYPER_OT_1 | GPIO_OTYPER_OT_2);
    GPIOB->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR0 | GPIO_OSPEEDER_OSPEEDR1 | GPIO_OSPEEDER_OSPEEDR2);
    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR0 | GPIO_PUPDR_PUPDR1 | GPIO_PUPDR_PUPDR2); // Clear pull-up/pull-down register 
    GPIOB->PUPDR |= (GPIO_PUPDR_PUPDR0_0 | GPIO_PUPDR_PUPDR1_0 | GPIO_PUPDR_PUPDR2_0); // Set Pull-up type for Pin 13
    // Устанавливаем CS в 1 (девайс не выбран)
    GPIOB->BSRR = GPIO_BSRR_BS_0 | GPIO_BSRR_BS_1 | GPIO_BSRR_BS_2;

    GPIOA->MODER &= ~GPIO_MODER_MODER4;
    GPIOA->MODER |= GPIO_MODER_MODER4_0;
    GPIOA->OTYPER &= ~GPIO_OTYPER_OT_4;
    GPIOA->OSPEEDR &= ~GPIO_OSPEEDER_OSPEEDR4;
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR4;
    GPIOA->PUPDR |= GPIO_PUPDR_PUPDR4_0;
    // Устанавливаем CS в 1 (девайс не выбран)
    GPIOA->BSRR = GPIO_BSRR_BS_4;

    // =========================================================================
    //  3. HALF-DUPLEX SPI2 (Master Data exchange with 330R + SRV05 protection)
    // =========================================================================
    // PB12 -> SPI2_NSS / CS  (Output Mode + Pull-Up for tracking EXTI shifts)
    // PA9  -> SPI2_NSS / CS  (Output Mode + Pull-Up for tracking EXTI shifts)
    // PA10 -> SPI2_NSS / CS  (Output Mode + Pull-Up for tracking EXTI shifts)
    // PA8  -> SPI2_NSS / CS  (Output Mode + Pull-Up for tracking EXTI shifts)
    // PB13 -> SPI2_SCK       (Alternate Function AF0)
    // PB15 -> SPI2_MOSI      (Alternate Function AF0 - Single line data stream)

    // Настройка PB13 и PB15 в режим альтернативной функции (10)
    GPIOB->MODER &= ~(GPIO_MODER_MODER13 | GPIO_MODER_MODER15);
    GPIOB->MODER |= (GPIO_MODER_MODER13_1 | GPIO_MODER_MODER15_1);

    // Выбор AF0 (0x00) для PB13 и PB15 в верхнем регистре AFRH (AFR[1])
    GPIOB->AFR[0] &= ~((0xFU << GPIO_AFRH_AFSEL13_Pos) | (0xFU << GPIO_AFRH_AFSEL15_Pos));
    GPIOB->AFR[0] |= ((0x0U << GPIO_AFRH_AFSEL13_Pos) | (0x0U << GPIO_AFRH_AFSEL15_Pos));

    // Настройка CS линии на порту B: PB12
    GPIOB->MODER   &= ~GPIO_MODER_MODER12;
    GPIOB->MODER   |= GPIO_MODER_MODER12_0; // Output (01)
    GPIOB->OTYPER  &= ~GPIO_OTYPER_OT_12; // Push-Pull
    GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR12; // High Speed (11)
    GPIOB->PUPDR   &= ~GPIO_PUPDR_PUPDR12;                               
    GPIOB->PUPDR   |= GPIO_PUPDR_PUPDR12_0; // Pull-Up (01)
    GPIOB->BSRR     = GPIO_BSRR_BS_12; // Деактивируем CS (High)

    // Настройка CS линий на порту A: PA8, PA9, PA10
    GPIOA->MODER   &= ~(GPIO_MODER_MODER8 | GPIO_MODER_MODER9 | GPIO_MODER_MODER10);
    GPIOA->MODER   |= (GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0 | GPIO_MODER_MODER10_0); // Output (01)
    GPIOA->OTYPER  &= ~(GPIO_OTYPER_OT_8 | GPIO_OTYPER_OT_9 | GPIO_OTYPER_OT_10); // Push-Pull
    GPIOA->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR8 | GPIO_OSPEEDER_OSPEEDR9 | GPIO_OSPEEDER_OSPEEDR10); // High Speed (11)
    GPIOA->PUPDR   &= ~(GPIO_PUPDR_PUPDR8 | GPIO_PUPDR_PUPDR9 | GPIO_PUPDR_PUPDR10);       
    GPIOA->PUPDR   |= (GPIO_PUPDR_PUPDR8_0 | GPIO_PUPDR_PUPDR9_0 | GPIO_PUPDR_PUPDR10_0); // Pull-Up (01)
    GPIOA->BSRR     = (GPIO_BSRR_BS_8 | GPIO_BSRR_BS_9 | GPIO_BSRR_BS_10); // Деактивируем CS (High)

    // =========================================================================
    //  4. MODBUS UART2 WITH RS-485 DRIVER ENABLE (DE) CONTROL
    // =========================================================================
    // PA2 -> USART2_TX (Alternate Function AF1)
    // PA3 -> USART2_RX (Alternate Function AF1)
    // PA1 -> USART2_DE (Alternate Function AF1 - Hardware Auto-DE handling)

    GPIOA->MODER &= ~(GPIO_MODER_MODER1 | GPIO_MODER_MODER2 | GPIO_MODER_MODER3);
    GPIOA->MODER |= (GPIO_MODER_MODER1_1 | GPIO_MODER_MODER2_1 | GPIO_MODER_MODER3_1);

    // Route PA1, PA2, PA3 to AF1 (0x1)
    GPIOA->AFR[0] &= ~((0xFU << GPIO_AFRL_AFSEL1_Pos) | (0xFU << GPIO_AFRL_AFSEL2_Pos) | (0xFU << GPIO_AFRL_AFSEL3_Pos));
    GPIOA->AFR[0] |= ((0x1U << GPIO_AFRL_AFSEL1_Pos) | (0x1U << GPIO_AFRL_AFSEL2_Pos) | (0x1U << GPIO_AFRL_AFSEL3_Pos));

    // =========================================================================
    //  5. EEPROM I2C1 INTERFACE
    // =========================================================================
    // PB6 -> I2C1_SCL (Alternate Function AF1 + Open Drain)
    // PB7 -> I2C1_SDA (Alternate Function AF1 + Open Drain)

    // Сброс битов
    GPIOB->MODER &= ~(GPIO_MODER_MODER6 | GPIO_MODER_MODER7); // Сброс битов
    GPIOB->MODER |= (GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1); // Установка битов режима в альтернативную функцию (10)

    GPIOB->AFR[0] &= ~((0xFU << GPIO_AFRL_AFSEL6_Pos) | (0xFU << GPIO_AFRL_AFSEL7_Pos));
    GPIOB->AFR[0] |= ((0x1U << GPIO_AFRL_AFSEL6_Pos) | (0x1U << GPIO_AFRL_AFSEL7_Pos)); // Альтернативная функция AF1 - 0001: AF1 - I2C1_SCL, I2C1_SDA

    // Настраиваем аппаратный Открытый Коллектор/Сток для шины I2C
    GPIOB->OTYPER |= (GPIO_OTYPER_OT_6 | GPIO_OTYPER_OT_7); // 1: Output open-drain
}