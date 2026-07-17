#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include <stdint.h>

#define PACKET_SIZE  (12U) /**< Жестко фиксированный размер кадра SPI */

/**
 * @brief Простая универсальная структура для пакетного обмена по SPI1
 */
typedef struct __attribute__((packed)) {
    uint16_t transaction_id;  /**< ID пакета для синхронизации */
    uint8_t  cmd;             /**< Код команды / Номер регистра */
    
    // Универсальный массив полезной нагрузки (Payload) на 8 байт.
    // Сюда можно положить что угодно: float, uint32_t или четыре uint16_t.
    uint8_t  data[8];         
    
    uint8_t  crc8;            /**< Контрольная сумма кадра */
} SPI_Packet_t;

// Объявляем глобальные переменные обмена для main.c и modbus_slave.c
extern SPI_Packet_t spi_master_tx;
extern SPI_Packet_t spi_master_rx;

#endif // SHARED_DATA_H
