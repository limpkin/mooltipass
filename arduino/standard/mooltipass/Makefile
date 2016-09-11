ARDUINODIR:=/Applications/Arduino.app/Contents/Resources/Java
AVRTOOLSPATH := $(ARDUINODIR)/hardware/tools/avr/bin
SOURCES := mooltipass.ino 
LIBRARIES := spi oledmp
BOARD := leonardo
ADD_LIBRARYPATH := $(PWD)/../libraries
AVRDUDEFLAGS := -vv

.SECONDARY:

include arduino.mk
