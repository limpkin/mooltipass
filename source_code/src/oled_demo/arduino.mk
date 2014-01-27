#_______________________________________________________________________________
#
#                         edam's Arduino makefile
#_______________________________________________________________________________
#                                                                    version 0.5
#
# Copyright (C) 2011, 2012, 2013 Tim Marston <tim@ed.am>.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
#_______________________________________________________________________________
#
#
# This is a general purpose makefile for use with Arduino hardware and
# software.  It works with the arduino-1.0 and later software releases.  It
# should work GNU/Linux and OS X.  To download the latest version of this
# makefile visit the following website where you can also find documentation on
# it's use.  (The following text can only really be considered a reference.)
#
#   http://ed.am/dev/make/arduino-mk
#
# This makefile can be used as a drop-in replacement for the Arduino IDE's
# build system.  To use it, just copy arduino.mk in to your project directory.
# Or, you could save it somewhere (I keep mine at ~/src/arduino.mk) and create
# a symlink to it in your project directory, named "Makefile".  For example:
#
#   $ ln -s ~/src/arduino.mk Makefile
#
# The Arduino software (version 1.0 or later) is required.  On GNU/Linux you
# can probably install the software from your package manager.  If you are
# using Debian (or a derivative), try `apt-get install arduino`.  Otherwise,
# you can download the Arduino software manually from http://arduino.cc/.  It
# is suggested that you install it at ~/opt/arduino (or /Applications on OS X)
# if you are unsure.
#
# If you downloaded the Arduino software manually and unpacked it somewhere
# other than ~/opt/arduino (or /Applications), you will need to set up the
# ARDUINODIR environment variable to be the path where you unpacked it.  (If
# unset, ARDUINODIR defaults to some sensible places).  You could set this in
# your ~/.profile by adding something like this:
#
#   export ARDUINODIR=~/somewhere/arduino-1.0
#
# For each project, you will also need to set BOARD to the type of Arduino
# you're building for.  Type `make boards` for a list of acceptable values.
# For example:
#
#   $ export BOARD=uno
#   $ make
#
# You may also need to set SERIALDEV if it is not detected correctly.
#
# The presence of a .ino (or .pde) file causes the arduino.mk to automatically
# determine values for SOURCES, TARGET and LIBRARIES.  Any .c, .cc and .cpp
# files in the project directory (or any "util" or "utility" subdirectories)
# are automatically included in the build and are scanned for Arduino libraries
# that have been #included.  Note, there can only be one .ino (or .pde) file in
# a project directory and if you want to be compatible with the Arduino IDE, it
# should be called the same as the directory name.
#
# Alternatively, if you want to manually specify build variables, create a
# Makefile that defines SOURCES and LIBRARIES and then includes arduino.mk.
# (There is no need to define TARGET).  You can also specify the BOARD here, if
# the project has a specific one.  Here is an example Makefile:
#
#   SOURCES := main.cc other.cc
#   LIBRARIES := EEPROM
#   BOARD := pro5v
#   include ~/src/arduino.mk
#
# Here is a complete list of configuration parameters:
#
# ARDUINODIR   The path where the Arduino software is installed on your system.
#
# ARDUINOCONST The Arduino software version, as an integer, used to define the
#              ARDUINO version constant.  This defaults to 100 if undefined.
#
# AVRDUDECONF  The avrdude.conf to use.  If undefined, this defaults to a guess
#              based on where avrdude is.  If set empty, no avrdude.conf is
#              passed to avrdude (so the system default is used).
#
# AVRDUDEFLAGS Specify any additional flags for avrdude.  The usual flags,
#              required to build the project, will be appended to this.
#
# AVRTOOLSPATH A space-separated list of directories that is searched in order
#              when looking for the avr build tools.  This defaults to PATH,
#              followed by subdirectories in ARDUINODIR.
#
# BOARD        Specify a target board type.  Run `make boards` to see available
#              board types.
#
# CPPFLAGS     Specify any additional flags for the compiler.  The usual flags,
#              required to build the project, will be appended to this.
#
# LINKFLAGS    Specify any additional flags for the linker.  The usual flags,
#              required to build the project, will be appended to this.
#
# LIBRARIES    A list of Arduino libraries to build and include.  This is set
#              automatically if a .ino (or .pde) is found.
#
# LIBRARYPATH  A space-separated list of directories that is searched in order
#              when looking for Arduino libraries.  This defaults to "libs",
#              "libraries" (in the project directory), then your sketchbook
#              "libraries" directory, then the Arduino libraries directory.
#
# SERIALDEV    The POSIX device name of the serial device that is the Arduino.
#              If unspecified, an attempt is made to guess the name of a
#              connected Arduino's serial device, which may work in some cases.
#
# SOURCES      A list of all source files of whatever language.  The language
#              type is determined by the file extension.  This is set
#              automatically if a .ino (or .pde) is found.
#
# TARGET       The name of the target file.  This is set automatically if a
#              .ino (or .pde) is found, but it is not necessary to set it
#              otherwise.
#
# This makefile also defines the following goals for use on the command line
# when you run make:
#
# all          This is the default if no goal is specified.  It builds the
#              target.
#
# target       Builds the target.
#
# upload       Uploads the target (building it, as necessary) to an attached
#              Arduino.
#
# clean        Deletes files created during the build.
#
# boards       Display a list of available board names, so that you can set the
#              BOARD environment variable appropriately.
#
# monitor      Start `screen` on the serial device.  This is meant to be an
#              equivalent to the Arduino serial monitor.
#
# size         Displays size information about the built target.
#
# bootloader   Burns the bootloader for your board to it.
#
# <file>       Builds the specified file, either an object file or the target,
#              from those that that would be built for the project.
#_______________________________________________________________________________

