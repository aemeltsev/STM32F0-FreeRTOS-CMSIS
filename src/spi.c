#include "spi.h"

// Отправка байта через SPI1
static uint8_t send_byte_spi1(uint8_t byte)
{
    while (!(SPI1->SR & SPI_SR_TXE)); // Wait until the transmit buffer is empty
    *(uint8_t *)&(SPI1->DR) = byte; // Send the byte
    while (!(SPI1->SR & SPI_SR_RXNE)); // Wait until the receive buffer is not empty
    return (uint8_t)SPI1->DR; // Return the received byte
}

// Отправка байта через SPI2
static uint8_t send_byte_spi2(uint8_t byte)
{
    while (!(SPI2->SR & SPI_SR_TXE)); // Ожидание, пока передающий буфер не станет пустым
    *(uint8_t *)&(SPI2->DR) = byte; // Отправка байта
    while (!(SPI2->SR & SPI_SR_RXNE)); // Ожидание, пока принимающий буфер не станет непустым
    return (uint8_t)SPI2->DR; // Возврат полученного байта
}

static uint8_t receive_byte_spi2(void)
{
    while (!(SPI2->SR & SPI_SR_RXNE)); // Wait until the receive buffer is not empty
    return (uint8_t)SPI2->DR; // Return the received byte
}

void write(uint8_t* pBuffer, uint8_t WriteAddr, uint16_t NumByteToWrite)
{
    if (NumByteToWrite > 0x01)
    {
        WriteAddr |= (uint8_t)MULTIPLEBYTE_CMD;
    }
    
    spi1_nss_low();

    send_byte_spi1(WriteAddr);
    while (NumByteToWrite >= 0x01)
    {
        send_byte_spi1(*pBuffer);
        NumByteToWrite--;
        pBuffer++;
    }
    spi1_nss_hight();
}

void read(uint8_t* pBuffer, uint8_t ReadAddr, uint16_t NumByteToRead)
{
    if (NumByteToRead > 0x01)
    {
        ReadAddr |= (uint8_t)(READWRITE_CMD | MULTIPLEBYTE_CMD);
    }
    else
    {
        ReadAddr |= (uint8_t)READWRITE_CMD;
    }

    send_byte_spi1(ReadAddr);

    while (NumByteToRead > 0x00)
    {
        *pBuffer = receive_byte_spi2();
        NumByteToRead--;
        pBuffer++;
    }
}

/**
 * Configure SPI1 gpios. SPI1 - working on GPIOA and alternate function - AF0 
 */
void spi1_gpio_init_full(void)
{
    /* Enable GPIOA clock */
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

    /* Configure PA4(NSS) output, open-drain, pull-up or output, push-pull, high speed*/
    GPIOA->MODER &= ~(GPIO_MODER_MODER4); // Reset
    GPIOA->MODER |= GPIO_MODER_MODER4_0; // General purpose output mode
    GPIOA->OTYPER &= ~(GPIO_OTYPER_OT_4); // Reset
    GPIOA->OTYPER |= GPIO_OTYPER_OT_4; // Output open-drain
    GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR4); // Reset
    GPIOA->PUPDR |= GPIO_PUPDR_PUPDR4_0; // Pin pull-up
    spi1_nss_hight();

    /* Configure 
       PA5 (SPI1_SCK) - AF, output, push-pull, high speed
       PA6 (SPI1_MISO) - AF, in, pull up/down
       PA7 (SPI1_MOSI) - AF, output, push-pull, high speed 
    */
    GPIOA->MODER &= ~(GPIO_MODER_MODER5 | GPIO_MODER_MODER6 | GPIO_MODER_MODER7); // Reset
    GPIOA->MODER |= (GPIO_MODER_MODER5_1 | GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1); // Alternate function mode

    GPIOA->AFR[0] &= ~(0xf << (5 * 4));
    GPIOA->AFR[0] |= (0x0 << (5 * 4)); // PA5 - AF0 (SPI1_SCK)

    GPIOA->AFR[0] &= ~(0xf << (6 * 4));
    GPIOA->AFR[0] |= (0x0 << (6 * 4)); // PA6 - AF0 (SPI1_MISO)

    GPIOA->AFR[0] &= ~(0xf << (7 * 4));
    GPIOA->AFR[0] |= (0x0 << (7 * 4)); // PA7 - AF0 (SPI1_MOSI)

}

