#include "spi_slave_hd_drv.h"

SPI_Packet_t slave_tx_telemetry;
SPI_Packet_t slave_rx_command;

void SPI1_Slave_Init(void) 
{
    RCC->AHBENR  |= RCC_AHBENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    // 1. PA5 (SCK) and PA7 (MOSI/MISO linked via resistor) -> Alternate Function AF0
    GPIOA->MODER &= ~(GPIO_MODER_MODER5 | GPIO_MODER_MODER7);
    GPIOA->MODER |= GPIO_MODER_MODER5_1 | GPIO_MODER_MODER7_1;

    // 2. PA4 (CS) configured as a Digital Input with an Internal Pull-Up Resistor
    GPIOA->MODER &= ~GPIO_MODER_MODER4;
    GPIOA->PUPDR |= GPIO_PUPDR_PUPDR4_0;

    // 3. Configure SPI1 in native Bidirectional Half-Duplex Slave Mode
    // Default to Output Drive Disabled (BIDIODE = 0) -> Strictly listening
    SPI1->CR1 = SPI_CR1_BIDIMODE | SPI_CR1_SSM | SPI_CR1_SSI; 
    SPI1->CR2 = SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2 | SPI_CR2_FRXTH; // 8-bit frame
    SPI1->CR1 |= SPI_CR1_SPE;

    // 4. Connect EXTI Line 4 Interrupt Engine directly onto Pin PA4 (CS Line monitoring)
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGCOMPEN;
    SYSCFG->EXTICR[1] &= ~SYSCFG_EXTICR2_EXTI4; // Map EXTI4 onto Port A
    
    EXTI->IMR  |= EXTI_IMR_IM4;   // Unmask EXTI4 line interrupt requests
    EXTI->FTSR |= EXTI_FTSR_TR4; // Catch Falling Edge transitions (CS going LOW)
    EXTI->RTSR |= EXTI_RTSR_TR4; // Catch Rising Edge transitions (CS going HIGH)

    NVIC_EnableIRQ(EXTI4_15_IRQn);
    NVIC_SetPriority(EXTI4_15_IRQn, 0); // Assign top priority to prevent data corruption
}

/**
 * @brief Integrated EXTI Interrupt Handler managing the Chip Select state changes
 */
void EXTI4_15_IRQHandler(void) 
{
    if (EXTI->PR & EXTI_PR_PR4) {
        EXTI->PR = EXTI_PR_PR4; // Acknowledge and clear the interrupt flag

        // === SCENARIO A: CS DRIVES LOW (Master claims bus / Start Transaction) ===
        if (!(GPIOA->IDR & GPIO_IDR_4)) {
            // Update outbound variables inside a clean memory layout wrapper
            slave_tx_telemetry.transaction_id = slave_rx_command.transaction_id; // Mirror master's tag
            slave_tx_telemetry.cmd            = 0x03; // Confirm read execution action
            
            // Populate actual sensor payload register positions
            slave_tx_telemetry.data[0] = 0x55; // Example byte 1
            slave_tx_telemetry.data[1] = 0x66; // Example byte 2
            for (uint8_t i = 2; i < 8; i++) {
                slave_tx_telemetry.data[i] = 0x00;
            }
            
            slave_tx_telemetry.crc8 = calculate_crc8((uint8_t*)&slave_tx_telemetry, PACKET_SIZE - 1);

            // Pivot peripheral state machine into Transmit drive mode (BIDIODE = 1)
            SPI1->CR1 |= SPI_CR1_BIDIOE; 

            // Push the structure out to the hardware TX FIFO buffer
            uint8_t *pBuf = (uint8_t*)&slave_tx_telemetry;
            for (uint16_t i = 0; i < PACKET_SIZE; i++) {
                while (!(SPI1->SR & SPI_SR_TXE));
                *(__IO uint8_t *)&SPI1->DR = pBuf[i];
            }
        }
        // === SCENARIO B: CS DRIVES HIGH (Master frees bus / Close Transaction) ===
        else {
            // Read out the incoming commands sent by the master from our hardware RX FIFO
            uint8_t *pRxBuf = (uint8_t*)&slave_rx_command;
            uint16_t rx_bytes = 0;

            while (SPI1->SR & SPI_SR_RXNE) {
                if (rx_bytes < PACKET_SIZE) {
                    pRxBuf[rx_bytes++] = *(__IO uint8_t *)&SPI1->DR;
                } else {
                    volatile uint8_t spill = *(__IO uint8_t *)&SPI1->DR;
                    (void)spill; // Discard overflow noise bytes cleanly
                }
            }

            // Confirm all data leaves the shift register before changing line direction
            while (SPI1->SR & SPI_SR_BSY);
            
            // Turn OFF Slave transmitter output. Pin returns to a High-Z listening state.
            SPI1->CR1 &= ~SPI_CR1_BIDIOE; 
            
            if (rx_bytes == PACKET_SIZE) {
                uint8_t calc_crc = calculate_crc8((uint8_t*)&slave_rx_command, PACKET_SIZE - 1);
                if (slave_rx_command.crc8 == calc_crc) {
                    // Valid command structure received from Master!
                    // Execute actions safely inside the main application loop.
                }
            }
        }
    }
}

/*
int main(void) {
    // Initialize standard slave interfaces
    SPI1_Slave_Init();

    while (1) {
        // The main application loop remains completely unblocked.
        // It can handle background updates or low-power sleep entry.
        __WFI(); 
    }
}
*/