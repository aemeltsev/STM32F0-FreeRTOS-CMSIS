#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include "stm32f0xx.h"

#define SPI_BUF_SIZE 64

#define SPI2_DATA (0xDE)
#define SPI1_DATA (0xCA)
#define WHO_AM_I 0x75

/* Read/Write command */
#define READWRITE_CMD              ((uint8_t)0x80)
/* Multiple byte read/write command */
#define MULTIPLEBYTE_CMD           ((uint8_t)0x40)
/* Dummy Byte Send by the SPI Master device in order to generate the Clock to the Slave device */
#define DUMMY_BYTE                 ((uint8_t)0x00)

static uint8_t send_byte_spi1(uint8_t byte);
static uint8_t receive_byte_spi2(void);
void write(uint8_t* pBuffer, uint8_t WriteAddr, uint16_t NumByteToWrite);
void read(uint8_t* pBuffer, uint8_t ReadAddr, uint16_t NumByteToRead);

static uint8_t spi1_tx_buffer[SPI_BUF_SIZE];
static uint8_t spi2_rx_buffer[SPI_BUF_SIZE];

void spi1_gpio_init_full(void);
void configure_spi1_full(void);
void spi2_gpio_init_full(void);
void configure_spi2_full(void);

void spi1_gpio_init_half(void);
void configure_spi1_half(void);

void spi1_enable(void);
void spi1_disable(void);

void spi1_nss_hight(void);
void spi1_nss_low(void);

void spi1_set_bidioe_output(void);
void spi1_set_bidioe_input(void);

void spi2_gpio_init_half(void);
void configure_spi2_half(void);

void spi2_enable(void);
void spi2_disable(void);

//void SPI1_Tx_Callback(void);
//void SPI2_Rx_Callback(void);
//void SPI_TransferError_Callback(void);
//void Delay(uint32_t nCount);

//void SPI1_Transmit(uint8_t data);
//uint8_t SPI1_Recieve(void);

#endif // SPI_H