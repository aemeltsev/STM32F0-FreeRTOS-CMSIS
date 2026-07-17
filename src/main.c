#include "stm32f0xx.h"
#include "system_init.h"

#include "timer_drv.h"
#include "uart_drv.h"
#include "buzzer.h"
#include "leds.h"
#include "spi_hd_drv.h"
#include "modbus_crc.h"
#include "util.h"

#include "usart.h"
#include "button.h"

#include <stdint.h>
#include <stdbool.h>

// Константы времени для неблокирующих задач
#define TICKS_PER_MONTH    2592000000UL // 1 месяц в миллисекундах (30 дней)

// Буферы для работы с Modbus RTU через прерывания
extern volatile uint8_t  modbus_rx_buffer[];
extern volatile uint16_t modbus_rx_index;
extern volatile uint8_t  modbus_frame_ready;

// Системное время (инкрементируется в SysTick_Handler каждые 1 мс)
volatile uint32_t current_ticks = 0;
uint32_t last_eeprom_tick = 0; // Время последней записи в EEPROM

// Local prototypes
void Modbus_Process_Frame(void);
void EEPROM_Write_Monthly_Backup(void);
void SysTick_Init(uint32_t ticks);

int main(void)
{
    // 1. First we need configure clock. Initialize the core clock frequency to 48 MHz
    // clock_init_hsi_8MHz();
    clock_init_hsi_48MHz(); 
    
    // 2. After call any ClockInit... functios need reinitialise SysTick
    //SysTick_Init(SystemCoreClock);
    SysTick_Init(48000000);

    // 3. Initialise other peripheral
    usart1_gpio_init(9600, 48000000);
    // 3. Низкоуровневая конфигурация всех GPIO (Пищалка, Светодиоды, SPI, I2C)
    system_gpio_init();

    // 4. Запуск периферийных модулей (Modbus на скорости 9600)
    buzzer_init();
    leds_init();
    uart2_init_hardware_rs485(9600);
    tim2_init_modbus_timeout(9600);
    spi1_master_hd_init();
    spi2_master_hd_init();
    i2c1_eeprom_init(); // Полудуплексный аппаратный режим SPI
    
    while (1)
    {
        //button_to_led(&button_state);
    }
}

/**
 * @brief  Configuration of system lernel timer
 */
void SysTick_Init(uint32_t ticks)
{
    // SystemCoreClock / 1000 — the number of ticks for 1 ms
    // The LOAD controller determines the period
    // If we passed 1000 as the "desired interrupt frequency in Hz"
    // then we need to divide the system frequency by this number
    if(ticks == 1000) {
        SysTick->LOAD = (SystemCoreClock / 1000) - 1;
    } else {
        // Otherwise, we consider that the finished processor frequency has been transferred
        SysTick->LOAD = (ticks / 1000) - 1;
    }
    
    // Reset current value
    SysTick->VAL = 0;
    
    // Control register settings:
    // BIT 2: CLKSOURCE = 1 (processor frequency)
    // BIT 1: TICKINT = 1 (enable interrupt)
    // BIT 0: ENABLE = 1 (start timer)
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | 
                    SysTick_CTRL_TICKINT_Msk   | 
                    SysTick_CTRL_ENABLE_Msk;
}

/**
 * @brief  System interrupt handler (Called by hardware every 1 ms)
 */
void SysTick_Handler(void)
{
    current_ticks++;
    //Buzzer_Tick_Handler(); // Обслуживание неблокирующего выключения пищалки
}