#ifndef MODBUS_SLAVE_H
#define MODBUS_SLAVE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ========================================================================== */
/*                      MODBUS NODE CONFIGURATION                             */
/* ========================================================================== */

#define MB_SLAVE_ADDR         0x11  /**< Device Address (17 Decimal = 0x11 HEX) */
#define MB_BUF_SIZE           256   /**< Network Transaction Buffer Boundary size */

/* ========================================================================== */
/*                ADDRESS SPACE VOLUMES (DATA TABLE SIZES)                    */
/* ========================================================================== */

#define MB_COILS_SIZE         16    /**< Read/Write Bits (FC: 0x01, 0x05) */
#define MB_DISCRETE_SIZE      16    /**< Read-Only Bits (FC: 0x02) */
#define MB_INPUT_REGS_SIZE    4     /**< Read-Only 16-Bit Words (FC: 0x04) */
#define MB_HOLDING_REGS_SIZE  4     /**< Read/Write 16-Bit Words (FC: 0x03, 0x06) */

/* ========================================================================== */
/*              STANDARD MODBUS EXCEPTION CODES (ERROR PACKETS)               */
/* ========================================================================== */

#define MB_ERR_ILLEGAL_FUNC   0x01  /**< Function code not supported by this node */
#define MB_ERR_ILLEGAL_ADDR   0x02  /**< Target Register address falls out of bounds */
#define MB_ERR_ILLEGAL_VAL    0x03  /**< Written value is structured or scaled incorrectly */

/* ========================================================================== */
/*                    APPLICATION LAYER GLOBAL REGISTERS                      */
/* ========================================================================== */

extern uint8_t  modbus_coils[MB_COILS_SIZE];
extern uint8_t  modbus_discrete_inputs[MB_DISCRETE_SIZE];
extern uint16_t modbus_input_registers[MB_INPUT_REGS_SIZE];
extern uint16_t modbus_holding_registers[MB_HOLDING_REGS_SIZE];

/* ========================================================================== */
/*                          CORE MODULE INTERFACE                             */
/* ========================================================================== */

/**
 * @brief  Initializes buffers, clears states, and starts underlying USART2/TIM3 hardware.
 */
void Modbus_Init(void);

/**
 * @brief  Background worker thread. Continuously polled inside main's while(1) loop.
 *         Parses incoming network frames, executes commands, and relays responses.
 */
void Modbus_Process(void);

/**
 * @brief  Hardware RX data pipe hook. Called inside USART2_IRQHandler for every incoming byte.
 * @param  received_byte: Raw byte pulled from the hardware receiver data register (RDR).
 */
void Modbus_RxISR(uint8_t received_byte);

/**
 * @brief  Hardware Frame Delimiter hook. Called inside TIM3_IRQHandler when T3.5 expires.
 *         Locks the current buffer and informs the background worker that a packet is ready.
 */
void Modbus_TimeoutISR(void);

#endif //MODBUS_SLAVE_H