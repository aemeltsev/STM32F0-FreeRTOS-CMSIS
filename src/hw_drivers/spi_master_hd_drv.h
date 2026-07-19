#ifndef SPI_MASTER_HD_DRV_H
#define SPI_MASTER_HD_DRV_H

#include "stm32f0xx.h"
#include <stdint.h>
#include <stdbool.h>
#include "shared_data.h"

/**
 * @brief  Initializes SPI1 as a native 1-Wire Half-Duplex Master (BIDIMODE).
 *         Configures PA5 (SCK) and PA7 (MOSI/MISO) for resistor-linked sharing.
 *         PA6 (MISO) is left completely free for user GPIO.
 * @param  None
 * @retval None
 */
void spi1_master_init(void);

/**
 * @brief  Atomically writes a packet to the Slave and reads the response back
 *         over a single resistor-linked wire, dynamically toggling BIDIODE.
 * @param  pTxPacket: Pointer to the compiled source structure to transmit.
 * @param  pRxPacket: Pointer to the target structure to store incoming data.
 * @return true if the full 12-byte bidirectional transaction succeeds.
 *         false if a hardware timeout occurs on the line.
 */
bool spi1_master_exchange_packet(const SPI_Packet_t *pTxPacket, SPI_Packet_t *pRxPacket);

#endif /* SPI_MASTER_HD_DRV_H */