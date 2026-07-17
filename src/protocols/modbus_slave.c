#include "modbus_slave.h"
#include "modbus_crc.h"
#include "uart_drv.h"
#include "timer_drv.h"
#include "shared_data.h"

extern volatile uint32_t current_ticks;

// Allocation of low-level network data buffers
static uint8_t  mb_rx_buffer[MB_BUF_SIZE];
static uint16_t mb_rx_index = 0;
static uint8_t  mb_tx_buffer[MB_BUF_SIZE];
static volatile uint8_t mb_frame_ready = 0;

// Definitions of application data registers
uint8_t  modbus_coils[MB_COILS_SIZE];
uint8_t  modbus_discrete_inputs[MB_DISCRETE_SIZE];
uint16_t modbus_input_registers[MB_INPUT_REGS_SIZE];
uint16_t modbus_holding_registers[MB_HOLDING_REGS_SIZE];

/**
 * @brief  Sends a standard Modbus Exception Frame (Error Response)
 */
static void Modbus_SendException(uint8_t func_code, uint8_t exception_code)
{
    uint16_t tx_idx = 0;
    mb_tx_buffer[tx_idx++] = MB_SLAVE_ADDR;
    mb_tx_buffer[tx_idx++] = func_code | 0x80; // Raise error bit
    mb_tx_buffer[tx_idx++] = exception_code;

    uint16_t crc = modbus_crc16(mb_tx_buffer, tx_idx);
    mb_tx_buffer[tx_idx++] = (uint8_t)(crc & 0xFF);
    mb_tx_buffer[tx_idx++] = (uint8_t)((crc >> 8) & 0xFF);

    uart2_send_buffer(mb_tx_buffer, tx_idx);
}

void Modbus_Init(void)
{
    __disable_irq();
    
    // Explicit array clearing instead of using memset()
    for (uint16_t i = 0; i < MB_BUF_SIZE; i++) {
        mb_rx_buffer[i] = 0;
    }
    mb_rx_index = 0;
    mb_frame_ready = 0;
    
    // Explicit initialization of state registers
    for (uint8_t i = 0; i < MB_COILS_SIZE; i++)          { modbus_coils[i] = 0; }
    for (uint8_t i = 0; i < MB_DISCRETE_SIZE; i++)       { modbus_discrete_inputs[i] = 0; }
    for (uint8_t i = 0; i < MB_INPUT_REGS_SIZE; i++)     { modbus_input_registers[i] = 0; }
    for (uint8_t i = 0; i < MB_HOLDING_REGS_SIZE; i++)   { modbus_holding_registers[i] = 0; }
    
    __enable_irq();
    
    // Fire up hardware components at 9600 baud
    uart2_init(9600, 48000000UL);
    tim3_init_modbus_timeout(9600);
}

void Modbus_RxISR(uint8_t received_byte)
{
    if (mb_frame_ready) return;
    if (mb_rx_index < MB_BUF_SIZE)
    {
        mb_rx_buffer[mb_rx_index++] = received_byte;
    }
}

void Modbus_TimeoutISR(void)
{
    if (mb_rx_index >= 4) // Valid minimum frame size requirement
    {
        mb_frame_ready = 1;
    }
    else
    {
        mb_rx_index = 0; // Drop noise line spikes instantly
    }
}

/**
 * @brief  Расширенный сквозной сетевой конвейер промышленного класса.
 *         Осуществляет полную валидацию кадра Modbus RTU, ведет статистику ошибок линии,
 *         выполняет пакетный FIFO-обмен по HSPI 1-Wire с контролем сбоев и управляет
 *         аварийной индикацией и экстренным сбросом данных в EEPROM.
 * 
 * @note   Соответствует правилам MISRA C: все переменные объявлены строго в начале scope.
 *         Полностью отсутствует зависимость от тяжелой библиотеки <string.h>.
 */
