TARGET = oled_demo

BASEDIR := $(PWD)/..

CROSS ?= avr-
CC=$(CROSS)gcc
LD=$(CROSS)ld
STRIP=$(CROSS)strip
AR=$(CROSS)ar
AS=$(CROSS)as
OBJCOPY=$(CROSS)objcopy
OBJDUMP=$(CROSS)objdump
SIZE=$(CROSS)size
AVRDUDE = /Applications/Arduino.app/Contents/Resources/Java/hardware/tools/avr/bin/avrdude

SERIALDEV ?= /dev/tty.usbmodem1411
AVRDUDEFLAGS += -vvvv -C avrdude.conf -DV -p $(MCU) -P $(SERIALDEV) -c avr109 -b 57600

MCU ?= atmega32u4 

# libraries for the target
LIBRARIES := oledmp spi

# list of base paths for libraries
LIBRARYPATHS := $(BASEDIR)/lib

# get full path for each library
LIBDIRS := $(foreach path, $(LIBRARYPATHS), \
    		   $(foreach lib, $(LIBRARIES),  \
		   $(wildcard $(path)/$(lib)/.)))

SRCS := $(wildcard *.c)
OBJS := $(addsuffix .o, $(basename $(SRCS)))
DEPS := $(patsubst %, deps/%.dep, $(SRCS))

MOOLTIPASS_LIBOBJS := $(foreach dir, $(LIBDIRS), $(patsubst %, build/%.o, $(wildcard $(addprefix $(dir)/, *.c *.cpp))))
MOOLTIPASS_LIB := build/mooltipass.a

CFLAGS += -Os -Wall -fno-exceptions -ffunction-sections -fdata-sections
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -std=gnu99
CFLAGS += -mmcu=$(MCU)
CFLAGS += -DF_CPU=16000000UL -DF_USB=16000000UL
CFLAGS += -DUSB_VID=0x2341 -DUSB_PID=0x8036
CFLAGS += -I. $(addprefix -I , $(LIBDIRS))

#LDFLAGS := -Wl,-gc-sections -lm
LDFLAGS += -Os -Wl,--gc-sections -mmcu=$(MCU) -lm

.PHONY: all
all: $(TARGET).hex

# dependency generation to handle headers
%.o: %.c
	mkdir -p deps
	$(COMPILE.c) -MMD -MP -MF deps/$<.dep -o $@ $<

build/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $<

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

$(TARGET).elf: $(MOOLTIPASS_LIB) $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(MOOLTIPASS_LIB) -o $@

$(MOOLTIPASS_LIB): $(MOOLTIPASS_LIBOBJS)
	$(AR) rcs $@ $?

build/%.c.o: %.c
	@echo "mkdir -p $(dir $@)"
	mkdir -p $(dir $@)
	$(COMPILE.c) -o $@ $<

size: $(TARGET).elf
	@echo ""
	$(SIZE) --format=avr --mcu=$(MCU) $(TARGET).elf

upload: $(TARGET).hex
	./reset.py $(SERIALDEV)
	$(AVRDUDE) $(AVRDUDEFLAGS) -U flash:w:$(<):i

.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET).hex $(TARGET).eep $(TARGET).lss $(TARGET).elf *~
	rm -rf build deps