ARDUINODIR?=/Applications/Arduino.app/Contents/Resources/Java
#SKETCHES=$(HOME)/Documents/Arduino
#BOARD=uno
#BAUD=115200

# default arduino software directory, check software exists
ifndef ARDUINODIR
ARDUINODIR := $(firstword $(wildcard ~/opt/arduino /usr/share/arduino \
	/Applications/Arduino.app/Contents/Resources/Java \
	$(HOME)/Applications/Arduino.app/Contents/Resources/Java))
endif
ifeq "$(wildcard $(ARDUINODIR)/hardware/arduino/boards.txt)" ""
$(error ARDUINODIR is not set correctly; arduino software not found)
endif

# default arduino version
ARDUINOCONST ?= 105

# default path for avr tools
AVRTOOLSPATH ?= $(subst :, , $(PATH)) $(ARDUINODIR)/hardware/tools \
	$(ARDUINODIR)/hardware/tools/avr/bin

# default path to find libraries
LIBRARYPATH ?= libraries libs $(SKETCHBOOKDIR)/libraries $(ARDUINODIR)/libraries

# default serial device to a poor guess (something that might be an arduino)
SERIALDEVGUESS := 0
ifndef SERIALDEV
SERIALDEV := $(firstword $(wildcard \
	/dev/ttyACM? /dev/ttyUSB? /dev/tty.usbserial* /dev/tty.usbmodem*))
SERIALDEVGUESS := 1
endif

# no board?
ifndef BOARD
ifneq "$(MAKECMDGOALS)" "boards"
ifneq "$(MAKECMDGOALS)" "clean"
$(error BOARD is unset.  Type 'make boards' to see possible values)
endif
endif
endif

