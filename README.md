# STM32F0-FreeRTOS-CMSIS

The template project for a quick start with STM32 microcontroller using CMSIS and FReeRTOS. Use this template for rapid prototyping and debugging firmware on Cortex-M0 microcontrollers. Note: This project forked from [this](https://github.com/WoodyWoodsta/STM32F0-freeRTOS-CMSIS) repo.

## Project features
### STM32F0 microcontroller peripherals

|                        |    STM32F030C6    |    STM32F051R8    |    STM32F072RB    |         STM32G0B1CB        |
|------------------------|:-----------------:|:-----------------:|:-----------------:|:--------------------------:|
| Flash (Kbyte)          |         32        |         64        |        128        |             128            |
| SRAM (Kbyte)           |         4         |         8         |         16        |             144            |
| Advanced Timers        |      1(16bit)     |      1(16bit)     |      1(16bit)     |          1(16bit)          |
| General Purpose Timers |      4(16bit)     | 5(16bit)/1(32bit) | 5(16bit)/1(32bit) | 6(16bit)/1(16bit)/1(32bit) |
| Basic Timers           |         -         |      1(16bit)     |      2(16bit)     |          2(16bit)          |
| SysTick                |         1         |         1         |         1         |              1             |
| Watchdog               |         2         |         2         |         2         |              2             |
| SPI                    |         1         |         2         |         2         |              3             |
| I2C                    |         1         |         2         |         2         |              3             |
| USART                  |         1         |         2         |         4         |              6             |
| CAN                    |         -         |         -         |         1         |          2(FDCAN)          |
| USB                    |         -         |         -         |         1         |              1             |
| CEC                    |         -         |         1         |         1         |              1             |
| 12bit ADC              | 1(10ext. + 2int.) | 1(16ext. + 3int.) | 1(16ext. + 3int.) |      1(14ext. + 3int.)     |
| 12bit DAC              |         -         |         1         |         1         |              2             |
| Comparator             |         -         |         2         |         2         |              3             |
| CPU Freq. Max.         |       48MHz       |       48MHz       |       48MHz       |            64MHz           |

For a more detailed acquaintance with the peripherals that the microcontroller contains, refer to the datasheet reference manual and errorsheet. Note: New peripheral functionality in STM32G0 includes Low-power Timers, LPUART, UCPD, [FDCAN](https://habr.com/en/articles/546282/), Internal voltage reference.

### Tests

The following were used for testing:
  - STM32F030C6 - "Blue Pill"
  - STM32F051R8 - [STM32F0DISCOVERY](https://www.st.com/en/evaluation-tools/stm32f0discovery.html)
  - STM32F072RB - [NUCLEO-F072RB](https://www.st.com/en/evaluation-tools/nucleo-f072rb.html)
  - STM32G0B1CB - TODO

### Makefile
TODO
### CMSIS
TODO
### FreeRTOS
TODO
### OpenOCD
TODO
### Debug
TODO