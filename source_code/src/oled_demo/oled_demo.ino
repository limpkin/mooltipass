
/*
 * Mooltipass Oled Demo
 */

#include <spi.h>
#include <oledmp.h>
#if 0
#include "sphere.h"
#include "jack.h"
#endif
#include "hackaday.h"
#include "gear.h"

uint8_t const OLED_CS =		 12;
uint8_t const OLED_DC =		 6;
uint8_t const OLED_nRESET =	 2;
uint8_t const OLED_nENABLE_12V = 11;

SPI spi(SPI_BAUD_8_MHZ);
OledMP oled(OLED_CS, OLED_DC, OLED_nRESET, OLED_nENABLE_12V, spi);	

uint32_t count=0;

void setup()
{

    Serial.begin(115200);
    for (uint8_t count=1; count<=10; count++) {
	delay(1000);
	Serial.println(count);
    }

    Serial.println("Ready");

    spi.begin();
    //SPI.setDataMode(SPI_MODE0);
    //SPI.setClockDivider(SPI_CLOCK_DIV2);
    //SPI.setBaud(SPI_BAUD_4);
    Serial.println("SPI ready");

    oled.begin();
    Serial.println("OLED ready");

    oled.setColour(15);
    oled.setBackground(0);
    oled.setContrast(255);

    oled.bitmapDraw(0,0, &image_hackaday);
    oled.setXY(72,37);
    oled.printf(F("HACK A DAY"));
    oled.setXY(108,4);
    oled.printf(F("Mooltipass"));
    oled.bitmapDraw(192,0, &image_gear);
#if 0
    oled.printf(F("ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
    oled.printf(F("abcdefghijklmnopqrstuvwxyz\n"));
    oled.printf(F("01234567890"));
    oled.printf(F("!$%%&'()*,./:;?"));
#endif
    Serial.println("Finished");
}

void loop()
{
}
