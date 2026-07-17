#include "test_led.h"

// Use PC13 Led - this is default led's pin for many dev doards.
void led_init()
{
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    
    CLEAR_BIT(GPIOC->MODER, GPIO_MODER_MODER13);
    SET_BIT(GPIOC->MODER, GPIO_MODER_MODER13_0);
    CLEAR_BIT(GPIOC->OTYPER, GPIO_OTYPER_OT_13);
    CLEAR_BIT(GPIOC->OSPEEDR, GPIO_OSPEEDER_OSPEEDR13);
    SET_BIT(GPIOC->OSPEEDR, GPIO_OSPEEDER_OSPEEDR13_0);
}

// TODO
void led_blink()
{
            // Включаем светодиод (устанавливаем PC13 в 1)
        *(uint32_t*)((uint32_t)GPIOC + 0x18) = GPIO_BSRR_BS_13;
        // Задержка
        for (volatile uint32_t i = 0; i < 1000000; i++);


        // Выключаем светодиод (сбрасываем PC13 в 0)
        *(uint32_t*)((uint32_t)GPIOC + 0x18) = GPIO_BSRR_BR_13;
        // Задержка
        for (volatile uint32_t i = 0; i < 1000000; i++);


        // Включаем светодиод (устанавливаем PC13 в 1)
        *(uint32_t*)((uint32_t)GPIOC + 0x18) = GPIO_BSRR_BS_13;
        // Задержка
        for (volatile uint32_t i = 0; i < 1000000; i++);
        
        // Выключаем светодиод (сбрасываем PC13 в 0)
        *(uint32_t*)((uint32_t)GPIOC + 0x18) = GPIO_BSRR_BR_13;
        // Задержка
        for (volatile uint32_t i = 0; i < 1000000; i++);
        
        
        // Включаем светодиод (устанавливаем PC13 в 1)
        *(uint32_t*)((uint32_t)GPIOC + 0x18) = GPIO_BSRR_BS_13;
        // Задержка
        for (volatile uint32_t i = 0; i < 9000000; i++);
}