# obtain board parameters from the arduino boards.txt file
BOARDSFILE := $(ARDUINODIR)/hardware/arduino/boards.txt
readboardsparam = $(shell sed -ne "s/$(BOARD).$(1)=\(.*\)/\1/p" $(BOARDSFILE))
BOARD_BUILD_MCU := $(call readboardsparam,build.mcu)
BOARD_BUILD_FCPU := $(call readboardsparam,build.f_cpu)
BOARD_BUILD_VARIANT := $(call readboardsparam,build.variant)
BOARD_UPLOAD_SPEED := $(call readboardsparam,upload.speed)
BOARD_UPLOAD_PROTOCOL := $(call readboardsparam,upload.protocol)
BOARD_USB_VID := $(call readboardsparam,build.vid)
BOARD_USB_PID := $(call readboardsparam,build.pid)
BOARD_BOOTLOADER_UNLOCK := $(call readboardsparam,bootloader.unlock_bits)
BOARD_BOOTLOADER_LOCK := $(call readboardsparam,bootloader.lock_bits)
BOARD_BOOTLOADER_LFUSES := $(call readboardsparam,bootloader.low_fuses)
BOARD_BOOTLOADER_HFUSES := $(call readboardsparam,bootloader.high_fuses)
BOARD_BOOTLOADER_EFUSES := $(call readboardsparam,bootloader.extended_fuses)
BOARD_BOOTLOADER_PATH := $(call readboardsparam,bootloader.path)
BOARD_BOOTLOADER_FILE := $(call readboardsparam,bootloader.file)

# obtain preferences from the IDE's preferences.txt
PREFERENCESFILE := $(firstword $(wildcard \
	$(HOME)/.arduino/preferences.txt $(HOME)/Library/Arduino/preferences.txt))
ifneq "$(PREFERENCESFILE)" ""
readpreferencesparam = $(shell sed -ne "s/$(1)=\(.*\)/\1/p" $(PREFERENCESFILE))
SKETCHBOOKDIR := $(call readpreferencesparam,sketchbook.path)
endif

# invalid board?
ifeq "$(BOARD_BUILD_MCU)" ""
ifneq "$(MAKECMDGOALS)" "boards"
ifneq "$(MAKECMDGOALS)" "clean"
$(error BOARD is invalid.  Type 'make boards' to see possible values)
endif
endif
endif

# auto mode?
INOFILE := $(wildcard *.ino *.pde)
ifdef INOFILE
ifneq "$(words $(INOFILE))" "1"
$(error There is more than one .pde or .ino file in this directory!)
endif

# automatically determine sources and targeet
TARGET := $(basename $(INOFILE))
SOURCES := $(INOFILE) \
	$(wildcard *.c *.cc *.cpp *.C) \
	$(wildcard $(addprefix util/, *.c *.cc *.cpp *.C)) \
	$(wildcard $(addprefix utility/, *.c *.cc *.cpp *.C))

