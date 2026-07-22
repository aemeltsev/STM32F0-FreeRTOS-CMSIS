
#include "spi_master_hd_drv.h"

void spi1_master_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    // 1. Configure GPIO in system_init.c

    // 2. Configure SPI (Bidirectional Half-Duplex Master, f_PCLK/16)
    // SPI_CR1_BIDIMODE = Enable 1-wire bidirectional mode
    // SPI_CR1_BIDIOE  = Set direction to Output (Transmit) by default
    SPI1->CR1 = SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI | 
                SPI_CR1_BR_1 | SPI_CR1_BR_0 | 
                SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE;
                
    SPI1->CR2 = SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2 | SPI_CR2_FRXTH; // 8-bit frame
    SPI1->CR1 |= SPI_CR1_SPE;
}


/**
 * @brief Atomically writes a packet and reads the response over a resistor-linked 1-Wire line.
 * @param pTxPacket: Pointer to the source structure to send.
 * @param pRxPacket: Pointer to the target structure to populate.
 * @return true if the exchange succeeds, false if a hardware timeout occurs.
 */
bool spi1_master_exchange_packet(const SPI_Packet_t *pTxPacket, SPI_Packet_t *pRxPacket)
{
    const uint8_t *pTx = (const uint8_t *)pTxPacket;
    uint8_t       *pRx = (uint8_t *)pRxPacket;
    uint16_t      timeout_guard;

    // Ensure any leftover junk in the RX FIFO is cleared
    while (SPI1->SR & SPI_SR_RXNE) {
        volatile uint8_t clear_dummy = *(__IO uint8_t *)&SPI1->DR;
        (void)clear_dummy;
    }

    // =========================================================================
    // PHASE 1: TRANSMIT PACKET TO SLAVE (Master Output Enabled)
    // =========================================================================
    SPI1->CR1 |= SPI_CR1_BIDIOE; // Ensure internal transmitter is driving the line

    for (uint16_t i = 0; i < PACKET_SIZE; i++)
    {
        timeout_guard = 0xFFFFU;
        while (!(SPI1->SR & SPI_SR_TXE)) {
            if (--timeout_guard == 0) return false;
        }
        
        // Output raw byte. In BIDIMODE + BIDIODE, this shifts out without pushing anything to RX FIFO.
        *(__IO uint8_t *)&SPI1->DR = pTx[i];
    }

    // Wait until the shift register is completely empty before changing line direction
    timeout_guard = 0xFFFFU;
    while (SPI1->SR & SPI_SR_BSY) {
        if (--timeout_guard == 0) return false;
    }

    // =========================================================================
    // PHASE 2: RECEIVE PACKET FROM SLAVE (Master Output Disabled)
    // =========================================================================
    SPI1->CR1 &= ~SPI_CR1_BIDIOE; // Turn off Master's transmitter (MOSI goes High-Z input)

    // Clear the RX FIFO one more time right before reading to ensure no transmission noise slipped in
    while (SPI1->SR & SPI_SR_RXNE) {
        volatile uint8_t clear_dummy = *(__IO uint8_t *)&SPI1->DR;
        (void)clear_dummy;
    }

    for (uint16_t i = 0; i < PACKET_SIZE; i++)
    {
        // CRITICAL STM32 BIDIMODE BEHAVIOR:
        // To read a byte in Master Receive-Only mode, you must write a dummy byte to SPI1->DR.
        // This triggers the hardware master state machine to generate exactly 8 clock pulses.
        timeout_guard = 0xFFFFU;
        while (!(SPI1->SR & SPI_SR_TXE)) {
            if (--timeout_guard == 0) return false;
        }

        *(__IO uint8_t *)&SPI1->DR = 0x00; // Trigger 8 SCK pulses

        // Wait for the Slave's response byte to arrive in our RX FIFO
        timeout_guard = 0xFFFFU;
        while (!(SPI1->SR & SPI_SR_RXNE)) {
            if (--timeout_guard == 0) return false;
        }

        pRx[i] = *(__IO uint8_t *)&SPI1->DR;
    }

    // Wait for the final clock pulses to finish before returning to the application
    timeout_guard = 0xFFFFU;
    while (SPI1->SR & SPI_SR_BSY) {
        if (--timeout_guard == 0) return false;
    }

    // Restore the Master to Transmit mode so it sits in a safe state until the next Modbus frame
    SPI1->CR1 |= SPI_CR1_BIDIOE; 

    return true;
}


/**
#include "stm32f0xx.h"
#include "spi_master_hd.h"
#include "shared_data.h"

volatile uint32_t current_ticks = 0;

void SysTick_Handler(void) {
    current_ticks++;
}

int main(void) {
    // 1. Core clock initialization (Standard 1ms heartbeat setup)
    SysTick_Config(SystemCoreClock / 1000);
    
    // 2. Hardware peripheral setups
    SPI1_Master_Init();
    
    // Configure PA4 as Push-Pull Output for Software Gated Slave Select (CS)
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER &= ~GPIO_MODER_MODER4;
    GPIOA->MODER |= GPIO_MODER_MODER4_0;
    GPIOA->BSRR = GPIO_BSRR_BS_4; // Set CS HIGH (Idle state)

    // Configure PC13 as Status LED Output (Turns ON during validation faults)
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    GPIOC->MODER &= ~GPIO_MODER_MODER13;
    GPIOC->MODER |= GPIO_MODER_MODER13_0;
    GPIOC->BSRR = GPIO_BSRR_BR_13; // LED OFF

    uint32_t last_transmission = 0;

    while (1) {
        if ((current_ticks - last_transmission) >= 100) {
            last_transmission = current_ticks;

            // Step A: Pack structural variables cleanly into Outbound Payload
            spi_master_tx.transaction_id = (uint16_t)(current_ticks & 0xFFFFU);
            spi_master_tx.cmd            = 0x03; // Simple Modbus-aligned READ command offset
            
            for (uint8_t i = 0; i < 8; i++) {
                spi_master_tx.data[i] = (uint8_t)(i + 10); // Dummy values
            }
            
            // Append mathematical CRC calculation signature
            spi_master_tx.crc8 = Calculate_CRC8((uint8_t*)&spi_master_tx, PACKET_SIZE - 1);

            // Step B: Assert Chip Select line (CS -> LOW)
            GPIOA->BSRR = GPIO_BSRR_BR_4; 
            
            // Step C: Trigger 1-Wire Half-Duplex SPI Exchange Engine
            bool success = SPI1_Master_Exchange_Packet(&spi_master_tx, &spi_master_rx);
            
            // Step D: De-assert Chip Select line (CS -> HIGH)
            GPIOA->BSRR = GPIO_BSRR_BS_4;

            // Step E: Assess integrity metrics
            if (success) {
                uint8_t calculated_crc = Calculate_CRC8((uint8_t*)&spi_master_rx, PACKET_SIZE - 1);
                
                if ((spi_master_rx.crc8 == calculated_crc) && 
                    (spi_master_rx.transaction_id == spi_master_tx.transaction_id)) {
                    // Frame valid! Unpack your sensors here...
                    GPIOC->BSRR = GPIO_BSRR_BR_13; // Turn OFF Error LED
                } else {
                    // CRC or transaction ID validation fault
                    GPIOC->BSRR = GPIO_BSRR_BS_13; // Turn ON Error LED
                }
            } else {
                // Hardware pipeline timeout
                GPIOC->BSRR = GPIO_BSRR_BS_13; // Turn ON Error LED
            }
        }
    }
}
*/