/**
 * Конфигурируем SPI1 (ведущий режим).
 * BIDIMODE: 0, включение режима с одной линией данных;
 * BIDIOE: 0, направление передачи (используется при BIDIMODE=1);
 * CRCEN: 0, включение аппаратного подсчёта CRC (отключено);
 * CRCNEXT:0, бит связан с вычислением CRC, используется при CRCEN=1;
 * RXONLY: 0, включение режима "только приём" (здесь - полнодуплексная связь);
 * SSM: 1, включение режима программного управления сигналом NSS;
 * SSI: 1, при SSM=1 бит замещает значение со входа NSS (здесь - не используется);
 * LSBFIRST: 0, порядок передачи битов (здесь - первым передаётся старший);
 * SPE: 0, бит включения SPI (здесь разделяем этапы конфигурирования и включения);
 * BR[2:0]: управление скоростью передачи (не влияет, если SPI настроен как подчинённое устройство);
 * здесь задано 0x7, что соотв. макс. делителю /256 (для теста выбираем минимальную скорость);
 * MSTR: 1, бит переключения в ведущий режим.
 */
void configure_spi1_full(void)
{
    /* Enable SPI1 clock */
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    /* Configure SPI1 communication */
    SPI1->CR1 = 0; // Reset CR1
    SPI1->CR1 |= SPI_CR1_BR_0; // Baud rate: fPCLK/4
    SPI1->CR1 |= SPI_CR1_CPOL; // Clock HIGH when idle CPOL=1
    SPI1->CR1 |= SPI_CR1_CPHA; // Data capture on second clock transmission CPHA=1
    SPI1->CR1 |= SPI_CR1_SSM; // Software slave management enabled
    SPI1->CR1 |= SPI_CR1_SSI; // Internal slave select

    // Configure SPI1 CR2
    SPI1->CR2 = 0; // Reset CR2
    SPI1->CR2 |= SPI_CR2_FRF; // TI mode enabled
    SPI1->CR2 |= SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0; // 8-bit data size
    SPI1->CR2 |= SPI_CR2_FRXTH; // Set RX threshold to 8 bit

    SPI1->CR1 |= SPI_CR1_MSTR; // Master mode

    /* Configure SPI1 transfer interrupts */
    /* Enable TXE Interrupt */
    //SPI1->CR2 |= SPI_CR2_TXEIE;
    /* Enable SPI1 Error Interrupt */
    //SPI1->CR2 |= SPI_CR2_ERRIE;

    //NVIC_SetPriority(SPI1_IRQn, 1);
    //NVIC_EnableIRQ(SPI1_IRQn);
}

/**
 * SPI2 - working on GPIOB and alternate function - AF0 
 */
void spi2_gpio_init_full(void)
{
    /* Enable GPIOB clock */
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

    /* Configure 
    PB12 (SPI2_NSS) - AF, input, pull up/down
    PB13 (SPI2_SCK) - AF, input, pull up/down
    PB14 (SPI2_MISO) - AF, output, push-pull
    PB15 (SPI2_MOSI) - AF, input, pull up/down
    */
    GPIOA->MODER &= ~(GPIO_MODER_MODER12 | GPIO_MODER_MODER13 | GPIO_MODER_MODER14 | GPIO_MODER_MODER15); // Reset
    GPIOA->MODER |= (GPIO_MODER_MODER12_1 | GPIO_MODER_MODER13_1 | GPIO_MODER_MODER14_1 | GPIO_MODER_MODER15_1); // Alternate function mode

    GPIOA->AFR[1] &= ~(0xf << (4 * 4));
    GPIOA->AFR[1] |= (0x0 << (4 * 4)); // PB12 - AF1 (SPI2_NSS)

    GPIOA->AFR[1] &= ~(0xf << (5 * 4));
    GPIOA->AFR[1] |= (0x0 << (5 * 4)); // PB13 - AF1 (SPI2_SCK)

    GPIOA->AFR[1] &= ~(0xf << (6 * 4));
    GPIOA->AFR[1] |= (0x0 << (6 * 4)); // PB14 - AF1 (SPI2_MISO)

    GPIOA->AFR[1] &= ~(0xf << (7 * 4));
    GPIOA->AFR[1] |= (0x0 << (7 * 4)); // PB15 - AF1 (SPI2_MOSI)
}

