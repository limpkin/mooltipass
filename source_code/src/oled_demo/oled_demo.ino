
/*
 * Mooltipass Oled Demo
 */

#include <SPI.h>
#include <oledmp.h>

#include "hackaday.h"
//#include "jack.h"
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

    oled.begin(font_CHECKBOOK_14);

    oled.setColour(15);
    oled.setBackground(0);
    oled.setContrast(255);

    oled.bitmapDraw(0,0, HACKADAY_WIDTH, HACKADAY_HEIGHT, hackaday_image);
    oled.bitmapDraw(192,0, GEAR_WIDTH, GEAR_HEIGHT, gear_image);
    //oled.bitmapDraw(160,0, JACK_WIDTH, JACK_HEIGHT, jack_image);
    //oled.bitmapDraw(160,0,SPHERE_WIDTH,SPHERE_HEIGHT,sphere_image);
    oled.setXY(108,4);
    oled.printf(F("Mooltipass"));
    oled.setXY(72,37);
    oled.printf(F("HACK A DAY"));
}

void loop()
{
}
