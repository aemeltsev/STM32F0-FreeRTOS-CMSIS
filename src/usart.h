#ifndef USART_H
#define USART_H

#include <stdint.h>
#include "stm32f0xx.h"

#define UART_BUF_SIZE 64

typedef enum {
    ALL_OK,
    LINE_BUSY,
    STR_TOO_LONG
} tx_status;

void usart1_gpio_init(uint32_t baudrate, uint32_t clock);
void usart1_send_data(uint8_t *data, uint8_t len);
void usart1_send_byte(uint8_t data);
void usart1_receive_byte(uint8_t *data);
uint8_t usart1_getline(uint8_t **line);
tx_status usart1_send(const uint8_t *str, uint8_t len);
void usart1_send_hex(uint8_t num);

#endif //USART_H