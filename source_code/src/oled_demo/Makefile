ARDUINODIR:=/Applications/Arduino.app/Contents/Resources/Java
AVRTOOLSPATH := $(ARDUINODIR)/hardware/tools/avr/bin
SOURCES := oled_demo.ino 
LIBRARIES := spi oledmp
BOARD := leonardo
ADD_LIBRARYPATH := $(PWD)/../lib
AVRDUDEFLAGS := -vv

include arduino.mk
