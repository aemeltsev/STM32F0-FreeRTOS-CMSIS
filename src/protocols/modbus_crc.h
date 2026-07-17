#ifndef MODBUS_CRC_H
#define MODBUS_CRC_H

#include <stdint.h>

/**
 * @brief Вычисляет контрольную сумму Modbus CRC16 для буфера данных
 * @param data: Указатель на массив байт
 * @param length: Количество байт для расчета
 * @return Вычисленное 16-битное значение CRC (Low byte и High byte уже на своих местах)
 */
uint16_t modbus_crc16(const uint8_t *data, uint16_t length);

#endif // MODBUS_CRC_H
