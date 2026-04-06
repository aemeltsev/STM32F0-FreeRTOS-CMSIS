PROJECT = firmware

# Get current date and time (Format: YYMMDD_HHMM)
BUILD_TIME := $(shell date "+%y%m%d_%H%M")

# File contain build number
BUILD_FILE = .build_no

# Read the old number (or 0 if there is no file)
OLD_BUILD  := $(shell [ -f $(BUILD_FILE) ] && cat $(BUILD_FILE) || echo 0)

# Increment (increase for current build)
# This will run once when you run make 
NEW_BUILD_NUMBER := $(shell expr $(OLD_BUILD) + 1 > $(BUILD_FILE); cat $(BUILD_FILE))

ARCHIVEDIR = archive
# Forming a name for the archive: firmware_v42_240520_1530
ARCHIVE_NAME = $(PROJECT)_v$(NEW_BUILD_NUMBER)_$(BUILD_TIME)

##############################################################################
# Block of service variables
# Environmental data
СOMPILER_VERSION := $(shell $(CC) --version | head -n 1)
HOSTNAME         := $(shell hostname)
USERNAME         := $(shell whoami)
GIT_REVISION     := $(shell git rev-parse --short HEAD 2>/dev/null || echo "unknown")

# Path to tools(for logs)
TOOLCHAIN_PATH   := $(shell which $(CC))

##############################################################################
# Microcontroller settings
MCU               = cortex-m0
MCU_FAMILY        = STM32F0xx
MCU_MODEL_FAMILY  = STM32F030x8
MCU_MODEL         = STM32F030C8

# Conditional compilation: Set to no to turn off OLED
USE_I2C_OLED = yes

ifeq ($(USE_I2C_OLED), yes)
  UDEFS += -DI2C_OLED
endif

# Automatically collect paths for INCDIR (to avoid specifying each folder)
# Add all folders containing .c files to the search path for .h files
INCDIR += $(sort $(dir $(CSRC)))
# -----------------------------

include ./lib/CMSIS.mk
#include ./lib/FreeRTOS.mk

CSRC += src/main.c
CSRC += src/ssd1306.c

ASMSRC += 

INCDIR += src/conf

##############################################################################
# Compiler & Toolchain settings
#
TRGT = arm-none-eabi-
CC   = $(TRGT)gcc
CPPC = $(TRGT)g++
LD   = $(TRGT)gcc
CP   = $(TRGT)objcopy
AS   = $(TRGT)gcc -x assembler-with-cpp
OD   = $(TRGT)objdump
SZ   = $(TRGT)size
HEX  = $(CP) -O ihex
BIN  = $(CP) -O binary

# General flags for optimisation and debugging
OPT = -O2 -ggdb -fomit-frame-pointer -falign-functions=16
OPT += -ffunction-sections -fdata-sections -fno-common

# Architecture flags (Cortex-M0 only works in Thumb mode)
MCFLAGS = -mcpu=$(MCU) -mthumb -DTHUMB -DTHUMB_PRESENT -DTHUMB_NO_INTERWORKING

# Warnings
CWARN   = -Wall -Wextra -Wstrict-prototypes
CPPWARN = -Wall -Wextra

# Linker
LDSCRIPT = ./ld/$(MCU_MODEL)_FLASH.ld
LDOPT    = --gc-sections
LDFLAGS  = $(MCFLAGS) $(OPT) -Wl,-Map=$(BUILDDIR)/$(PROJECT).map,--cref,--script=$(LDSCRIPT),$(LDOPT)

##############################################################################
# Processing paths and files
#
BUILDDIR ?= build
OBJDIR    = $(BUILDDIR)/obj
LSTDIR    = $(BUILDDIR)/lst

# Assembling a list of objects (we automatically count everything in Thumb, since there is no ARM mod)
OBJS = $(addprefix $(OBJDIR)/, $(notdir $(CSRC:.c=.o) $(ASMSRC:.s=.o) $(ASMXSRC:.S=.o) $(CPPSRC:.cpp=.o)))

# Formatting flags for compilation
IINCDIR = $(patsubst %,-I%,$(INCDIR) $(DINCDIR) $(UINCDIR))
DEFS    = $(DDEFS) $(UDEFS)

CFLAGS   = $(MCFLAGS) $(OPT) $(CWARN) $(DEFS) $(IINCDIR)
CPPFLAGS = $(MCFLAGS) $(OPT) $(CPPWARN) $(DEFS) $(IINCDIR) -fno-rtti
ASFLAGS  = $(MCFLAGS) $(DEFS) $(IINCDIR)

# Generating dependencies
CFLAGS   += -MD -MP -MF .dep/$(@F).d
CPPFLAGS += -MD -MP -MF .dep/$(@F).d

# Path for cource search
VPATH = $(sort $(dir $(CSRC) $(CPPSRC) $(ASMSRC) $(ASMXSRC)))

OUTFILES = $(BUILDDIR)/$(PROJECT).elf $(BUILDDIR)/$(PROJECT).hex \
           $(BUILDDIR)/$(PROJECT).bin $(BUILDDIR)/$(PROJECT).dmp

###############################################################################
# Makefile rules
#