/**
 * Конфигурируем SPI2 (ведомый режим).
 * BIDIMODE: 0, включение режима с одной линией данных;
 * BIDIOE: 0, направление передачи (используется при BIDIMODE=1);
 * CRCEN: 0, включение аппаратного подсчёта CRC (отключено);
 * CRCNEXT:0, бит связан с вычислением CRC, используется при CRCEN=1;
 * RXONLY: 0, включение режима "только приём" (здесь - полнодуплексная связь);
 * SSM: 0, включение режима программного управления сигналом NSS;
 * SSI: 0, при SSM=1 бит замещает значение со входа NSS (здесь - не используется);
 * LSBFIRST: 0, порядок передачи битов (здесь - первым передаётся старший);
 * SPE: 0, бит включения SPI (здесь разделяем этапы конфигурирования и включения);
 * BR[2:0]: управление скоростью передачи (не влияет, если SPI настроен как подчинённое устройство);
 * здесь задано 0x7, что соотв. макс. делителю /256 (для теста выбираем минимальную скорость);
 * MSTR: 0, бит переключения в ведущий режим.
 */
void configure_spi2_full(void)
{
    /* Enable SPI2 clock */
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;

    /* Configure SPI2 communication */
    SPI2->CR1 = 0; // Reset CR1
    SPI2->CR1 |= SPI_CR1_SPE;
    SPI2->CR1 |= SPI_CR1_CPOL; // Clock HIGH when idle CPOL=1
    SPI2->CR1 |= SPI_CR1_CPHA; // Data captured on rising edge CPHA=1

    // Configure SPI1 CR2
    SPI1->CR2 = 0; // Reset CR2
    //SPI1->CR2 |= SPI_CR2_FRF; // TI mode enabled
    SPI1->CR2 |= SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0; // 8-bit data size
    SPI1->CR2 |= SPI_CR2_FRXTH; // Set RX threshold to 8 bit
}

// SPI1 Master mode, HALF-DUPLEX
// Use Software slave select pin PA4(NSS)
// Use PA5(SCK), PA6(MISO), PA7(MOSI) - alternate function mode
void spi1_gpio_init_half(void)
{
    /* Enable GPIOA clock */
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

    /* Configure PA4(NSS) output, open-drain, pull-up or output, push-pull, high speed*/
    GPIOA->MODER &= ~(GPIO_MODER_MODER4); // Reset
    GPIOA->MODER |= GPIO_MODER_MODER4_0; // General purpose output mode
    GPIOA->OTYPER &= ~(GPIO_OTYPER_OT_4); // Reset
    GPIOA->OTYPER |= GPIO_OTYPER_OT_4; // Output open-drain
    GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR4); // Reset
    GPIOA->PUPDR |= GPIO_PUPDR_PUPDR4_0; // Pin pull-up
    spi1_nss_hight();

    /* Configure 
       PA5 (SPI1_SCK) - AF, output, push-pull, high speed
       PA6 (SPI1_MISO) - AF, in, pull up/down
       PA7 (SPI1_MOSI) - AF, output, push-pull, high speed 
    */
    GPIOA->MODER &= ~(GPIO_MODER_MODER5 | GPIO_MODER_MODER7 | GPIO_MODER_MODER6); // Reset and PA6 - input mode(00)
    GPIOA->MODER |= (GPIO_MODER_MODER5_1 | GPIO_MODER_MODER7_1); // Alternate function mode

    GPIOA->AFR[0] &= ~(0xf << (5 * 4));
    GPIOA->AFR[0] |= (0x0 << (5 * 4)); // PA5 - AF0 (SPI1_SCK)

    GPIOA->AFR[0] &= ~(0xf << (6 * 4));
    GPIOA->AFR[0] |= (0x0 << (6 * 4)); // PA6 - AF0 (SPI1_MISO)

    GPIOA->AFR[0] &= ~(0xf << (7 * 4));
    GPIOA->AFR[0] |= (0x0 << (7 * 4)); // PA7 - AF0 (SPI1_MOSI)

    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR6; // Reset
    GPIOA->PUPDR |= GPIO_PUPDR_PUPDR6_0; // PUPDR - pull up
}

