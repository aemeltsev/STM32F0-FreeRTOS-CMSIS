#include "button.h"

void init_button_led(void)
{
    // Нужная кнопка подключена к 0-ому выводу порта A, а светодиод - к 13-ому выводу порта C.
    // Enable the GPIOA and GPIOC peripheral clocks
    SET_BIT(RCC->AHBENR, (RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOCEN));

    // Configure GPIOA Pin 0 as input with pull-down - button
    CLEAR_BIT(GPIOA->MODER, GPIO_MODER_MODER0); // Clear the mode bits for Pin 0
    CLEAR_BIT(GPIOA->PUPDR, GPIO_PUPDR_PUPDR0); // Clear the pull-up/pull-down bits for Pin 0
    SET_BIT(GPIOA->PUPDR, GPIO_PUPDR_PUPDR0_1); // Set pull-down for Pin 0

    // Configure GPIOC Pin 13 as output push-pull - led
    CLEAR_BIT(GPIOC->MODER, GPIO_MODER_MODER13); // Clear the mode bits for Pin 13
    SET_BIT(GPIOC->MODER, GPIO_MODER_MODER13_0); // Set output mode for Pin 13
    CLEAR_BIT(GPIOA->OTYPER, GPIO_OTYPER_OT_13); // Set push-pull type for Pin 13
    CLEAR_BIT(GPIOA->OSPEEDR, GPIO_OSPEEDER_OSPEEDR13); // Clear the speed bits for Pin 13
    SET_BIT(GPIOA->OSPEEDR, GPIO_OSPEEDER_OSPEEDR13_0); // Set medium speed for Pin 13
}

void button_to_led(uint8_t *button_state)
{
    // Toggle the LED when the button is pressed
    (*button_state) = (uint8_t)(GPIOA->IDR & GPIO_IDR_0) >> 0; // Read the button state
    if (*button_state == 0) // Button is pressed
    {
        delay_ms(5000); // Wait for 5 seconds before toggling the LED again
        GPIOC->BSRRL |= GPIO_BSRR_BS_13; // Set GPIOC Pin 13 (turn on the LED)
    }
    else // Button is released
    {
        delay_ms(5000); // Wait for 5 seconds before toggling the LED again
        GPIOC->BSRRH |= GPIO_BSRR_BR_13; // Reset GPIOC Pin 13 (turn off the LED)
    }
}

