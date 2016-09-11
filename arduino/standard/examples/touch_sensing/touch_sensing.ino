#include <mooltipass_touch_sensing.h>
#include <usart_spi.h>
#include <oledmp.h>
#include <Wire.h>

#define RELAY_PIN  5
mooltipass_touch_sensing touch;
USARTSPI spi(SPI_BAUD_8_MHZ);
OledMP oled(spi);	

void setup()
{
    spi.begin();
    oled.begin();
    touch.begin();    
    Serial.begin(9600);
    pinMode(RELAY_PIN, OUTPUT);
    oled.printf(F("TOUCH sketch\n"));
}

void loop()
{
  uint8_t return_val = touch.touchDetectionRoutine(0);
  
  if(return_val & RETURN_LEFT_PRESSED)
  {
    digitalWrite(RELAY_PIN, 0);
  }
  else if(return_val & RETURN_RIGHT_PRESSED)
  {
    digitalWrite(RELAY_PIN, 1);
  }
    
}
