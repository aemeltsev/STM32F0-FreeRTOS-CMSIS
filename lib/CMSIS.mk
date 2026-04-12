# New version
CMSIS_VERSION = 5.9.0

MCU_FAMILY_LOWERCASE = $(shell echo $(MCU_FAMILY) | tr '[:upper:]' '[:lower:]')
MCU_MODEL_FAMILY_LOWERCASE  = $(shell echo $(MCU_MODEL_FAMILY) | tr '[:upper:]' '[:lower:]')

CMSIS = ./lib/CMSIS/$(CMSIS_VERSION)

# The paths remain the same
CSRC += $(CMSIS)/Device/ST/$(MCU_FAMILY)/Source/Templates/system_$(MCU_FAMILY_LOWERCASE).c

ASMSRC += $(CMSIS)/Device/ST/$(MCU_FAMILY)/Source/Templates/gcc/startup_$(MCU_MODEL_FAMILY_LOWERCASE).s

# IMPORTANT: Add path to Core Include
INCDIR += $(CMSIS)/Device/ST/$(MCU_FAMILY)/Include \
          $(CMSIS)/Include
