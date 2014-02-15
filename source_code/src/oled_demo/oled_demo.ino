
/*
 * Mooltipass Oled Demo
 */

#include <spi.h>
#include <oledmp.h>
#include <util/delay.h>
#include "jack.h"
#include "aqua.h"
#include "sphere.h"
#include "hackaday.h"
#include "gear.h"

uint8_t const OLED_CS =		 6;	// PD6 (D12)
uint8_t const OLED_DC =		 7;	// PD7 (D6)
uint8_t const OLED_nRESET =	 1;	// PD1 (D2)
uint8_t const OLED_nENABLE_12V = 7;	// PB7 (D11)

SPI spi(SPI_BAUD_8_MHZ);
OledMP oled(&PORTD, OLED_CS, &PORTD, OLED_DC, &PORTD, OLED_nRESET, &PORTB, OLED_nENABLE_12V, spi);	

uint32_t count=0;

void setup()
{
    //_delay_ms(5000);

    Serial.begin(115200);
    Serial.println("Ready");

    spi.begin();
    Serial.println("SPI ready");

    oled.begin();
    Serial.println("OLED ready");

    oled.setColour(2);
    oled.setBackground(0);
    //oled.setContrast(80);
    oled.setContrast(255);

#if 1
    oled.bitmapDraw(0,0, &image_hackaday);
    oled.setXY(72,37);
    oled.printf(F("HACK A DAY"));
    oled.setXY(108,4);
    oled.printf(F("Mooltipass"));
    oled.bitmapDraw(192,0, &image_gear);
#endif


#if 0
    oled.bitmapDraw(0,0, &image_hackaday);
    oled.bitmapDraw(60,0, &image_sphere);
    oled.bitmapDraw(116,0, &image_aqua);
    oled.bitmapDraw(192,0, &image_gear);
#endif

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
