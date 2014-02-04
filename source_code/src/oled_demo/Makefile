ARDUINODIR:=/Applications/Arduino.marlin.app/Contents/Resources/Java
AVRTOOLSPATH := $(ARDUINODIR)/hardware/tools/avr/bin
SOURCES := oled_demo.ino 
LIBRARIES := SPI oled256
BOARD := mighty_opt
ADD_LIBRARYPATH := ../lib
#BOARD := atmega328
CPPFLAGS = -std=gnu99

include arduino.mk
