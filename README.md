# STM32F0-FreeRTOS-CMSIS

This repository is a boilerplate project designed for the fastest possible deployment of applications based on the STM32F0 (Cortex-M0) microcontroller family.
The main philosophy is that the project is based on pure CMSIS. In my opinion, this provides complete control over peripheral registers and eliminates redundant HAL/LL library code.
The project integrates the FreeRTOS kernel, allowing you to instantly transition from simple while(1) loops to a full-fledged real-time operating system with task sharing, queues, and semaphores.
A custom Makefile allows you to build the firmware in any environment (Linux, Windows/MSYS2), automate version numbering, and save build history in an archive.

Perfect for a quick start. You don't need to spend a lot of time configuring linker scripts (.ld) and startup files (.s)—everything is already configured for the STM32F030. Optimized compiler flags and automatic generation of dump files (.dmp) and listings (.lst) allow for on-the-fly analysis of assembly code and memory usage.
Thanks to the console build interface, the project easily integrates into automated testing systems (GitHub Actions, etc.).

The project is a fork of [this](https://github.com/WoodyWoodsta/STM32F0-freeRTOS-CMSIS) repository, reworked to improve the build system, support modern displays (OLED SSD1306), and provide extended compilation logging.

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

### :books: Documentation
For a more in-depth exploration of the peripherals, use the official STMicroelectronics documentation. Datasheet / Reference Manual for escription of registers and electrical characteristics. And Errata for known hardware errors and workarounds.

This project retains partial compatibility with the STM32G0, which offers enhanced functionality: Low-power Timers, LPUART, UCPD (USB-C Power Delivery), [FDCAN](https://habr.com/en/articles/546282/), and a stable internal reference voltage source.

### :test_tube: Testing and Debug Boards (Hardware Tests)
This template has been successfully tested and works stably on the following configurations:

✅ STM32F030C8 — Main platform (current project focus).
✅ STM32F030C6 — Popular "Blue Pill" board (requires Flash size control < 32KB).
✅ STM32F051R8 — Official [STM32F0DISCOVERY](https://www.st.com/en/evaluation-tools/stm32f0discovery.html) board.
✅ STM32F072RB — [NUCLEO-F072RB](https://www.st.com/en/evaluation-tools/nucleo-f072rb.html) board.
⚠️ STM32G0B1CB — In the process of integration (Experimental).

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
The repository contains the FreeRTOS kernel version 7.6 and can be replaced with a newer version of the FreeRTOS kernel 11.0.0 of the functions suitable for Cortex-M0, M0+, M23 are support for the corePKCS11 and WolfSSL cryptography libraries as an add-on from FreeRTOS-Plus. Examples of projects can be found [here](https://github.com/FreeRTOS/FreeRTOS/tree/main/FreeRTOS-Plus/Demo) and more details about PKCS (Public-Key Cryptography Standards) [here](https://habr.com/en/companies/aktiv-company/articles/544748/) for beginners, about WolfSSL [here](https://www.wolfssl.com/docs/). The difference and updates to the kernel in the 7.6 and 11.0 releases can be estimated - [ FreeRTOS-Kernel v11.0](https://github.com/FreeRTOS/FreeRTOS-Kernel) and [Changes between V7.5.3 and V7.6.0 released](https://www.freertos.org/Documentation/04-Roadmap-and-release-note/02-Release-notes/00-Release-history#changes-between-v753-and-v760-released-18th-november-2013)

  - The root of this repository contains the three files that are common to every port - list.c, queue.c and tasks.c. The kernel is contained within these three files. croutine.c implements the optional co-routine functionality - which is normally only used on very memory limited systems.
  - The `./portable` directory contains the files that are specific to a particular microcontroller and/or compiler. See the readme file in the `./portable` directory for more information.
  - The `./include` directory contains the real time kernel header files.
  - The `./template_configuration` directory contains a sample `FreeRTOSConfig.h` to help jumpstart a new project. See the [FreeRTOSConfig](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/main/examples/template_configuration/FreeRTOSConfig.h).h file for instructions. In this repo current configuration file contain in `.src/conf`.

More about FreeRTOS see in this repo [FreeRTOS Kernel Book](https://github.com/FreeRTOS/FreeRTOS-Kernel-Book).

### OpenOCD
OpenOCD (Open On-Chip Debugger) is open-source software that interfaces with a hardware debugger's JTAG port. OpenOCD provides debugging and in-system programming for embedded target devices. Commonly, OpenOCD is paired with GDB (GNU Debugger) to provide a rich environment for debugging embedded applications. The debugger is capable of setting breakpoints, examining memory, and stepping through code execution, enabling precise control over what is happening on the device. For more info about openocd see [docs](https://openocd.org/pages/documentation.html), and on this guide series [OpenOCD: user guide, first](https://microsin.net/programming/arm/openocd-manual-part1.html), [OpenOCD: user guide, second](https://microsin.net/programming/ARM/openocd-manual-part2.html), [OpenOCD: user guide, third](https://microsin.net/programming/ARM/openocd-manual-part3.html).

Install under Ubuntu 22.04 and use it:
```
sudo apt-get install openocd

# Start OpenOCD with JLink debugger and STM32 target configuration
openocd -f interface/stlink-v2-1.cfg -c "transport select hla_swd" -f target/stm32l0.cfg

# Connect to OpenOCD via telnet
telnet 127.0.0.1 4444

#Connect to OpenOCD via gdb
gdb
(gdb) target extended-remote localhost:3333
(gdb) monitor reset halt
(gdb) load
(gdb) continue

# Halt the CPU
halt

# Reset and initialize the CPU
reset init

# Get flash memory information
halt; flash info 0

# Dump the flash memory to a file
halt; dump_image flashdump.bin 0x00000000 0xF90600
```
For Win32 here is the prebuild binary can be downloaded from: https://gnutoolchains.com/arm-eabi/openocd/


### Debug
TODO