# automatically determine included libraries
LIBRARIES := $(filter $(notdir $(wildcard $(addsuffix /*, $(LIBRARYPATH)))), \
	$(shell sed -ne "s/^ *\# *include *[<\"]\(.*\)\.h[>\"]/\1/p" $(SOURCES)))

endif

# software
findsoftware = $(firstword $(wildcard $(addsuffix /$(1), $(AVRTOOLSPATH))))
CC := $(call findsoftware,avr-gcc)
CXX := $(call findsoftware,avr-g++)
LD := $(call findsoftware,avr-ld)
AR := $(call findsoftware,avr-ar)
OBJCOPY := $(call findsoftware,avr-objcopy)
AVRDUDE := $(call findsoftware,avrdude)
AVRSIZE := $(call findsoftware,avr-size)

# directories
ARDUINOCOREDIR := $(ARDUINODIR)/hardware/arduino/cores/arduino
LIBRARYDIRS := $(foreach lib, $(LIBRARIES), \
	$(firstword $(wildcard $(addsuffix /$(lib), $(LIBRARYPATH)))))
LIBRARYDIRS += $(addsuffix /utility, $(LIBRARYDIRS))

# files
TARGET := $(if $(TARGET),$(TARGET),a.out)
OBJECTS := $(addsuffix .o, $(basename $(SOURCES)))
DEPFILES := $(patsubst %, .dep/%.dep, $(SOURCES))
ARDUINOLIB := .lib/arduino.a
ARDUINOLIBOBJS := $(foreach dir, $(ARDUINOCOREDIR) $(LIBRARYDIRS), \
	$(patsubst %, .lib/%.o, $(wildcard $(addprefix $(dir)/, *.c *.cpp))))
BOOTLOADERHEX := $(addprefix \
	$(ARDUINODIR)/hardware/arduino/bootloaders/$(BOARD_BOOTLOADER_PATH)/, \
	$(BOARD_BOOTLOADER_FILE))

# avrdude confifuration
ifeq "$(AVRDUDECONF)" ""
ifeq "$(AVRDUDE)" "$(ARDUINODIR)/hardware/tools/avr/bin/avrdude"
AVRDUDECONF := $(ARDUINODIR)/hardware/tools/avr/etc/avrdude.conf
else
AVRDUDECONF := $(wildcard $(AVRDUDE).conf)
endif
endif

# flags
CPPFLAGS += -Os -Wall -fno-exceptions -ffunction-sections -fdata-sections
CPPFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CPPFLAGS += -mmcu=$(BOARD_BUILD_MCU)
CPPFLAGS += -DF_CPU=$(BOARD_BUILD_FCPU) -DARDUINO=$(ARDUINOCONST)
CPPFLAGS += -DUSB_VID=$(BOARD_USB_VID) -DUSB_PID=$(BOARD_USB_PID)
CPPFLAGS += -I. -Iutil -Iutility -I $(ARDUINOCOREDIR)
CPPFLAGS += -I $(ARDUINODIR)/hardware/arduino/variants/$(BOARD_BUILD_VARIANT)/
CPPFLAGS += $(addprefix -I , $(LIBRARYDIRS))
CPPDEPFLAGS = -MMD -MP -MF .dep/$<.dep
CPPINOFLAGS := -x c++ -include $(ARDUINOCOREDIR)/Arduino.h
AVRDUDEFLAGS += $(addprefix -C , $(AVRDUDECONF)) -DV
AVRDUDEFLAGS += -p $(BOARD_BUILD_MCU) -P $(SERIALDEV)
AVRDUDEFLAGS += -c $(BOARD_UPLOAD_PROTOCOL) -b $(BOARD_UPLOAD_SPEED)
LINKFLAGS += -Os -Wl,--gc-sections -mmcu=$(BOARD_BUILD_MCU)

# figure out which arg to use with stty (for OS X, GNU and busybox stty)
STTYFARG := $(shell stty --help 2>&1 | \
	grep -q 'illegal option' && echo -f || echo -F)

# include dependencies
ifneq "$(MAKECMDGOALS)" "clean"
-include $(DEPFILES)
endif

# default rule
.DEFAULT_GOAL := all

#_______________________________________________________________________________
#                                                                          RULES

.PHONY:	all target upload clean boards monitor size bootloader

all: target

target: $(TARGET).hex

upload: target
	@echo "\nUploading to board..."
	@test -n "$(SERIALDEV)" || { \
		echo "error: SERIALDEV could not be determined automatically." >&2; \
		exit 1; }
	@test 0 -eq $(SERIALDEVGUESS) || { \
		echo "*GUESSING* at serial device:" $(SERIALDEV); \
		echo; }
ifeq "$(BOARD_BOOTLOADER_PATH)" "caterina"
	stty $(STTYFARG) $(SERIALDEV) speed 1200
	sleep 1
else
	stty $(STTYFARG) $(SERIALDEV) hupcl
endif
	$(AVRDUDE) $(AVRDUDEFLAGS) -U flash:w:$(TARGET).hex:i

clean:
	rm -f $(OBJECTS)
	rm -f $(TARGET).elf $(TARGET).hex $(ARDUINOLIB) *~
	rm -rf .lib .dep

boards:
	@echo "Available values for BOARD:"
	@sed -nEe '/^#/d; /^[^.]+\.name=/p' $(BOARDSFILE) | \
		sed -Ee 's/([^.]+)\.name=(.*)/\1            \2/' \
			-e 's/(.{12}) *(.*)/\1 \2/'

monitor:
	@test -n "$(SERIALDEV)" || { \
		echo "error: SERIALDEV could not be determined automatically." >&2; \
		exit 1; }
	@test -n `which screen` || { \
		echo "error: can't find GNU screen, you might need to install it." >&2 \
		exit 1; }
	@test 0 -eq $(SERIALDEVGUESS) || { \
		echo "*GUESSING* at serial device:" $(SERIALDEV); \
		echo; }
	#screen $(SERIALDEV)
	minicom -D $(SERIALDEV) -o

size: $(TARGET).elf
	echo && $(AVRSIZE) --format=avr --mcu=$(BOARD_BUILD_MCU) $(TARGET).elf

bootloader:
	@echo "Burning bootloader to board..."
	@test -n "$(SERIALDEV)" || { \
		echo "error: SERIALDEV could not be determined automatically." >&2; \
		exit 1; }
	@test 0 -eq $(SERIALDEVGUESS) || { \
		echo "*GUESSING* at serial device:" $(SERIALDEV); \
		echo; }
	stty $(STTYFARG) $(SERIALDEV) hupcl
	$(AVRDUDE) $(AVRDUDEFLAGS) -U lock:w:$(BOARD_BOOTLOADER_UNLOCK):m
	$(AVRDUDE) $(AVRDUDEFLAGS) -eU lfuse:w:$(BOARD_BOOTLOADER_LFUSES):m
	$(AVRDUDE) $(AVRDUDEFLAGS) -U hfuse:w:$(BOARD_BOOTLOADER_HFUSES):m
ifneq "$(BOARD_BOOTLOADER_EFUSES)" ""
	$(AVRDUDE) $(AVRDUDEFLAGS) -U efuse:w:$(BOARD_BOOTLOADER_EFUSES):m
endif
ifneq "$(BOOTLOADERHEX)" ""
	$(AVRDUDE) $(AVRDUDEFLAGS) -U flash:w:$(BOOTLOADERHEX):i
endif
	$(AVRDUDE) $(AVRDUDEFLAGS) -U lock:w:$(BOARD_BOOTLOADER_LOCK):m

# building the target

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

.INTERMEDIATE: $(TARGET).elf

$(TARGET).elf: $(ARDUINOLIB) $(OBJECTS)
	$(CC) $(LINKFLAGS) $(OBJECTS) $(ARDUINOLIB) -lm -o $@

%.o: %.c
	#@echo 1 mkdir -p .dep/$(dir $<)
	#mkdir -p .dep/$(dir $<)
	$(COMPILE.c) $(CPPDEPFLAGS) -o $@ $<

%.o: %.cpp
	#@echo 2 mkdir -p .dep/$(dir $<)
	#mkdir -p .dep/$(dir $<)
	$(COMPILE.cpp) $(CPPDEPFLAGS) -o $@ $<

%.o: %.cc
	#@echo 3 mkdir -p .dep/$(dir $<)
	#mkdir -p .dep/$(dir $<)
	$(COMPILE.cpp) $(CPPDEPFLAGS) -o $@ $<

%.o: %.C
	#@echo 4 mkdir -p .dep/$(dir $<)
	#mkdir -p .dep/$(dir $<)
	$(COMPILE.cpp) $(CPPDEPFLAGS) -o $@ $<

%.o: %.ino
	#@echo 5 mkdir -p .dep/a$(dir $<)
	#@echo 5 mkdir -p .dep/$(<D)a
	mkdir -p .dep/$(<D)a
	$(COMPILE.cpp) $(CPPDEPFLAGS) -o $@ $(CPPINOFLAGS) $<

%.o: %.pde
	#@echo 6 mkdir -p .dep/$(dir $<)
	#mkdir -p .dep/$(dir $<)
	$(COMPILE.cpp) $(CPPDEPFLAGS) -o $@ $(CPPINOFLAGS) $<

# building the arduino library

$(ARDUINOLIB): $(ARDUINOLIBOBJS)
	$(AR) rcs $@ $?

.lib/%.c.o: %.c
	mkdir -p $(dir $@)
	$(COMPILE.c) -o $@ $<

.lib/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(COMPILE.cpp) -o $@ $<

.lib/%.cc.o: %.cc
	mkdir -p $(dir $@)
	$(COMPILE.cpp) -o $@ $<

.lib/%.C.o: %.C
	mkdir -p $(dir $@)
	$(COMPILE.cpp) -o $@ $<

# Local Variables:
# mode: makefile
# tab-width: 4
# End:
