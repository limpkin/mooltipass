
/*
 * Mooltipass Oled Demo
 */

#include <SPI.h>
#include <oledmp.h>
#if 0
#include "sphere.h"
#include "jack.h"
#endif
#include "hackaday.h"
#include "gear.h"

OledMP oled(17, 16, 10);	// cs, data_command, reset

void setup()
{
    Serial.begin(115200);
    pinMode(4, OUTPUT);
    digitalWrite(4, LOW);

    SPI.begin();
    SPI.setDataMode(SPI_MODE0);
    SPI.setClockDivider(SPI_CLOCK_DIV2);

    oled.begin();

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
}

void loop()
{
}
