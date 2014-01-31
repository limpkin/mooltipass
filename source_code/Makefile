CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump

# Object files and target name.
OBJECTS = src/CARD/smart_card_higher_level_functions.o \
		  src/CARD/smartcard.o \
		  src/OLED/oled.o \
		  src/USB/teensy_usb.o \
		  src/USB/usb_serial.o \
		  src/interrupts.o \
		  src/FLASH/flash_mem.o \
		  src/UTILS/utils.o \
		  src/avr_project.o \
		  src/GRAPHICS/graphics.o
TARGET = mooltipass

# Compiler and linker configuration.
FORMAT=ihex

# Set CONFIG=DEBUG in the shell from which you invoke make to build a debug
# build.
CONFIG  ?= RELEASE
DEFINES := F_CPU=16000000UL F_USB=16000000UL

CFLAGS  := -Wall -mmcu=atmega32u4 -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
LDFLAGS := -lm

ifeq ($(CONFIG), RELEASE)
  DEFINES += NDEBUG
  CFLAGS  += -Os
else ifeq ($(CONFIG), DEBUG)
  DEFINES += DEBUG
  CFLAGS  += -O1 -g2 -Wa,-g
endif

# Add all defines, each prefixed with -D.
CFLAGS += $(foreach def,$(DEFINES),$(def:%=-D%))

.PHONY: all
all: $(TARGET).hex $(TARGET).eep $(TARGET).lss

# Pattern rule to compile C sources into object files. TODO: Use GCC dependency
# generation instead of the simplified foo.o: foo.c foo.h.
%.o: %.c %.h
	$(CC) -o $@ -c $(CFLAGS) $<

# Target to link the object files into the final hex file.
$(TARGET).elf: $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Extract the sections that should be in the .hex file from the .elf file.
%.hex: %.elf
	$(OBJCOPY) -O $(FORMAT) -R .eeprom -R .fuse -R .lock -R .signature $< $@

# Extract the eeprom parts from the .elf file.
%.eep: %.elf
	-$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" \
	--change-section-lma .eeprom=0 --no-change-warnings -O $(FORMAT) $< $@ || exit 0

%.lss: %.elf
	$(OBJDUMP) -h -S -z $< > $@

.PHONY: clean
clean:
	-rm -f $(OBJECTS) $(TARGET).hex $(TARGET).eep $(TARGET).lss $(TARGET).elf
