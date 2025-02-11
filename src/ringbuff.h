/**
 *******************************************
 * @file    ringbuff.h
 * @author  Anton Emeltsev
 * @version 1.0
 * @date	10-February-2020
 * @brief   This header file part of Cortex-M0, M0+ lib
 *******************************************
 */
#ifndef RINGBUFF_H
#define RINGBUFF_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define BUFFER_SIZE 128
#define CLEAR_ARRAY 1

typedef enum BUFFER_STATUS {
    STATUS_OK,        // Ok
    STATUS_ERR,       // Error
    STATUS_OVERFLOW,  // Buffer is overflow
    STATUS_EMPTY,     // Buffer is empty
} BUFFER_STATUS;

typedef struct ringbuffer {
    volatile uint16_t tail; // Tail for read
    volatile uint16_t head; // Head for write
    uint16_t size;          // Buffwe size
    volatile uint8_t* data; // Pointer to buffer array
} ringbuffer_t;

BUFFER_STATUS buffer_init(ringbuffer_t *rb,  uint8_t *data, uint8_t data_clear, uint16_t size); 
BUFFER_STATUS buffer_clear(ringbuffer_t *rb);
uint16_t buffer_count(const ringbuffer_t *rb);
bool buffer_full(const ringbuffer_t *rb);
bool buffer_empty(const ringbuffer_t *rb);
BUFFER_STATUS buffer_peek(const ringbuffer_t *rb, uint8_t *elem);
BUFFER_STATUS buffer_back(ringbuffer_t *rb);
BUFFER_STATUS buffer_put(ringbuffer_t *rb, uint8_t byte);
BUFFER_STATUS buffer_get(ringbuffer_t *rb, uint8_t *byte);
BUFFER_STATUS buffer_put_string(ringbuffer_t *rb, uint8_t *string);
bool buffer_find_byte(ringbuffer_t *rb, uint8_t byte);
uint16_t buffer_get_token(ringbuffer_t *rb, uint8_t *str, uint16_t len, uint8_t term);

#endif