/**
 * Конфигурируем SPI1 (ведущий режим в данном случае).
 * BIDIMODE: 1, включение режима с одной линией данных;
 * BIDIOE: 0, направление передачи (используется при BIDIMODE=1);
 * CRCEN: 0, включение аппаратного подсчёта CRC (отключено);
 * CRCNEXT:0, бит связан с вычислением CRC, используется при CRCEN=1;
 * RXONLY: 0, включение режима "только приём" (здесь - полнодуплексная связь);
 * SSM: 1, включение режима программного управления сигналом NSS;
 * SSI: 1, при SSM=1 бит замещает значение со входа NSS (здесь - не используется);
 * LSBFIRST: 0, порядок передачи битов (здесь - первым передаётся старший);
 * SPE: 0, бит включения SPI (здесь разделяем этапы конфигурирования и включения);
 * BR[2:0]: управление скоростью передачи (не влияет, если SPI настроен как подчинённое устройство);
 * здесь задано 0x7, что соотв. макс. делителю /256 (для теста выбираем минимальную скорость);
 * MSTR: 1, бит переключения в ведущий режим.
 */
void configure_spi1_half(void)
{
    /* Enable the peripheral clock SPI1 */
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    /* Configure SPI1 communication */
    SPI1->CR1 = 0; // Reset CR1
    SPI1->CR1 |= SPI_CR1_BR_0; // Baud rate: fPCLK/4
    //SPI1->CR1 |= SPI_CR1_LSBFIRST; // LSB first maybe better use &= ~(SPI_CR1_LSBFIRST)
    //SPI1->CR1 |= SPI_CR1_CPOL; // Clock HIGH when idle CPOL=1
    //SPI1->CR1 |= SPI_CR1_CPHA; // Data capture on second clock transmission CPHA=1
    SPI1->CR1 |= SPI_CR1_BIDIMODE; // 1-line bidirectional mode
    SPI1->CR1 |= SPI_CR1_SSM; // Software slave management enabled
    SPI1->CR1 |= SPI_CR1_SSI; // Internal slave select
    // CRC
    //SPI1->CR1 |= SPI_CR1_CRCEN; // CRC calculation enabled
    //SPI1->CR1 |= SPI_CR1_CRCL; // 8 bit CRC length
    //SPI1->CRCPR = 0x07;

    // Configure SPI1 CR2
    SPI1->CR2 = 0; // Reset CR2
    SPI1->CR2 |= SPI_CR2_FRF; // TI mode enabled
    SPI1->CR2 |= SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0; // 8-bit data size
    SPI1->CR2 |= SPI_CR2_FRXTH; // Set RX threshold to 8 bit

    SPI1->CR1 |= SPI_CR1_MSTR; // Master mode

    /* Configure SPI1 transfer interrupts */
    /* Enable TXE Interrupt */
    SPI1->CR2 |= SPI_CR2_TXEIE;
    /* Enable SPI1 Error Interrupt */
    SPI1->CR2 |= SPI_CR2_ERRIE;

    NVIC_SetPriority(SPI1_IRQn, 1);
    NVIC_EnableIRQ(SPI1_IRQn);
}

void spi1_enable(void) 
{
    SPI1->CR1 |= SPI_CR1_SPE; // Enable SPI1
}

void spi1_disable(void) 
{
    SPI1->CR1 &= ~SPI_CR1_SPE; // Disable SPI1
}

// Use SPI1 software slave select pin PA4(NSS)
void spi1_nss_hight(void)
{
    GPIOA->BSRR |= GPIO_BSRR_BS_4; // Set NSS Hight
}

// Use SPI1 software slave select pin PA4(NSS)
void spi1_nss_low(void)
{
    GPIOA->BSRR |= GPIO_BSRR_BR_4; // Set NSS Low
}

// Set SPI1 BIDIOE to 1 (output mode)
void spi1_set_bidioe_output(void)
{
    SPI1->CR1 |= SPI_CR1_BIDIOE;
}

// Set SPI1 BIDIOE to 0 (input mode)
void spi1_set_bidioe_input(void)
{
    SPI1->CR1 &= ~SPI_CR1_BIDIOE;
}

