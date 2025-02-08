# STM32F0-FreeRTOS-CMSIS

The template project for a quick start with STM32 microcontroller using CMSIS and FReeRTOS. Use this template for rapid prototyping and debugging firmware on Cortex-M0 microcontrollers. Note: This project forked from [this](https://github.com/WoodyWoodsta/STM32F0-freeRTOS-CMSIS) repo.

## Project features points
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
Specify the type of microcontroller

```
MCU_FAMILY        = STM32F0xx
MCU_MODEL_FAMILY  = STM32F030x6
MCU_MODEL         = STM32F030C6
```

Of course, this project uses arm cortex-m0 microcontrollers arhitecture libraries. GNU Arm Embedded Toolchain is required to compile c, c++ and assembler files.

```
sudo apt update
sudo apt install gcc-arm-none-eabi
```

Or you can read about specific installation details in [this](https://askubuntu.com/questions/1243252/how-to-install-arm-none-eabi-gdb-on-ubuntu-20-04-lts-focal-fossa) topic, or use script.
Check if it works:

```
arm-none-eabi-gcc --version
arm-none-eabi-g++ --version
arm-none-eabi-gdb --version
arm-none-eabi-size --version
```

During compilation, debug information is created, `-O2` optimization, flags for reducing the size of the generated code, THUMB instructions etc. Warning flags are enabled `-Wall` `-Wextra` `-Wstrict-prototypes`.
The linker parameters have the `USE_LINK_GC` garbage collection and `USE_LTO` link time optimization flags set. The linker uses a script that describes the memory allocation map. Entry Point of the program, typically the reset handler function. Memory Regions, available in the microcontroller:
  - `FLASH`: Read-only memory where the program code and constants are stored.
  - `RAM`: Read-write memory used for variables and stack.

Sections defines how different parts of the program are mapped to the memory regions.
  - `.text`: Contains the program code and read-only data.
  - `.data`: Contains initialized global and static variables.
  - `.bss`: Contains uninitialized global and static variables.

### CMSIS
This project contains a legacy version of the Standard Interface Library - [CMSIS 4](https://github.com/ARM-software/CMSIS_4), which supports Cortex-M0, -M0+. It will be replaced by version [CMSIS 5](https://github.com/STMicroelectronics/cmsis-device-f0/tree/v2.3.7), which includes updated macros for register declarations and bit definitions, updated data structures and address mapping for peripherals, and support for Cortex-M23 and Cortex-M33.

### FreeRTOS
The repository contains the FreeRTOS kernel version 7.6 and can be replaced with a newer version of the FreeRTOS kernel version 11.0.0 of the functions suitable for Cortex-M0, M0+, M23 are support for the corePKCS11 and WolfSSL cryptography libraries as an add-on from FreeRTOS-Plus. Examples of projects can be found [here](https://github.com/FreeRTOS/FreeRTOS/tree/main/FreeRTOS-Plus/Demo) and more details about PKCS (Public-Key Cryptography Standards) [here](https://habr.com/en/companies/aktiv-company/articles/544748/) for beginners, about WolfSSL [here](https://www.wolfssl.com/docs/). The difference and updates to the kernel in the 7.6 and 11.0 releases can be estimated - [ FreeRTOS-Kernel v11.0](https://github.com/FreeRTOS/FreeRTOS-Kernel) and [Changes between V7.5.3 and V7.6.0 released](https://www.freertos.org/Documentation/04-Roadmap-and-release-note/02-Release-notes/00-Release-history#changes-between-v753-and-v760-released-18th-november-2013)

  - The root of this repository contains the three files that are common to every port - list.c, queue.c and tasks.c. The kernel is contained within these three files. croutine.c implements the optional co-routine functionality - which is normally only used on very memory limited systems.
  - The `./portable` directory contains the files that are specific to a particular microcontroller and/or compiler. See the readme file in the `./portable` directory for more information.
  - The `./include` directory contains the real time kernel header files.
  - The `./template_configuration` directory contains a sample `FreeRTOSConfig.h` to help jumpstart a new project. See the [FreeRTOSConfig](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/main/examples/template_configuration/FreeRTOSConfig.h).h file for instructions. 


### OpenOCD
TODO

### Debug
TODO