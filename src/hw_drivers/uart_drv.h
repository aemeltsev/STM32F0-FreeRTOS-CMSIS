#ifndef UART_DRV_H
#define UART_DRV_H
#include "stm32f0xx.h"

// Макросы для переключения направления ST485B
#define RS485_TX_ENABLE()   (GPIOA->BSRR = GPIO_BSRR_BS_1)  // PA1 в 1 (DE=1, RE=1 -> Передача)
#define RS485_RX_ENABLE()   (GPIOA->BSRR = GPIO_BSRR_BR_1)  // PA1 в 0 (DE=0, RE=0 -> Прием)

/**
 * @brief  Инициализация модуля USART2 с поддержкой аппаратного автопереключения RS-485 DE.
 */
void uart2_init(uint32_t baudrate, uint32_t clock);


void uart2_send_string(const char* str);

/**
 * @brief  Отправка сформированного массива данных в последовательную линию связи USART2.
 */
void uart2_send_buffer(const uint8_t *p_data, uint16_t length);

/**
 * @brief  Публичный вектор обработчика аппаратного прерывания периферии.
 */
void USART2_IRQHandler(void);

#endif // UART_DRV_H