// SPI2 Slave mode, HALF-DUPLEX
// Use Software slave select pin PB12(NSS)
// Use PB13(SCK), PB14(MISO), PB15(MOSI) - alternate function mode
void spi2_gpio_init_half(void)
{
    /* Enable GPIOA clock */
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

    /* Configure 
    PB12 (SPI2_NSS) - AF, input, pull up/down
    PB13 (SPI2_SCK) - AF, input, pull up/down
    PB14 (SPI2_MISO) - AF, output, push-pull
    PB15 (SPI2_MOSI) - AF, input, pull up/down
    */
    GPIOA->MODER &= ~(GPIO_MODER_MODER12 | GPIO_MODER_MODER13 | GPIO_MODER_MODER14 | GPIO_MODER_MODER15); // Reset
    GPIOA->MODER |= (GPIO_MODER_MODER12_1 | GPIO_MODER_MODER13_1 | GPIO_MODER_MODER14_1 | GPIO_MODER_MODER15_1); // Alternate function mode

    GPIOA->AFR[1] &= ~(GPIO_AFRH_AFRH4); // PB12 - AF1 (SPI2_NSS)
    //GPIOA->AFR[1] &= ~(0xf << (4 * 4));
    //GPIOA->AFR[1] |= (0x0 << (4 * 4)); // PB12 - AF1 (SPI2_NSS)

    GPIOA->AFR[1] &= ~(GPIO_AFRH_AFRH5); // PB13 - AF1 (SPI2_SCK)
    //GPIOA->AFR[1] &= ~(0xf << (5 * 4));
    //GPIOA->AFR[1] |= (0x0 << (5 * 4)); // PB13 - AF1 (SPI2_SCK)

    GPIOA->AFR[1] &= ~(GPIO_AFRH_AFRH6); // PB14 - AF1 (SPI2_MISO)
    //GPIOA->AFR[1] &= ~(0xf << (6 * 4));
    //GPIOA->AFR[1] |= (0x0 << (6 * 4)); // PB14 - AF1 (SPI2_MISO)

    GPIOA->AFR[1] &= ~(GPIO_AFRH_AFRH7); // PB15 - AF1 (SPI2_MOSI)
    //GPIOA->AFR[1] &= ~(0xf << (7 * 4));
    //GPIOA->AFR[1] |= (0x0 << (7 * 4)); // PB15 - AF1 (SPI2_MOSI)
}

/**
 * Конфигурируем SPI2 (ведомый режим, одна двунаправленная линия).
 * BIDIMODE: 1, включение режима с одной линией данных;
 * BIDIOE: 0, прием по умолчанию (используется при BIDIMODE=1);
 * CRCEN: 0, включение аппаратного подсчёта CRC (отключено);
 * CRCNEXT:0, бит связан с вычислением CRC, используется при CRCEN=1;
 * RXONLY: 0, включение режима "только приём" (здесь - двунаправленная линия);
 * SSM: 0, управления сигналом NSS аппаратное;
 * SSI: 0, при SSM=1 бит замещает значение со входа NSS (здесь - не используется);
 * LSBFIRST: 0, порядок передачи битов (здесь - первым передаётся старший);
 * SPE: 0, бит включения SPI (здесь разделяем этапы конфигурирования и включения);
 * BR[2:0]: управление скоростью передачи (не влияет, если SPI настроен как подчинённое устройство);
 * здесь задано 0x7, что соотв. макс. делителю /256 (для теста выбираем минимальную скорость);
 * MSTR: 0, ведомый режим.
*/
void configure_spi2_half(void)
{
    /* Enable the peripheral clock SPI2 */
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;

    /* Configure SPI2 in slave */
    /* nSS hard, slave, CPOL and CPHA at zero (rising first edge) */
    /* (1) RXNE IT, 8-bit Rx fifo */
    /* (2) Enable SPI2 */
    SPI2->CR1 = 0; // Reset CR1
    //SPI2->CR1 |= SPI_CR1_BR_0; // Baud rate: fPCLK/4
    //SPI2->CR1 |= SPI_CR1_LSBFIRST; // LSB first maybe better use &= ~(SPI_CR1_LSBFIRST)
    //SPI2->CR1 |= SPI_CR1_CPOL; // Clock HIGH when idle CPOL=1
    //SPI2->CR1 |= SPI_CR1_CPHA; // Data capture on second clock transmission CPHA=1
    SPI2->CR1 |= SPI_CR1_BIDIMODE; // 1-line bidirectional mode
    //SPI2->CR1 |= SPI_CR1_SSM; // Software slave management enabled
    //SPI2->CR1 |= SPI_CR1_SSI; // Internal slave select

    // CRC
    //SPI2->CR1 |= SPI_CR1_CRCEN; // CRC calculation enabled
    //SPI2->CR1 |= SPI_CR1_CRCL; // 8 bit CRC length
    //SPI2->CRCPR = 0x07;

    // Configure SPI2 CR2
    SPI2->CR2 = 0; // Reset CR2
    SPI2->CR2 |= SPI_CR2_FRF; // TI mode enabled
    SPI2->CR2 |= SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0; // 8-bit data size
    SPI2->CR2 |= SPI_CR2_FRXTH; // Set RX threshold to 8 bit

    //SPI2->CR1 &= ~SPI_CR1_MSTR; // Slave mode

    /* Configure SPI2 transfer interrupts */
    /* Enable RXNE Interrupt */
    SPI2->CR2 |= SPI_CR2_RXNEIE;
        /* Enable SPI1 Error Interrupt */
    SPI2->CR2 |= SPI_CR2_ERRIE;

    NVIC_SetPriority(SPI2_IRQn, 1);
    NVIC_EnableIRQ(SPI2_IRQn);
}

