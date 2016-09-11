#include <usart_spi.h>
#include <oledmp.h>

USARTSPI spi(SPI_BAUD_8_MHZ);
OledMP oled(spi);	

void setup()
{
    spi.begin();
    oled.begin();
    oled.printf(F("Hello world!\n"));
}

void loop()
{
}
