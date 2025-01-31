# STM32F0-FreeRTOS-CMSIS

The template project for a quick start with STM32 microcontroller using CMSIS and FReeRTOS. Use this template for rapid prototyping and debugging firmware on Cortex-M0 microcontrollers. Note: This project forked from this repo - https://github.com/WoodyWoodsta/STM32F0-freeRTOS-CMSIS.

## Project features
### STM32F0 microcontroller peripherals

|                        |    STM32F030C6    |    STM32F051R8    |    STM32F072RB    |
|------------------------|:-----------------:|:-----------------:|:-----------------:|
| Flash (Kbyte)          |         32        |         64        |        128        |
| SRAM (Kbyte)           |         4         |         8         |         16        |
| Advanced Timers        |      1(16bit)     |      1(16bit)     |      1(16bit)     |
| General Purpose Timers |      4(16bit)     | 5(16bit)/1(32bit) | 5(16bit)/1(32bit) |
| Basic Timers           |         -         |      1(16bit)     |      2(16bit)     |
| SPI                    |         1         |         2         |         2         |
| I2C                    |         1         |         2         |         2         |
| USART                  |         1         |         2         |         4         |
| CAN                    |         -         |         -         |         1         |
| USB                    |         -         |         -         |         1         |
| CEC                    |         -         |         1         |         1         |
| 12bit ADC              | 1(10ext. + 2int.) | 1(16ext. + 3int.) | 1(16ext. + 3int.) |
| 12bit DAC              |         -         |         1         |         1         |
| Comparator             |         -         |         2         |         2         |
| CPU Freq. Max.         |       48MHz       |       48MHz       |       48MHz       |

For a more detailed acquaintance with the peripherals that the microcontroller contains, refer to the datasheet reference manual and errorsheet.

### Tests

The following were used for testing:
  - STM32F030C6 - "Blue Pill"
  - STM32F051R8 - [STM32F0DISCOVERY](https://www.st.com/en/evaluation-tools/stm32f0discovery.html)
  - STM32F072RB - [NUCLEO-F072RB](https://www.st.com/en/evaluation-tools/nucleo-f072rb.html)

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