/**
 * 
 */
void spi2_enable(void) 
{
    SPI2->CR1 |= SPI_CR1_SPE; // Enable SPI2
}

void spi2_disable(void) 
{
    SPI2->CR1 &= ~SPI_CR1_SPE; // Disable SPI2
}

/**
  * @brief  This function handles SPI1 interrupt request.
  * @param  None
  * @retval None
  */
//void SPI1_IRQHandler(void)
//{
//  /* Check TXE flag value in SR register */
//  if (SPI1->SR & SPI_SR_TXE)
//  {
//    /* Call function Slave Transmission Callback */
//    SPI1_Tx_Callback();
//  }
//}

/**
  * @brief  This function handles SPI2 interrupt request.
  * @param  None
  * @retval None
  */
//void SPI2_IRQHandler(void)
//{
//  /* Check RXNE flag value in SR register */
//  if (SPI2->SR & SPI_SR_RXNE)
//  {
//    /* Call function Slave Reception Callback */
//    SPI3_Rx_Callback();
//  }
//  /* Check OVR flag value in SR register */
//  else if (SPI2->SR & SPI_SR_OVR)
//  {
//    /* Call Error function */
//    SPI_TransferError_Callback();
//  }
//}
/*
void SPI1_Tx_Callback(void)
{
    if (Tx_Idx < sizeof(TxBuffer)) {
        SPI1_NSS_Low();
        SPI1_Set_BIDIOE_Output();
        SPI1->DR = TxBuffer[Tx_Idx++];
    } else {
        SPI1->CR2 &= ~SPI_CR2_TXEIE;
        SPI1_Set_BIDIOE_Input();
        SPI1_NSS_Low();
    }    
}

void SPI2_Rx_Callback(void)
{

}

void SPI1_Transmit(uint8_t data)
{
    // Wait until TX buffer empty
    while (!(SPI1->SR & SPI_SR_TXE));
    // Or Wait for TX buffer to not be full
    //while ((SPI1->SR & SPI_SR_FTLVL) == SPI_SR_FTLVL);    
    
    SPI1_NSS_Low();
    SPI1_Set_BIDIOE_Output();
    // Write data to transmit register
    SPI1->DR = data; 

    // Wait for BSY flag to be cleared
    while (SPI1->SR & SPI_SR_BSY) {}

    SPI1_NSS_Hight();
}

uint8_t SPI1_Recieve(void)
{
    SPI1_Set_BIDIOE_Input();
    // Wait for RXNE flag to be set
    while (!(SPI1->SR & SPI_SR_RXNE)) {}

    SPI1_NSS_Low();
    uint8_t data = SPI1->DR;

    SPI1_NSS_Hight();
    return data;
}
*/
