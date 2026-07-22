#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include <stdint.h>

/* Включаем упаковку по 1 байту для исключения скрытых зазоров (padding) */
#define PACKET_SIZE  (12U) /**< Жестко фиксированный размер кадра SPI */

/**
 * @brief Универсальная упакованная структура для пакетного обмена по SPI
 * Итоговый размер: 2 (id) + 1 (cmd) + 8 (data) + 1 (crc8) = 12 байт.
 */
#pragma pack(push, 1)
typedef struct {
    uint16_t transaction_id;  /**< ID пакета для синхронизации */
    uint8_t  cmd;             /**< Код команды / Номер регистра */
    
    // Универсальный массив полезной нагрузки (Payload) на 8 байт.
    // Сюда можно положить что угодно: float, uint32_t или четыре uint16_t.
    uint8_t  data[8];         
    
    uint8_t  crc8;            /**< Контрольная сумма кадра */
} SPI_Packet_t;
/* Возвращаем стандартное выравнивание компилятора */
#pragma pack(pop)

// Объявляем внешние переменные (память здесь НЕ выделяется)
// Доступны во всех файлах, где подключен этот заголовочник
extern SPI_Packet_t spi_master_tx;
extern SPI_Packet_t spi_master_rx;


/**
 * @brief  Calculates a fast, zero-overhead CRC-8 (Polynomial: 0x07) over a memory block.
 *         Replaces string.h dependency for bare-metal safety.
 */static inline uint8_t calculate_crc8(const uint8_t *pBuffer, uint16_t length)
 {
    uint8_t crc = 0x00;
    for (uint16_t i = 0; i < length; i++) {
        crc ^= pBuffer[i];
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x80) {
                crc = (uint8_t)((crc << 1) ^ 0x07);
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

#endif // SHARED_DATA_H