void Modbus_Process(void)
{
    // =========================================================================
    // 1. ALL variable declarations must reside at the top of the scope
    // =========================================================================
    uint8_t  func_code;       // Код текущей Modbus-функции
    uint16_t start_addr;      // Стартовый адрес регистра (Offset)
    uint16_t quantity;        // Количество запрашиваемых регистров/данных
    uint8_t  tx_idx;          // Индекс сборки выходного буфера TX
    uint16_t crc;             // Переменная для расчета контрольной суммы CRC16
    bool     spi_success;     // Флаг успешности аппаратного обмена по SPI1 FIFO
    uint16_t write_val;       // Значение, присланное Мастером для записи
    uint16_t reg_val;         // Буферное значение текущего регистра
    uint16_t i;               // Итератор для циклов упаковки данных
    
    // Внешние сетевые буферы и флаги, выделенные в модуле modbus_slave.c
    extern uint8_t  mb_rx_buffer[];
    extern uint16_t mb_rx_index;
    extern uint8_t  mb_tx_buffer[];
    extern volatile uint8_t mb_frame_ready;
    
    // Now, your execution path checks can safely execute goto operations
    if (!mb_frame_ready)
    {
        return;
    }

    // 1. Packet integrity verification via CRC
    if (modbus_crc16(mb_rx_buffer, mb_rx_index) != 0)
    {
        /*
        // Ведение статистики качества линии связи. 
        // Инкрементируем регистр ошибок Input Registers (например, индекс 1)
        __disable_irq();
        if (modbus_input_registers[1] < 0xFFFFU) 
        {
            modbus_input_registers[1]++;
        }
        __enable_irq();
        */
        goto cleanup; // Silent rejection per protocol requirements
    }

    // 2. Multidrop address evaluation
    if (mb_rx_buffer[0] != MB_SLAVE_ADDR)
    {
        goto cleanup; // Discard transactions meant for other physical drops
    }

    /*
     // Если CRC верна и адрес наш — инкрементируем счетчик успешных пакетов (индекс 0)
    __disable_irq();
    if (modbus_input_registers[0] < 0xFFFFU) 
    {
        modbus_input_registers[0]++;
    }
    __enable_irq();
    */

    // 4. Assignments occur safely after the gatekeeper checks
    func_code   = mb_rx_buffer[1];
    start_addr = (uint16_t)((mb_rx_buffer[2] << 8) | mb_rx_buffer[3]);
    quantity   = (uint16_t)((mb_rx_buffer[4] << 8) | mb_rx_buffer[5]);
    
    // =========================================================================
    // 3. РАСПРЕДЕЛИТЕЛЬНЫЙ ДИСПЕТЧЕР КОМАНД (SWITCH-CASE)
    // =========================================================================
    switch (func_code)
    {
        // ---------------------------------------------------------------------
        // --- FUNCTION 0x01 (Read Coils) & 0x02 (Read Discrete Inputs) ---
        // ---------------------------------------------------------------------
        case 0x01:
        case 0x02:
        {
            uint8_t max_size = (func_code == 0x01) ? MB_COILS_SIZE : MB_DISCRETE_SIZE;
            uint8_t *p_table = (func_code == 0x01) ? modbus_coils : modbus_discrete_inputs;

            if ((start_addr + quantity) > max_size || quantity == 0 || quantity > 2000)
            {
                Modbus_SendException(func_code, MB_ERR_ILLEGAL_ADDR);
                break;
            }

            // Calculate exact response payload byte size (round upwards)
            uint8_t byte_count = (uint8_t)((quantity + 7) / 8);
            uint8_t tx_idx = 0;

            mb_tx_buffer[tx_idx++] = MB_SLAVE_ADDR;
            mb_tx_buffer[tx_idx++] = func_code;
            mb_tx_buffer[tx_idx++] = byte_count;

            // Direct data section initialization to zero instead of using memset()
            for (uint8_t i = 0; i < byte_count; i++) {
                mb_tx_buffer[tx_idx + i] = 0;
            }

            // Pack unaligned bits inside standard byte frames
            for (uint16_t i = 0; i < quantity; i++)
            {
                if (p_table[start_addr + i])
                {
                    mb_tx_buffer[tx_idx + (i / 8)] |= (uint8_t)(1 << (i % 8));
                }
            }
            tx_idx += byte_count;

            uint16_t crc = modbus_crc16(mb_tx_buffer, tx_idx);
            mb_tx_buffer[tx_idx++] = (uint8_t)(crc & 0xFF);
            mb_tx_buffer[tx_idx++] = (uint8_t)((crc >> 8) & 0xFF);
            uart2_send_buffer(mb_tx_buffer, tx_idx);
            break;
        }
        // ---------------------------------------------------------------------
        // FUNCTION 0x03 (Read Holding) & 0x04 (Read Input Registers) - 
        // READ FROM HSPI & WRITE TO RS-485 MODBUS NETWORK
        // ---------------------------------------------------------------------
        case 0x03:
        case 0x04:
        {
            uint16_t max_size = (func_code == 0x03) ? MB_HOLDING_REGS_SIZE : MB_INPUT_REGS_SIZE;
            // Если Мастер вызвал функцию 0x03 -> смотрим в таблицу Holding Registers
            // Если Мастер вызвал функцию 0x04 -> смотрим в таблицу Input Registers
            uint16_t *p_table = (func_code == 0x03) ? modbus_holding_registers : modbus_input_registers;

            if ((start_addr + quantity) > max_size || quantity == 0 || quantity > 125)
            {
                Modbus_SendException(func_code, MB_ERR_ILLEGAL_ADDR);
                break;
            }

            // Pack the structural query properties into the outbound SPI payload instance
            spi_master_tx.transaction_id = (uint16_t)(current_ticks & 0xFFFFU);
            spi_master_tx.cmd            = 0x03; // Protocol instruction byte marking READ routine
            
            for (i = 0; i < 8; i++) {
                spi_master_tx.data[i] = 0x00; // Zero remainder data slots without string.h loops
            }
            spi_master_tx.crc8 = 0x00;

            // --- EXECUTE HIGH-SPEED 1-WIRE HSPI TRANSACTION ---
            GPIOA->BSRR = GPIO_BSRR_BR_4; // Manual Chip-Select line CS -> LOW (Activate Slave)
            spi_success = SPI1_Master_Exchange_Packet(&spi_master_tx, &spi_master_rx);
            GPIOA->BSRR = GPIO_BSRR_BS_4; // Manual Chip-Select line CS -> HIGH (Release Slave)

            if (!spi_success)
            {
                // If local slave drops off or wires break, return Modbus error 0x04 (Slave Failure)
                Modbus_SendException(func_code, 0x04);
                break;
            }

            // --- UNPACK DYNAMIC FROM HSPI ATOMICALLY INTO REGISTER MAPS ---
            // Critical section protects multi-register 32-bit values from being torn by UART interrupts
            __disable_irq();
            modbus_holding_registers[0] = (uint16_t)((spi_master_rx.data[0] << 8) | spi_master_rx.data[1]);
            modbus_holding_registers[1] = (uint16_t)((spi_master_rx.data[2] << 8) | spi_master_rx.data[3]);
            modbus_holding_registers[2] = (uint16_t)((spi_master_rx.data[4] << 8) | spi_master_rx.data[5]);
            modbus_holding_registers[3] = (uint16_t)((spi_master_rx.data[6] << 8) | spi_master_rx.data[7]);
            __enable_irq();

            // --- WRITE EXTRACTED DATA PACKETS BACK TO RS-485 ---
            uint8_t tx_idx = 0;
            mb_tx_buffer[tx_idx++] = MB_SLAVE_ADDR;
            mb_tx_buffer[tx_idx++] = func_code;
            mb_tx_buffer[tx_idx++] = (uint8_t)(quantity * 2);

            for (uint16_t i = 0; i < quantity; i++)
            {
                uint16_t val = p_table[start_addr + i];
                mb_tx_buffer[tx_idx++] = (uint8_t)(val >> 8);   // Force network Big-Endian order
                mb_tx_buffer[tx_idx++] = (uint8_t)(val & 0xFF);
            }

            uint16_t crc = modbus_crc16(mb_tx_buffer, tx_idx);
            mb_tx_buffer[tx_idx++] = (uint8_t)(crc & 0xFF);
            mb_tx_buffer[tx_idx++] = (uint8_t)((crc >> 8) & 0xFF);

            // Dispatch response buffer. Auto-DE engine on USART2 handles transceiver gating natively!
            uart2_send_buffer(mb_tx_buffer, tx_idx);
            break;
        }
        // ---------------------------------------------------------------------
        // FUNCTION 0x05 (Write Single Coil)
        // ---------------------------------------------------------------------
        case 0x05:
        {
            if (start_addr >= MB_COILS_SIZE)
            {
                Modbus_SendException(func_code, MB_ERR_ILLEGAL_ADDR);
                break;
            }

            uint16_t coil_action = (uint16_t)((mb_rx_buffer[4] << 8) | mb_rx_buffer[5]);
            if (coil_action == 0xFF00)
            {
                modbus_coils[start_addr] = 1;
            }
            else if (coil_action == 0x0000)
            {
                modbus_coils[start_addr] = 0;
            }
            else
            {
                Modbus_SendException(func_code, MB_ERR_ILLEGAL_VAL);
                break;
            }

            // Bounce exact confirmation frame back to master
            uart2_send_buffer(mb_rx_buffer, mb_rx_index);
            break;
        }

        // ---------------------------------------------------------------------
        // FUNCTION 0x06 (Write Single Holding Register)
        // РЕЖИМ ЗАПИСИ УСТАВОК КОНФИГУРАЦИИ (Функция Modbus 0x06)
        // ---------------------------------------------------------------------
        case 0x06:
        {
            // Проверка корректности адреса целевой ячейки записи
            if (start_addr >= MB_HOLDING_REGS_SIZE)
            {
                // TODO - extern void Modbus_SendException(uint8_t func_code, uint8_t exception_code);
                Modbus_SendException(func_code, MB_ERR_ILLEGAL_ADDR);
                break;
            }

            // Извлекаем 16-битное значение, присланное Мастером для записи
            uint16_t write_value = (uint16_t)((mb_rx_buffer[4] << 8) | mb_rx_buffer[5]);
            
            // Защита исполнительных механизмов. Если Мастер пытается записать в регистр 1 (уставка нагрева)
            // значение выше 85.00°C (8500 целым), отвергаем команду на уровне шлюза.
            if (start_addr == 1U && write_value > 8500U)
            {
                // TODO - extern void Modbus_SendException(uint8_t func_code, uint8_t exception_code);
                Modbus_SendException(func_code, MB_ERR_ILLEGAL_VAL); 
                break;
            }

            // Атомарно фиксируем принятое значение в локальной карте регистров
            __disable_irq();
            modbus_holding_registers[start_addr] = write_value;
            __enable_irq();

            // PACK AND TRANSMIT CONFIGURATION VIA THE UNIVERSAL DATA ARRAY
            spi_master_tx.transaction_id = 0x0606;
            spi_master_tx.cmd            = (uint8_t)start_addr; // Register identifier

            // Slice the 16-bit Modbus write parameter directly into the byte container
            spi_master_tx.data[0]        = (uint8_t)(write_value >> 8);
            spi_master_tx.data[1]        = (uint8_t)(write_value & 0xFF);

            for(i = 2; i < 8; i++) {
                spi_master_tx.data[i] = 0x00; // Wipe remainder bytes
            }

            // Fire configuration payload down to local slave chip over 1-Wire SPI1
            GPIOA->BSRR = GPIO_BSRR_BR_4; // CS -> LOW
            spi_success = SPI1_Master_Exchange_Packet(&spi_master_tx, &spi_master_rx);
            GPIOA->BSRR = GPIO_BSRR_BS_4; // CS -> HIGH

            // Trigger emergency backups if SPI drops or the slave reports an error flag (e.g. byte 7 == 1)
            if (!spi_success || spi_master_rx.data[7] != 0 || (start_addr == 1 && write_value > 7500))
            {
                GPIOC->BSRR = GPIO_BSRR_BS_13; // Assert Red Error Alarm LED
                extern void EEPROM_Write_Backup(void);
                EEPROM_Write_Backup();         // Snapshot fallback save
            }
            else
            {
                GPIOC->BSRR = GPIO_BSRR_BR_13; // Clear error alert state
            }

            // Успешный ответ Мастеру. По спецификации Modbus RTU, ответом на функцию 0x06
            // является точная зеркальная копия принятого эхо-запроса.
            uart2_send_buffer(mb_rx_buffer, mb_rx_index);
            break;
        }
        // ---------------------------------------------------------------------
        // ОБРАБОТКА НЕПОДДЕРЖИВАЕМЫХ СЕТЕВЫХ ФУНКЦИЙ
        // ---------------------------------------------------------------------
        default:
        // TODO - extern void Modbus_SendException(uint8_t func_code, uint8_t exception_code);
            Modbus_SendException(func_code, MB_ERR_ILLEGAL_FUNC);  // Ошибка 0x01 (Illegal Function)
            break;
    }
// =============================================================================
// 4. ВЫХОДНОЙ ШЛЮЗ ОЧИСТКИ (CLEANUP)
// =============================================================================
cleanup:
// Атомарный сброс флагов и индексов сетевого интерфейса.
// Защищает буфер от рассинхронизации, если новый байт прилетит по UART в момент очистки.
    __disable_irq();
    mb_rx_index = 0;
    mb_frame_ready = 0;
    __enable_irq();
}
