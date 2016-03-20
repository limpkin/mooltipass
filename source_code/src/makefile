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
SRC          =  $(LIBOBJS) $(OBJECTS)
CC_FLAGS     = -flto -fuse-linker-plugin $(addprefix -I, $(LIBDIRS)) -DF_USB=16000000UL -DNDEBUG -DMOOLTIPASS_VERSION="\"v0.8-856-g959c6ea-dirty\""
LD_FLAGS     = -flto -fuse-linker-plugin

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

test:
	echo $(LIBDIRS)
	echo "----"
	echo $(LIBOBJS)
	echo "----"
	echo $(OBJECTS)