void init_buttons_for_bin_sum(void)
{
    /* Реализовать сумматор двоичных чисел.
     В ходе работы микроконтроллер должен считывать
     с переключателей два 4-х значных двоичных числа
     и выводить их сумму на светодиоды.
     Нужные кнопки подключены к выводам 0, 1, 2, 3, 4, 5, 6, 7 порта A - DIP-переключатель 8 контактов,
     а светодиоды - к 8-ому, 9-ому, 10-ому, 11-ому выводу порта A.
     */
    // Enable the GPIOA peripheral clocks
    SET_BIT(RCC->AHBENR, RCC_AHBENR_GPIOAEN);

    // Configure GPIOA Pins 0, 1, 2, 3, 4, 5, 6, 7 as input with pull-down - buttons
    // Clear the mode bits for Pins 0, 1, 2, 3, 4, 5, 6, 7
    CLEAR_BIT(GPIOA->MODER, (GPIO_MODER_MODER0 | GPIO_MODER_MODER1 | 
                             GPIO_MODER_MODER2 | GPIO_MODER_MODER3 |
                             GPIO_MODER_MODER4 | GPIO_MODER_MODER5 | 
                             GPIO_MODER_MODER6 | GPIO_MODER_MODER7));
    // Clear the pull-up/pull-down bits for Pins 0, 1, 2, 3, 4, 5, 6, 7
    CLEAR_BIT(GPIOA->PUPDR, (GPIO_MODER_MODER0 | GPIO_MODER_MODER1 | 
                             GPIO_MODER_MODER2 | GPIO_MODER_MODER3 |
                             GPIO_MODER_MODER4 | GPIO_MODER_MODER5 |
                             GPIO_MODER_MODER6 | GPIO_MODER_MODER7));
    // Set pull-down for Pins 0, 1, 2, 3, 4, 5, 6, 7
    SET_BIT(GPIOA->PUPDR, (GPIO_MODER_MODER0_1 | GPIO_MODER_MODER1_1 | 
                           GPIO_MODER_MODER2_1 | GPIO_MODER_MODER3_1 |
                           GPIO_MODER_MODER4_1 | GPIO_MODER_MODER5_1 |
                           GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1));


    // Configure GPIOA Pins 8, 9, 10, 11 as output push-pull - leds
    // Clear the mode bits for Pins 8, 9, 10, 11
    CLEAR_BIT(GPIOA->MODER, (GPIO_MODER_MODER8 | GPIO_MODER_MODER9 | 
                             GPIO_MODER_MODER10 | GPIO_MODER_MODER11));
    // Set output mode for Pins 8, 9, 10, 11
    SET_BIT(GPIOA->MODER, (GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0 | 
                             GPIO_MODER_MODER10_0 | GPIO_MODER_MODER11_0));
    // Set push-pull type for Pins 8, 9, 10, 11
    CLEAR_BIT(GPIOA->OTYPER, (GPIO_OTYPER_OT_8 | GPIO_OTYPER_OT_9 | 
                             GPIO_OTYPER_OT_10 | GPIO_OTYPER_OT_11)); 
    // Clear the speed bits for Pins 8, 9, 10, 11
    CLEAR_BIT(GPIOA->OSPEEDR, (GPIO_OSPEEDER_OSPEEDR8 | GPIO_OSPEEDER_OSPEEDR9 |
                               GPIO_OSPEEDER_OSPEEDR10 | GPIO_OSPEEDER_OSPEEDR11));
    // Set medium speed for Pins 8, 9, 10, 11
    SET_BIT(GPIOA->OSPEEDR, (GPIO_OSPEEDER_OSPEEDR8_0 | GPIO_OSPEEDER_OSPEEDR9_0 |
                               GPIO_OSPEEDER_OSPEEDR10_0 | GPIO_OSPEEDER_OSPEEDR11_0));
}

void buttons_bin_sum_to_led(uint8_t x_a, uint8_t x_b, uint8_t x_c, uint8_t x_d,\
                            uint8_t y_a, uint8_t y_b, uint8_t y_c, uint8_t y_d)
{
    // Read the binary numbers from the buttons
    uint8_t a = bin_to_dec(x_a, x_b, x_c, x_d);
    uint8_t b = bin_to_dec(y_a, y_b, y_c, y_d);
    uint8_t sum = (a + b)%16;
    uint8_t out[4] = {};
    dec_to_bin(sum, out);
    // 1st rank
    if (out[0] == 1) {
        GPIOA->BSRRL |= GPIO_BSRR_BS_8; // Reset GPIOA Pin 8 (turn off the LED)
    } else {
        GPIOA->BSRRH |= GPIO_BSRR_BR_8; // Reset GPIOA Pin 8 (turn off the LED)
    }
    // 2nd rank
    if (out[1] == 1) {
        GPIOA->BSRRL |= GPIO_BSRR_BS_9; // Reset GPIOA Pin 9 (turn off the LED)
    } else {
        GPIOA->BSRRH |= GPIO_BSRR_BR_9; // Reset GPIOA Pin 9 (turn off the LED)
    }
    // 3rd rank
    if (out[2] == 1) {
        GPIOA->BSRRL |= GPIO_BSRR_BS_10; // Reset GPIOA Pin 10 (turn off the LED)
    } else {
        GPIOA->BSRRH |= GPIO_BSRR_BR_10; // Reset GPIOA Pin 10 (turn off the LED)
    }
    // 4th rank
    if (out[3] == 1) {
        GPIOA->BSRRL |= GPIO_BSRR_BS_11; // Reset GPIOA Pin 11 (turn off the LED)
    } else {
        GPIOA->BSRRH |= GPIO_BSRR_BR_11; // Reset GPIOA Pin 11 (turn off the LED)
    }
}

