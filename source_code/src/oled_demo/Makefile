ARDUINODIR:=/Applications/Arduino.app/Contents/Resources/Java
AVRTOOLSPATH := $(ARDUINODIR)/hardware/tools/avr/bin
SOURCES := oled_demo.ino 
LIBRARIES := spi oledmp
#BOARD := mighty_opt
BOARD := leonardo
ADD_LIBRARYPATH := ../lib
#BOARD := atmega328
#CPPFLAGS = -std=gnu99
AVRDUDEFLAGS := -vv

include arduino.mk
