#include <mooltipass_touch_sensing.h>
#include <usart_spi.h>
#include <oledmp.h>
#include <Wire.h>

mooltipass_touch_sensing touch;
USARTSPI spi(SPI_BAUD_8_MHZ);
OledMP oled(spi);	

void setup()
{
    spi.begin();
    oled.begin();
    touch.begin();    
    Serial.begin(9600);
    oled.printf(F("TOUCH sketch\n"));
}

void loop()
{
    touch.touchDetectionRoutine(0);
}
