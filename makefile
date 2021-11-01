#
#             LUFA Library
#     Copyright (C) Dean Camera, 2019.
#
#  dean [at] fourwalledcubicle [dot] com
#           www.lufa-lib.org
#
# --------------------------------------
#         LUFA Project Makefile.
# --------------------------------------

# Run "make help" for target help.

# Settings for a 5v Arduino Pro Micro (make && make avrdude)
# NB: Must run make avrdude within 8 seconds of reset (short GND & RST)
MCU          = atmega32u4
BOARD        = MICRO
AVRDUDE_PROGRAMMER=avr109
AVRDUDE_PORT=/dev/ttyACM0

# Settings for minimus (make && make dfu)
#MCU          = at90usb162
#BOARD        = MINIMUS
ARCH         = AVR8

F_CPU        = 16000000
F_USB        = $(F_CPU)
OPTIMIZATION = s
TARGET       = SerialRemote
SRC          = $(TARGET).c Descriptors.c avr-nec-ir-decoder/IR_Reciever.cpp $(LUFA_SRC_USB) $(LUFA_SRC_USBCLASS)
LUFA_PATH    = LUFA/LUFA
CC_FLAGS     = -DUSE_LUFA_CONFIG_HEADER -IConfig/
LD_FLAGS     =

# Default target
all:

# Include LUFA-specific DMBS extension modules
DMBS_LUFA_PATH ?= $(LUFA_PATH)/Build/LUFA
include $(DMBS_LUFA_PATH)/lufa-sources.mk
include $(DMBS_LUFA_PATH)/lufa-gcc.mk

# Include common DMBS build system modules
DMBS_PATH      ?= $(LUFA_PATH)/Build/DMBS/DMBS
include $(DMBS_PATH)/core.mk
include $(DMBS_PATH)/cppcheck.mk
include $(DMBS_PATH)/doxygen.mk
include $(DMBS_PATH)/dfu.mk
include $(DMBS_PATH)/gcc.mk
include $(DMBS_PATH)/hid.mk
include $(DMBS_PATH)/avrdude.mk
include $(DMBS_PATH)/atprogram.mk