# Customizing console colors (works in MSYS2/Linux)
printf_format = "\%-20s \%s\n"
C_CYAN   = \033[1;36m
C_RESET  = \033[0m

all: $(OBJS) $(OUTFILES) MAKE_ALL_RULE_HOOK

MAKE_ALL_RULE_HOOK:
	@mkdir -p $(ARCHIVEDIR)
	@cp $(BUILDDIR)/$(PROJECT).bin $(ARCHIVEDIR)/$(ARCHIVE_NAME).bin
	@cp $(BUILDDIR)/$(PROJECT).hex $(ARCHIVEDIR)/$(ARCHIVE_NAME).hex
	@echo "------------------------------------------------" > $(ARCHIVEDIR)/$(ARCHIVE_NAME).log
	@printf $(printf_format) "Project:" "$(PROJECT)" >> $(ARCHIVEDIR)/$(ARCHIVE_NAME).log
	@printf $(printf_format) "Build:" "$(NEW_BUILD_NUMBER)" >> $(ARCHIVEDIR)/$(ARCHIVE_NAME).log
	@printf $(printf_format) "Date:" "$(BUILD_TIME)" >> $(ARCHIVEDIR)/$(ARCHIVE_NAME).log
	@printf $(printf_format) "Git Rev:" "$(GIT_REVISION)" >> $(ARCHIVEDIR)/$(ARCHIVE_NAME).log
	@printf $(printf_format) "Compiler:" "$(СOMPILER_VERSION)" >> $(ARCHIVEDIR)/$(ARCHIVE_NAME).log
	@printf $(printf_format) "User:" "$(USERNAME)@$(HOSTNAME)" >> $(ARCHIVEDIR)/$(ARCHIVE_NAME).log
	@printf $(printf_format) "Flags:" "$(CFLAGS)" >> $(ARCHIVEDIR)/$(ARCHIVE_NAME).log
	@echo "------------------------------------------------" >> $(ARCHIVEDIR)/$(ARCHIVE_NAME).log
	@$(SZ) $(BUILDDIR)/$(PROJECT).elf >> $(ARCHIVEDIR)/$(ARCHIVE_NAME).log
	@echo "Build archived: $(ARCHIVE_NAME)"

# Creating the necessary directories
$(OBJS): | $(BUILDDIR) $(OBJDIR) $(LSTDIR)

$(BUILDDIR) $(OBJDIR) $(LSTDIR):
	@mkdir -p $(OBJDIR)
	@mkdir -p $(LSTDIR)
	@echo "------------------------------------------------"
	@echo -e "$(C_CYAN)Preparing Build: v$(BUILD_NUMBER) ($(BUILD_TIME))$(C_RESET)"
	@printf $(printf_format) "  PROJECT:" "$(PROJECT)"
	@printf $(printf_format) "  BUILD VER:" "$(BUILD_NUMBER) (Time: $(BUILD_TIME))"
	@printf $(printf_format) "  GIT REV:" "$(GIT_REVISION)"
	@printf $(printf_format) "  COMPILER:" "$(СOMPILER_VERSION)"
	@printf $(printf_format) "  TOOLCHAIN:" "$(TOOLCHAIN_PATH)"
	@printf $(printf_format) "  USER/HOST:" "$(USERNAME)@$(HOSTNAME)"
	@printf $(printf_format) "  OPTIMIZATION:" "$(OPT)"
	@echo "------------------------------------------------"

# A universal rule for C files
$(OBJDIR)/%.o: %.c Makefile
	@echo "Compiling $(<F)"
	@$(CC) -c $(CFLAGS) -I. $< -o $@

# A universal rule for C++ files
$(OBJDIR)/%.o: %.cpp Makefile
	@echo "Compiling C++ $(<F)"
	@$(CPPC) -c $(CPPFLAGS) -I. $< -o $@

# Universal rule for assembler (.s and .S)
$(OBJDIR)/%.o: %.s Makefile
	@echo "Assembling $(<F)"
	@$(AS) -c $(ASFLAGS) -I. $< -o $@

$(OBJDIR)/%.o: %.S Makefile
	@echo "Assembling $(<F)"
	@$(CC) -c $(ASFLAGS) -I. $< -o $@

# Linking and generating output formats
%.elf: $(OBJS) $(LDSCRIPT)
	@echo Linking .elf file $@
	@$(LD) $(OBJS) $(LDFLAGS) $(LIBS) -o $@

%.hex: %.elf $(LDSCRIPT)
	@echo Creating .hex file $@
	@$(HEX) $< $@

%.bin: %.elf $(LDSCRIPT)
	@echo Creating .bin file $@
	@$(BIN) $< $@

%.dmp: %.elf $(LDSCRIPT)
	@echo "Creating dump and size info"
#	@$(OD) $(ODFLAGS) $< > $@
	@$(OD) -x --syms $< > $@
	@echo "------------------------------------------------"
	@$(SZ) $<
	@echo "------------------------------------------------"
	@echo "Done"

clean:
	@echo "Cleaning project..."
	-rm -fR .dep $(BUILDDIR)
	@echo "Done"

################################################################################
# Include the dependency files, should be the last of the makefile
#
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

################################################################################
# Service commands
#
show_defs:
	@echo "Current DEFS: $(DEFS)"

# Flashes your board using OpenOCD
flash: all
#	openocd -f $(OPENOCD_BOARD_DIR)/$(OPENOCD_BOARD_CFG) -f openocd/stm32f0-flash.cfg \
            -c "stm_flash `pwd`/$(BUILDDIR)/$(PROJECT).bin" -c shutdown

# *** EOF ***
