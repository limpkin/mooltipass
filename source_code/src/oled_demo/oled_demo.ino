
/*
 * Mooltipass Oled Demo
 */

#include <SPI.h>
#include <oled256.h>

#include "hackaday.h"
//#include "jack.h"
#include "gear.h"

#define LCD_WIDTH 32
#define LCD_HEIGHT 5

LcdDisplay lcd(17, 16, 10);	// cs, data_command, reset

void setup()
{
    Serial.begin(115200);
    pinMode(4, OUTPUT);
    digitalWrite(4, LOW);

    SPI.begin();
    SPI.setDataMode(SPI_MODE0);
    SPI.setClockDivider(SPI_CLOCK_DIV2);

    lcd.begin(LCD_WIDTH, LCD_HEIGHT, FONT_NINE_DOT);
    //lcd.begin(LCD_WIDTH, LCD_HEIGHT, FONT_FIFTEEN_DOT);
    lcd.setColour(15);
    lcd.setBackground(0);
    lcd.setContrast(255);

    lcd.setCursor(1,0);

    lcd.bitmapDraw(0,0, HACKADAY_WIDTH, HACKADAY_HEIGHT, hackaday_image);
    lcd.bitmapDraw(192,0, GEAR_WIDTH, GEAR_HEIGHT, gear_image);
    //lcd.bitmapDraw(160,0, JACK_WIDTH, JACK_HEIGHT, jack_image);
    //lcd.bitmapDraw(160,0,SPHERE_WIDTH,SPHERE_HEIGHT,sphere_image);
    lcd.setFontHQ(font_CHECKBOOK_14);
    lcd.setXY(108,4);
    lcd.print(F("Mooltipass"));
    lcd.setXY(72,37);
    lcd.print(F("HACK A DAY"));
}

void loop()
{
}
