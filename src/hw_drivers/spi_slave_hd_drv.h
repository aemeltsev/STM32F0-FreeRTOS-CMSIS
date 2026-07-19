#ifndef SPI_SLAVE_HD_DRV_H
#define SPI_SLAVE_HD_DRV_H

#include "stm32f0xx.h"
#include "shared_data.h"

void SPI1_Slave_Init(void);
/**
 * @brief Integrated EXTI Interrupt Handler managing the Chip Select state changes
 */
void EXTI4_15_IRQHandler(void);

#endif // SPI_SLAVE_HD_DRV_H