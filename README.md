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

### Features
Each model in this template is selected for specific use cases:

The [STM32F030C6](https://www.st.com/en/microcontrollers-microprocessors/stm32f030c6.html) (Value Line) is the most affordable entry into the world of 32-bit systems. It's ideal for replacing 8-bit AVRs (like Arduino). It contains only 32 KB of Flash memory. It requires rigorous code optimization (the `--gc-sections` flags are useful here). It's suitable for use in simple sensors, lighting controllers, and I2C nodes.

The [STM32F051R8](https://www.st.com/en/microcontrollers-microprocessors/stm32f051r8.html) (Access Line) features a 12-bit DAC and two analog comparators. It supports a larger number of timers (including one 32-bit timer), allowing for generating very precise PWM signals or measuring long intervals. It's suitable for controlling simple motors and as the main controller in budget-level laboratory instruments.

The [STM32F072RB](https://www.st.com/en/microcontrollers-microprocessors/stm32f072rb.html) (USB/CAN Crystal-less) supports USB 2.0 Full Speed ​​without an external crystal oscillator (saving board space) and a CAN bus. It contains 128 KB of Flash, allowing for the smooth use of FreeRTOS with heavy protocol stacks. It can be used to implement USB-HID devices (keyboards, joysticks) and automotive diagnostic adapters (OBD-II).

The [STM32G0B1CB](https://www.st.com/en/microcontrollers-microprocessors/stm32g0b1cb.html) (Next Gen) utilizes a next-generation architecture. It operates at 64 MHz (versus 48 MHz on the F0) and has a whopping 144 KB of SRAM. It supports Low-power Timers, LPUART, UCPD (USB-C Power Delivery), [FDCAN - 8x faster than standard CAN](https://habr.com/en/articles/546282/), and a stable internal reference voltage source at the hardware level. High-speed data transfer, powered devices via USB-C, and complex systems with graphic displays are supported.

### Documentation
For a more in-depth exploration of the peripherals, use the official STMicroelectronics documentation. Datasheet / Reference Manual for escription of registers and electrical characteristics. And Errata for known hardware errors and workarounds.

### Testing and Debug Boards (Hardware Tests)
This template has been successfully tested and works stably on the following configurations:

  - ✅ STM32F030C8 — Main platform (current project focus).
  - ✅ STM32F030C6 — Popular "Blue Pill" board (requires Flash size control < 32KB).
  - ✅ STM32F051R8 — Official [STM32F0DISCOVERY](https://www.st.com/en/evaluation-tools/stm32f0discovery.html) board.
  - ✅ STM32F072RB — [NUCLEO-F072RB](https://www.st.com/en/evaluation-tools/nucleo-f072rb.html) board.
  - ⚠️ STM32G0B1CB — In the process of integration (Experimental).


## Configuration and Architecture (Target Hardware)
This project is designed for use with STM32F0 series microcontrollers with an ARM Cortex-M0 core.

## Target Chip Configuration
All key parameters are set at the beginning of the Makefile. To change the model, simply update the following variables:

```
MCU_FAMILY        = STM32F0xx
MCU_MODEL_FAMILY  = STM32F030x6   # Family (affects memory limits)
MCU_MODEL         = STM32F030C6   # Specific case
```

## Environment Requirements (Toolchain)
The GNU Arm Embedded Toolchain is required to compile C, C++, and assembly files. Installation (Linux/Ubuntu):

```
sudo apt update
sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi libnewlib-arm-none-eabi
```

You can read about specific installation details in [this](https://askubuntu.com/questions/1243252/how-to-install-arm-none-eabi-gdb-on-ubuntu-20-04-lts-focal-fossa) topic, or use script.
Check if it works:

```
arm-none-eabi-gcc --version
arm-none-eabi-g++ --version
arm-none-eabi-gdb --version
arm-none-eabi-size --version
```
  * arm-none-eabi-gcc - C compiller.
  * arm-none-eabi-g++ - C++ compiller.
  * arm-none-eabi-size - memory using analysis.

## Build Process Features
The build system automatically manages optimization and resources:
  * Optimization (`-O2` + `-flto`) - size and speed optimization are enabled. The LTO (Link Time Optimization) flag optimizes code at the junction of different modules, maximizing firmware compression.
  * Smart Linking (`--gc-sections`) - the linker automatically removes unused code (functions that have never been called), which is critical for chips with small Flash memory (32 KB).
  * Diagnostics (`-Wall` `-Wextra`) - extended warning output is enabled to ensure high code quality.
  * Log Generation - each build creates a report in the archive folder with memory analysis results and the Git version.

## Memory Map
The linking process uses a special script (.ld) that distributes data by region:
| Section | Memory Type | Description |
|---|---|---|
| .text | FLASH | Program code, interrupt vector tables, and constants. |
| .data | RAM | Global initialized variables (copied from Flash at startup). |
| .bss | RAM | Global uninitialized variables (cleared at startup). |

### CMSIS
In the current version, the project has been fully migrated to [CMSIS version 5.x](https://github.com/STMicroelectronics/cmsis-device-f0/tree/v2.3.7) from [CMSIS 4](https://github.com/ARM-software/CMSIS_4). Version 5.x includes expanded support for classic Cortex-M0/M0+ cores, and the library now fully supports modern Cortex-M23 and Cortex-M33 cores (TrustZone and security extensions). Updated macros are used for register declarations and bitfield definitions (`_Pos` / `_Msk`). Peripheral data and address maps have been aligned with the latest ARM specifications, eliminating alignment and memory access errors. Thanks to built-in macros (e.g., `__STATIC_INLINE`, `__PACKED_STRUCT`), the project builds correctly in various environments: GCC (Arm Embedded Toolchain), Keil (Arm Compiler 6), and IAR.

### FreeRTOS
This repository uses the FreeRTOS kernel version 10.x. This is a stable branch of the operating system, suitable for commercial and industrial projects. It is actively used as the base for Long Term Support releases. This ensures critical security patches are released without breaking backward compatibility.
Native support for lightweight interprocess communication primitives (Stream Buffer and Message Buffer) optimized for single-writer, single-reader scenarios (ideal for interrupt-to-task coupling—for example, for quickly transferring data from a UART/DMA interrupt to a processing task).
Compatible with modern Cortex-M23 and Cortex-M33 (ARMv8-M (10.3+)) microcontrollers with basic TrustZone support.
Seamless integration of IoT and security libraries ([coreMQTT](https://github.com/FreeRTOS/coreMQTT/blob/main/README.md), [coreHTTP](https://github.com/FreeRTOS/coreHTTP), [mbedTLS](https://github.com/Mbed-TLS/mbedtls))(FreeRTOS-Plus).
Examples of projects can be found [here](https://github.com/FreeRTOS/FreeRTOS/tree/main/FreeRTOS-Plus/Demo) and more details about PKCS (Public-Key Cryptography Standards) [here](https://habr.com/en/companies/aktiv-company/articles/544748/) for beginners, about WolfSSL [here](https://www.wolfssl.com/docs/).

The core functionality of FreeRTOS is concentrated in the root directory and divided into the following modules:
* `list.c`, `queue.c`, `tasks.c` - basic scheduler, list, and interprocess communication functionality (queues, semaphores).
* `timers.c` - software support for timers.
* `stream_buffer.c` - implementation of stream and ring buffers.
* `croutine.c` - [Deprecated] Optional co-routine functionality for systems with extremely low RAM (almost never used in modern projects).

Repository file organization:
* `./portable` - hardware abstraction layer (HAL) for specific microcontrollers and compilers (ports). Refer to the README inside the folder to select the port for your chip.
* `./include` - kernel header files.
* `./src/conf` - current project configuration. The [FreeRTOSConfig.h](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/main/examples/template_configuration/FreeRTOSConfig.h) file in this directory is preconfigured for quick start.
* `./template_configuration` - reference configuration templates for other architectures.

For more information about FreeRTOS, see this repo [FreeRTOS Kernel Book](https://github.com/FreeRTOS/FreeRTOS-Kernel-Book)

### OpenOCD
OpenOCD (Open On-Chip Debugger) is open-source software that communicates with a hardware programmer/debugger via JTAG or SWD interfaces [2]. It enables in-circuit debugging and programming of embedded systems [2].

OpenOCD is typically used in conjunction with GDB (GNU Debugger) [2]. This allows for setting breakpoints, viewing memory, and stepping through code, providing full control over the device [2].

  * Official documentation: [OpenOCD Documentation](https://openocd.org)
  * Useful guides in Russian: [Part 1](https://microsin.net), [Part 2](https://microsin.net), [Part 3](https://microsin.net).

Installing Ubuntu using 22.04 as an example

```bash
sudo apt-get update
sudo apt-get install openocd
```

Example runtime for the ST-Link programmer and the STM32L0 target microcontroller:
```bash
openocd -f interface/stlink.cfg -c "transport select hla_swd" -f target/stm32l0.cfg
```

In the MSYS2 environment (UCRT64 or MINGW64 environments are recommended), OpenOCD is installed using the standard pacman package manager. Open the MSYS2 terminal and install the package
```bash
# Update package databases
pacman -Sy

# Installation for the 64-bit UCRT environment
pacman -S mingw-w64-ucrt-x86_64-openocd

# OR for the classic MinGW64 environment:
# pacman -S mingw-w64-x86_64-openocd
```
Driver setup (Important for Windows) - To ensure Windows and OpenOCD work correctly with the programmer (ST-Link, J-Link, or CMSIS-DAP), download Zadig. Connect the programmer, find it in the Zadig list, and replace the current driver with WinUSB.

Launching OpenOCD is similar to the Linux version, but all paths use the forward slash `/`:
```bash
openocd -f interface/stlink.cfg -c "transport select hla_swd" -f target/stm32l0.cfg
```

💡 Note: If you're not using MSYS2, you can download precompiled OpenOCD binaries for Windows from the Sysprogs Gnutoolchains website.

Once OpenOCD has launched and successfully connected to the chip, you can connect to it from a separate terminal window in one of two ways:

  1. Via GDB (for flashing and step-by-step debugging) - Launch a debugger (e.g., arm-none-eabi-gdb) and run:
```gdb
(gdb) target extended-remote localhost:3333
(gdb) monitor reset halt
(gdb) load
(gdb) continue
```

  2. Via Telnet (for low-level chip control)
```bash
# For MSYS2, first install Telnet with the command: pacman -S inetutils
telnet 127.0.0.1 4444
```
The following OpenOCD commands can be entered within a Telnet session:
* `halt` - stop the processor.
* `reset init` - reset and initialize the processor.
* `flash info 0` - get information about flash memory.
* `dump_image flashdump.bin 0x08000000 0x10000` - dump flash memory to a file (specify your starting address and size).

### Debug
For project debugging, recommend using the VS Code editor with the Cortex-Debug extensions. This allows to instantly view the state of registers, peripherals, and calls directly in the graphical interface.

The following must be installed:
  * `Cortex-Debug` extension for VS Code - for managing ARM debugging processes.
  * `C/C++ Extension Pack` - for syntax highlighting and tooltips.

And also installed `openocd` and the `gdb` debugger for ARM in the MSYS2 (UCRT64/MINGW64) session.

VS Code configuration (`.vscode/launch.json`) - create `.vscode` folder into the root of project and add the `launch.json` file:

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug (OpenOCD)",
            "cwd": "${workspaceFolder}",
            "executable": "${workspaceFolder}/build/your_firmware.elf", // path to the `.elf` file
            "request": "launch",
            "type": "cortex-debug",
            "servertype": "openocd",
            "interface": "swd",
            "runToEntryPoint": "main",
			// Path to the OpenOCD configuration files
            "configFiles": [
                "interface/stlink.cfg",
                "target/stm32l0.cfg"
            ],
			// Support to FreeRTOS task relation into call panel of GDB
            "rtos": "FreeRTOS" 
        }
    ]
}
```

Debugging process
  * Opens in VS Code.
  * Go to the Run and Debug tab (Ctrl + Shift + D).
  * Debug (OpenOCD) configuration in the drop-down list at the top.
  * F5 - the environment run the OpenOCD in the background, connects via GDB, flashes the controller, and stops on the main() function.