void init_buttons_for_sel_led(void)
{
    /* Реализовать выбор светодиода.
     В ходе работы микроконтроллер должен считывать
     с переключателей 2-х значных двоичных числа
     и выбирать светодиод по номеру числа.
     Нужные кнопки подключены к выводам 0, 1 порта A - DIP-переключатель 2 контакта,
     а светодиоды - к 2-ому, 3-ому, 4-ому выводу порта A.
     */
    // Enable the GPIOA peripheral clock
    SET_BIT(RCC->AHBENR, RCC_AHBENR_GPIOAEN);

    // Configure GPIOA Pins 0, 1 as input with pull-down - buttons
    // Clear the mode bits for Pins 0, 1
    CLEAR_BIT(GPIOA->MODER, (GPIO_MODER_MODER0 | GPIO_MODER_MODER1));
    // Clear the pull-up/pull-down bits for Pins 0, 1
    CLEAR_BIT(GPIOA->PUPDR, (GPIO_MODER_MODER0 | GPIO_MODER_MODER1));
    // Set pull-down for Pins 0, 1
    SET_BIT(GPIOA->PUPDR, (GPIO_MODER_MODER0_1 | GPIO_MODER_MODER1_1));

    // Configure GPIOA Pins 2, 3, 4 as output push-pull - leds
    // Clear the mode bits for Pins 2, 3, 4
    CLEAR_BIT(GPIOA->MODER, (GPIO_MODER_MODER2 | GPIO_MODER_MODER3 | 
                             GPIO_MODER_MODER4));
    // Set output mode for Pins 2, 3, 4
    SET_BIT(GPIOA->MODER, (GPIO_MODER_MODER2_0 | GPIO_MODER_MODER3_0 | 
                             GPIO_MODER_MODER4_0));
    // Set push-pull type for Pins 2, 3, 4
    CLEAR_BIT(GPIOA->OTYPER, (GPIO_OTYPER_OT_2 | GPIO_OTYPER_OT_3 | 
                             GPIO_OTYPER_OT_4));
    // Clear the speed bits for Pins 2, 3, 4
    CLEAR_BIT(GPIOA->OSPEEDR, (GPIO_OSPEEDER_OSPEEDR2 | GPIO_OSPEEDER_OSPEEDR3 |
                               GPIO_OSPEEDER_OSPEEDR4));
    // Set medium speed for Pins 2, 3, 4
    SET_BIT(GPIOA->OSPEEDR, (GPIO_OSPEEDER_OSPEEDR2_0 | GPIO_OSPEEDER_OSPEEDR3_0 |
                               GPIO_OSPEEDER_OSPEEDR4_0));
}

void buttons_sel_led(void)
{
    // Select the LED when someone combination buttons is pressed
    if ((GPIOA->IDR & (GPIO_IDR_0) == 1) && (GPIOA->IDR & (GPIO_IDR_1) == 0)) // [1:0] = Pin 2 LED on
    {
        GPIOA->BSRRL |= GPIO_BSRR_BS_2; // Set GPIOA Pin 2 (turn on the LED)
        GPIOA->BSRRH |= GPIO_BSRR_BR_3; // Reset GPIOA Pin 2 (turn off the LED)
        GPIOA->BSRRH |= GPIO_BSRR_BR_4; // Reset GPIOA Pin 3 (turn off the LED)
    } else if ((GPIOA->IDR & (GPIO_IDR_0) == 0) && (GPIOA->IDR & (GPIO_IDR_1) == 1)) // [0:1] = Pin 3 LED on
    {
        GPIOA->BSRRL |= GPIO_BSRR_BS_3; // Set GPIOA Pin 3 (turn on the LED)
        GPIOA->BSRRH |= GPIO_BSRR_BR_2; // Reset GPIOA Pin 2 (turn off the LED)
        GPIOA->BSRRH |= GPIO_BSRR_BR_4; // Reset GPIOA Pin 4 (turn off the LED)
    } else if ((GPIOA->IDR & (GPIO_IDR_0) == 1) && (GPIOA->IDR & (GPIO_IDR_1) == 1)) // [1:1] = Pin 4 LED on
    {
        GPIOA->BSRRL |= GPIO_BSRR_BS_4; // Set GPIOA Pin 4 (turn on the LED)
        GPIOA->BSRRH |= GPIO_BSRR_BR_2; // Reset GPIOA Pin 2 (turn off the LED)
        GPIOA->BSRRH |= GPIO_BSRR_BR_3; // Reset GPIOA Pin 3 (turn off the LED)
    }
}