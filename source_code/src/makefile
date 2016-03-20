#
#            DMBS Build System
#     Released into the public domain.
#
#  dean [at] fourwalledcubicle [dot] com
#        www.fourwalledcubicle.com
#

# Run "make help" for target help.

MCU          = atmega32u4
ARCH         = AVR8
F_CPU        = 16000000
OPTIMIZATION = s
TARGET       = mooltipass
SRC          = $(LIBOBJS) $(OBJECTS)
CC_FLAGS     = -Werror $(addprefix -I, $(LIBDIRS)) $(addprefix -D, $(DEFINES))
LD_FLAGS     = -Werror

# Defines
DEFINES += NDEBUG
DEFINES += F_USB=16000000UL
DEFINES += MOOLTIPASS_VERSION="\"v1.0\""

# Avrdude settings Arduino as ISP
#AVRDUDE_PORT       = /dev/ttyACM0
#AVRDUDE_PROGRAMMER = stk500v1
#AVRDUDE_FLAGS      = -b 19200 -e -Ulock:w:0x3F:m -Uefuse:w:0xFB:m -Uhfuse:w:0xD9:m -Ulfuse:w:0xFF:m

# Avrdude settings Arduino Bootloader
AVRDUDE_PORT       = /dev/ttyACM0
AVRDUDE_PROGRAMMER = avr109
AVRDUDE_FLAGS      = -b 115200

# Special optimization (most important at the top)
COMPILER_PATH =../AVR-Development-Environment-Script/bin/bin/
CC_FLAGS  += -flto -fuse-linker-plugin
LD_FLAGS  += -flto -fuse-linker-plugin
CC_FLAGS  += -mcall-prologues
CC_FLAGS  += -fno-split-wide-types
#TODO useless?
CC_FLAGS  += -fno-tree-scev-cprop

# Some helpers to get all the .c and .h files.
# $(TARGET) is included inside $(OBJECTS).
LIBDIRS := $(addprefix ./, GUI CARD FLASH USB SPI OLEDMP UTILS AES NODEMGMT RNG PWM TOUCH LOGIC OLEDMINI)
LIBOBJS := $(foreach lib, $(LIBDIRS), $(wildcard $(lib)/*.c))
OBJECTS := $(wildcard ./*.c)

# Default target
all:

# Include DMBS build script makefiles
DMBS_PATH   ?= ../dmbs/DMBS
include $(DMBS_PATH)/core.mk
include $(DMBS_PATH)/gcc.mk
include $(DMBS_PATH)/cppcheck.mk
include $(DMBS_PATH)/doxygen.mk
include $(DMBS_PATH)/dfu.mk
include $(DMBS_PATH)/hid.mk
include $(DMBS_PATH)/avrdude.mk
include $(DMBS_PATH)/atprogram.mk
