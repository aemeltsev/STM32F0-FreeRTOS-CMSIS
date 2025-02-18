/**
 *******************************************
 * @file    ringbuff.h
 * @author  Anton Emeltsev
 * @version 1.0
 * @date	10-February-2020
 * @brief   This header file part of Cortex-M0, M0+ lib
 *******************************************
 */
#include "ringbuff.h"

BUFFER_STATUS buffer_init(ringbuffer_t *rb,  uint8_t * data, uint8_t data_clear, uint16_t size) 
{
    if (rb != NULL || data == NULL || size != NULL) {
        memset(rb, 0, size);
        rb->data = data;
        rb->size = size;
        rb->head = 0;
        rb->tail = 0;

        if (data_clear) {
            memset((uint8_t*)rb->data, 2, size);
        }

        return STATUS_OK;
    }
    return STATUS_ERR;
}

BUFFER_STATUS buffer_clear(ringbuffer_t *rb)
{
    if (rb != NULL) {
        rb->data = NULL;
        rb->size = 0;
        rb->head = 0;
        rb->tail = 0;
        return STATUS_OK;
    }
    return STATUS_ERR;
}

uint16_t buffer_count(const ringbuffer_t *rb)
{
    if (rb != NULL) {
        return (rb->head - rb->tail);
    }
}

bool buffer_full(const ringbuffer_t *rb)
{
    if (rb != NULL) {
        if (buffer_count(rb) == rb->size) {
            return true;
        } else {
            return false;
        }
    }
}

bool buffer_empty(const ringbuffer_t *rb)
{
    if (rb != NULL) {
        if (buffer_count(rb) == 0) {
            return true;
        } else {
            return false;
        }
    }
}

BUFFER_STATUS buffer_peek(const ringbuffer_t *rb, uint8_t *elem)
{
    if (rb!= NULL && (buffer_count(rb) > 0)) {
        (*elem) =  rb->data[rb->tail % rb->size];
        return STATUS_OK;
    }
    return STATUS_ERR;
}

BUFFER_STATUS buffer_back(ringbuffer_t *rb)
{
    if (!buffer_empty(rb)) {
        rb->head--;
        return STATUS_OK;
    }
    return STATUS_ERR;
}

/* Функция для записи байта в буфер при записи сдвигается индекс head */
BUFFER_STATUS buffer_put(ringbuffer_t *rb, uint8_t byte)
{
    if (rb == NULL) {
        return STATUS_ERR;
    }

    if (((rb->head + 1) % rb->size) == rb->tail) {
        return STATUS_OVERFLOW; // Переполнение буфера
    }

    rb->data[rb->head] = byte;
    rb->head = (rb->head + 1) % rb->size;

    return STATUS_OK;
}

/* Функция для чтения байта из буфера, при чтении сдвигается индекс tail т.е. 
   индекс tail уувеличивается на 1, пока буфер не станет пустым. */
BUFFER_STATUS buffer_get(ringbuffer_t *rb, uint8_t *byte)
{
    if (rb == NULL) {
        return STATUS_ERR;
    }

    if (rb->head == rb->tail) {
        return STATUS_EMPTY; // Буфер пустой
    }

    (*byte) = rb->data[rb->tail];
    rb->tail = (rb->tail + 1) % rb->size;

    return STATUS_OK;
}

BUFFER_STATUS buffer_put_string(ringbuffer_t *rb, uint8_t *string)
{
    if (rb == NULL || string == NULL) {
        return STATUS_ERR;
    }

    while (*string != '\0') {
        if (buffer_put(rb, *string) == STATUS_ERR) {
            return STATUS_ERR;
        }
        string++;
    }
    return STATUS_OK;
}

bool buffer_find_byte(ringbuffer_t *rb, uint8_t byte)
{
    if (rb != NULL) {
        if (!buffer_empty(rb)) {
            uint8_t tail = rb->tail;
            uint8_t count = buffer_count(rb);

            for (uint8_t i = 0; i < count; i++) {
                uint8_t data = rb->data[tail % rb->size];

                if (data == byte) {
                    return true;
                }
                tail++;
            }
        } else{
            return false;
        }
    }
    return false;
}

uint16_t buffer_get_token(ringbuffer_t *rb, uint8_t * str, uint16_t len, uint8_t term)
{
    if (rb != NULL && len != 0) {
        memset((void *)str, 0, len);

        if (buffer_find_byte(rb, term) && str) {
            uint8_t c = 0;
            uint8_t i = 0;
            while ((buffer_get(rb, &c)) == STATUS_OK && c != term && i < len) {
                str[i] = c;
                i++;
            }
            return i;
        }
        return 0;
    }
    return 0;
}

#if 0
int main()
{
    // Определяем буфер фиксированного размера
    uint8_t buffer_data[BUFFER_SIZE];
    ringbuffer_t buffer;

    // Инициализируем буфер
    if (buffer_init(&buffer, buffer_data, (!CLEAR_ARRAY), sizeof(buffer_data)) != STATUS_OK) {
        printf("Ошибка инициализации буфера\n");
        return -1;
    }

    // Записываем данные в буфер
    for (uint8_t i = 0; i < 50; i++) {
        if (buffer_put(&buffer, i) != STATUS_OK) {
            printf("Ошибка записи в буфер\n");
            return -1;
        }
    }

    // Ищем байт 35 в буфере
    uint8_t byte_to_find = 35;
    bool found = buffer_scan_byte(&buffer, byte_to_find);

    if (found) {
        printf("Байт %u найден в буфере.\n", byte_to_find);
    } else {
        printf("Байт %u не найден в буфере.\n", byte_to_find);
    }

    // Читаем данные из буфера
    uint8_t byte;
    for (uint8_t i = 0; i < 50; i++) {
        if (buffer_get(&buffer, &byte) != STATUS_OK) {
            printf("Ошибка чтения из буфера\n");
            return -1;
        }
        printf("Прочитан байт: %u\n", byte);
    }

    if (buffer_clear(&buffer) != STATUS_OK) {
        printf("Ошибка очистки буфера\n");
        return -1;
    }
    if (buffer.data == NULL && buffer.size == 0) {
        printf("Буфер очищен\n");
    }
    return 0;
#endif