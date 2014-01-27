/*-
 * Copyright (c) 2014 Darran Hunt (darran [at] hunt dot net dot nz)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file OledMP Mooltipass 256x64x4 OLED display driver
 */

#include <SPI.h>
#include "oledmp.h"

#include <avr/pgmspace.h>
/* Work around a bug with PROGMEM and PSTR where the compiler always
 * generates warnings.
 */
#undef PROGMEM 
#define PROGMEM __attribute__(( section(".progmem.data") )) 
#undef PSTR 
#define PSTR(s) (__extension__({static prog_char __c[] PROGMEM = (s); &__c[0];})) 

#define pinLow(port, pin)	*port &= ~pin
#define pinHigh(port, pin)	*port |= pin

#define MIN_SEG 28
#define MAX_SEG 91

#define OLED_WIDTH 256
#define OLED_HEIGHT 64

OledMP::OledMP(const uint8_t cs, const uint8_t dc, const uint8_t reset)
{
    _cs = cs;
    _dc = dc;
    _reset = reset;
    foreground = 15;
    background = 0;
    cur_x = 0;
    cur_y = 0;
    end_x = 0;
    end_y = 0;
    wrap = true;
    _offset = 0;
    _bufHeight = OLED_HEIGHT;
    _fontHQ = NULL;
}

void OledMP::setColour(uint8_t colour)
{
    foreground = colour & 0x0F;
}

void OledMP::setBackground(uint8_t colour)
{
    background = colour & 0x0F;
}

void OledMP::setContrast(uint8_t contrast)
{
    writeCommand(CMD_SET_CONTRAST_CURRENT);
    writeData(contrast);
}

void OledMP::putstr(char *str)
{
    char ch;

    while ((ch=*str++) != 0) {
	if (ch == '\n') {
	    cur_y += glyphHeight();
	    cur_x = 0;
	} else if (ch == '\r') {
	    // skip em
	} else {
	    uint8_t width = glyphWidth(ch);
	    if (wrap && ((cur_x + width) > OLED_WIDTH)) {
		cur_y += glyphHeight();
		cur_x = 0;
	    }
	    cur_x += glyphDraw(cur_x, cur_y, ch, foreground, background);
	}
    }
}


int OledMP::printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int ret = vsnprintf(_printBuf, sizeof(_printBuf), fmt, ap);
      
    putstr(_printBuf);

    return ret;
} 

int OledMP::printf(const __fstr *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int ret = vsnprintf_P(_printBuf, sizeof(_printBuf), (const char *)fmt, ap);
      
    putstr(_printBuf);

    return ret;
} 

void OledMP::setXY(uint8_t col, uint8_t row)
{
    cur_x = col;
    cur_y = row;
}


void OledMP::begin(uint8_t font)
{
    setFont(font);
    port_cs = portOutputRegister(digitalPinToPort(_cs));
    pin_cs = digitalPinToBitMask(_cs);
    port_dc = portOutputRegister(digitalPinToPort(_dc));
    pin_dc = digitalPinToBitMask(_dc);

    pinMode(_cs, OUTPUT);
    pinMode(_dc, OUTPUT);
    pinMode(_reset, OUTPUT);
    digitalWrite(_cs,HIGH);

    reset();
    init();

    for (uint8_t ind=0; ind<LCDHEIGHT; ind++) {
	gddram[ind].xaddr = 0;
	gddram[ind].pixels = 0;
    }

    clear();
}

/**
 * Initialise the OLED hardware and get it ready for use.
 */
void OledMP::init()
{

    writeCommand(CMD_SET_COMMAND_LOCK);
    writeData(0x12); /* Unlock OLED driver IC*/

    writeCommand(CMD_SET_DISPLAY_OFF);

    writeCommand(CMD_SET_CLOCK_DIVIDER);
    writeData(0x91);

    writeCommand(CMD_SET_MULTIPLEX_RATIO);
    writeData(0x3F); /*duty = 1/64*,64 COMS are enabled*/

    writeCommand(CMD_SET_DISPLAY_OFFSET);
    writeData(0x00);

    writeCommand(CMD_SET_DISPLAY_START_LINE); /*set start line position*/
    writeData(0x00);

    writeCommand(CMD_SET_REMAP);
    writeData(0x14);	//Horizontal address increment,Disable Column Address Re-map,Enable Nibble Re-map,Scan from COM[N-1] to COM0,Disable COM Split Odd Even
    writeData(0x11);	//Enable Dual COM mode

    /*writeCommand(0xB5); //GPIO

    writeCommand(0x00); */

    writeCommand(CMD_SET_FUNCTION_SELECTION);
    writeData(0x01); /* selection external VDD */

    writeCommand(CMD_DISPLAY_ENHANCEMENT);
    writeData(0xA0);	/*enables the external VSL*/
    writeData(0xfd);	/*0xfd,Enhanced low GS display quality;default is 0xb5(normal),*/

    writeCommand(CMD_SET_CONTRAST_CURRENT);
    writeData(0xff); /* 0xff */	/*default is 0x7f*/

    writeCommand(CMD_MASTER_CURRENT_CONTROL); 
    writeData(0x0f);	/*default is 0x0f*/

    /* writeCommand(0xB9); GRAY TABLE,linear Gray Scale*/

    writeCommand(CMD_SET_PHASE_LENGTH);
    writeData(0xE2);	 /*default is 0x74*/

    writeCommand(CMD_DISPLAY_ENHANCEMENT_B);
    writeData(0x82);	 /*Reserved;default is 0xa2(normal)*/
    writeData(0x20);

    writeCommand(CMD_SET_PRECHARGE_VOLTAGE);
    writeData(0x1F);	 /*0.6xVcc*/

    writeCommand(CMD_SET_SECOND_PRECHARGE_PERIOD);
    writeData(0x08);	 /*default*/

    writeCommand(CMD_SET_VCOMH_VOLTAGE	);
    writeData(0x07);	 /*0.86xVcc;default is 0x04*/

    writeCommand(CMD_SET_DISPLAY_MODE_NORMAL);

    writeCommand(CMD_SET_DISPLAY_ON);
}

/**
 * Write a command or register address to the display
 * @param reg - the command or register to write
 */
void OledMP::writeCommand(uint8_t reg)
{
    pinLow(port_cs, pin_cs);
    pinLow(port_dc, pin_dc);
    SPI.transfer(reg);
    pinHigh(port_cs, pin_cs);
}

/**
 * Write a byte of data to the display
 * @param data - data to write
 */
void OledMP::writeData(uint8_t data)
{
    pinLow(port_cs, pin_cs);
    pinHigh(port_dc, pin_dc);
    SPI.transfer(data);
    pinHigh(port_cs, pin_cs);
}

/**
 * Set the current pixel data column start and end address
 * @param start - start column
 * @param end - end column
 */
void OledMP::setColumnAddr(uint8_t start, uint8_t end)
{
    writeCommand(CMD_SET_COLUMN_ADDR);
    writeData(start);
    writeData(end);
}

/**
 * Set the current pixel data row start and end address
 * @param start - start row
 * @param end - end row
 */
void OledMP::setRowAddr(uint8_t start, uint8_t end)
{
    writeCommand(CMD_SET_ROW_ADDR);
    writeData(start);
    writeData(end);
}

/**
 * Set the current pixel data window.
 * Data writes will update only this section of the display.
 * @param x - start row
 * @param y - start column
 * @param xend - end row
 * @param yend - end column
 */
void OledMP::setWindow(uint8_t x, uint8_t y, uint8_t xend, uint8_t yend)
{
    setColumnAddr(MIN_SEG + x / 4, MIN_SEG + xend / 4);
    setRowAddr(y, yend);
    //cur_x = x;
    end_x = xend;
    //cur_y = y;
    end_y = yend;
}

/**
 * Set the display pixel row offset.  Can be used to scroll the display.
 * Effectively moves y=0 to the offset y row.  The display wraps around to y=63.
 * @param offset - set y origin to this offset
 */
void OledMP::setOffset(uint8_t offset)
{
    writeCommand(CMD_SET_DISPLAY_OFFSET);
    writeData(offset);
    _offset = offset;
}

/**
 * Get the current display offset (y origin).
 * @returns current y offset
 */
uint8_t OledMP::getOffset(void)
{
    return _offset;
}

void OledMP::setBufHeight(uint8_t rows)
{
    if (rows < LCDHEIGHT) {
	return;
    }
    writeCommand(CMD_SET_MULTIPLEX_RATIO);
    writeData(rows & 0x7F);
    _bufHeight = rows;
}

uint8_t OledMP::getBufHeight(void)
{
    return _bufHeight;
}


/**
 * Set the font to use
 * @param font - new font to use
 */
void OledMP::setFont(uint8_t font)
{
    _fontHQ = &fontsHQ[font];
}


/**
 * Fill the display with the specified colour by setting
 * every pixel to the colour.
 * @param colour - fill the display with this colour.
 */
void OledMP::fill(uint8_t colour)
{
    uint8_t x,y;
    setColumnAddr(MIN_SEG, MAX_SEG);	// SEG0 - SEG479
    setRowAddr(0, 63);	

    colour = (colour & 0x0F) | (colour << 4);;

    writeCommand(CMD_WRITE_RAM);
    for(y=0; y<64; y++)
    {
	for(x=0; x<64; x++) {
	    writeData(colour);
	    writeData(colour);
	}
    }
    delay(1);
}

/**
 * Clear the display by setting every pixel to the background colour.
 */
void OledMP::clear()
{
    fill(background);

    for (uint8_t ind=0; ind<LCDHEIGHT; ind++) {
	gddram[ind].xaddr = 0;
	gddram[ind].pixels = 0;
    }
}

/**
 * Reset the OLED display.
 */
void OledMP::reset()
{
    digitalWrite(_reset,LOW);
    delay(10);
    digitalWrite(_reset,HIGH);
    delay(10);
}


/**
 * Return the width of the specified character in the current font.
 * @param ch - return the width of this character
 * @returns width of the glyph
 */
uint8_t OledMP::glyphWidth(char ch)
{
    if (_fontHQ) {
	uint8_t gind;
	if (ch >= ' ' && ch <= 0x7f) {
	    gind = pgm_read_byte(&_fontHQ->map[ch - ' ']);
	} else {
	    // default to a space
	    gind = 0;
	}
	return (uint8_t)pgm_read_byte(&(_fontHQ->glyphs[gind].width));
    } else {
	return 0;
    }
}

/**
 * Return the height of the current font. All characters in a font are the same height.
 * @returns height of the glyph font
 */
uint8_t OledMP::glyphHeight()
{
    if (_fontHQ) {
	return _fontHQ->height;
    } else {
	return 0;
    }
}

/**
 * Draw a character glyph on the screen at x,y.
 * @param x - x position to start glyph (x=0 for left, x=256-glyphWidth() for right)
 * @param y - y position to start glyph (y=0 for top, y=64-glyphHeight() for bottom)
 * @param ch - the character to draw
 * @param colour - foreground colour
 * @param bg - background colour
 * @returns width of the glyph
 * @note currently only works with fixed width glyphs due to display buffer used
 *       to determine the adjacent glyph when updating a shared cell (two pixels per byte)
 */
uint8_t OledMP::glyphDraw(int16_t x, int16_t y, char ch, uint16_t colour, uint16_t bg)
{
    uint8_t pix;
    int ind;
    const uint8_t *glyph;
    uint8_t glyph_width;
    uint8_t glyph_height;
    int8_t glyph_offset;
    uint8_t gind;
    uint8_t byteWidth=0;

    if (_fontHQ == NULL) {
	return 0;
    }

    if (colour == bg) {
	bg = 0;
    }

    // get glyph index
    if (ch >= ' ' && ch <= 0x7f) {
	gind = pgm_read_byte(&_fontHQ->map[ch - ' ']);
    } else {
	// default to a space
	gind = 0;
    }

    glyph = (const uint8_t *)pgm_read_word(&_fontHQ->glyphs[gind].glyph);

    if (glyph == NULL) {
	// space character, just fill in the gddram buffer and output background pixels
	glyph_width = (uint8_t)pgm_read_byte(&(_fontHQ->glyphs[gind].width));
	glyph_height = _fontHQ->height;
	glyph_offset = 0;
    } else {
	glyph_width = (uint8_t)pgm_read_byte(&(_fontHQ->glyphs[gind].xrect));
	glyph_height = (uint8_t)pgm_read_byte(&(_fontHQ->glyphs[gind].yrect));
	glyph_offset = (int8_t)pgm_read_byte(&(_fontHQ->glyphs[gind].xoffset));
	x += glyph_offset;
	y += (int8_t) pgm_read_byte(&(_fontHQ->glyphs[gind].yoffset));
	if (x < 0) x = 0;
	if (y < 0) y = 0;
    }

    uint8_t xoff = x & 0x3;

    byteWidth = (x+glyph_width-1)/4 - (x/4) + 1;

    setWindow(x, y, x+glyph_width-1, y+glyph_height-1);

    writeCommand(CMD_WRITE_RAM);

    // build pixel gddramdata
    for (uint16_t yind=0; yind<glyph_height; yind++) {
	ind = (uint8_t)(yind*glyph_width);
	uint16_t pixels;
	// check for shared pixel data to the left
	if ((x/4) == gddram[y+yind].xaddr) {
	    // overlap
	    pixels = gddram[y+yind].pixels >> ((3-xoff)*4);	// 4 pixels
	} else {
	    pixels = 0;
	}

	// write a pixel row
	uint8_t pind=0;
	uint8_t byteCount = byteWidth;
	for (pix=0; pix<glyph_width; pix+=4) {
	    for (pind=0; pind<4; pind++) {
		pixels &= 0xFFF0;
		if (pix+pind < glyph_width) {
		    uint8_t pixel = glyph ? pgm_read_byte(glyph++) : 0;
		    pixels |= pixel;
		}
		if (pind == (3-xoff)) {
		    writeData((uint8_t)(pixels >> 8));
		    writeData((uint8_t)pixels);
		    byteCount--;
		} else {
		    pixels <<= 4;
		}
	    }
	}

	if (byteCount != 0) {
	    // write final pixels
	    pixels <<= (3-xoff)*4;
	    writeData((uint8_t)(pixels >> 8));
	    writeData((uint8_t)pixels);
	}

	// Include blank spacing pixel 
	//pixels <<= 4;

	if ((x+glyph_width) & 0x3) {
	    gddram[y+yind].pixels = pixels;
	} else {
	    // rolled over 
	    gddram[y+yind].pixels = 0;
	}
	gddram[y+yind].xaddr = (x+glyph_width) / 4;
    }

    return (uint8_t)(glyph_width + glyph_offset) + 1;
    //return (uint8_t)pgm_read_byte(&(_fontHQ->glyphs[gind].width));
}


void OledMP::bitmapDraw(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint16_t *image)
{
    setWindow(x, y, x+width-1, y+height-1);
    writeCommand(CMD_WRITE_RAM);

    uint8_t xoff = x - (x / 4) * 4;
    uint16_t pixels;
    uint8_t xind;
    uint8_t byteWidth = (width+3)/4;

    if (xoff == 0) {
	for (uint8_t yind=0; yind < height; yind++) {
	    for (xind=0; xind < byteWidth; xind++) {
		pixels = (uint16_t)pgm_read_byte(&image[yind*byteWidth+xind]) | pgm_read_byte((uint8_t *)&image[yind*byteWidth+xind]+1) << 8;
		writeData((uint8_t)(pixels >> 8));
		writeData((uint8_t)pixels);
	    }
	    gddram[yind].pixels = pixels;
	    gddram[yind].xaddr = xind-1;
	}
    } else {
	for (uint8_t yind=0; yind < height; yind++) {

	    if ((x/4) == gddram[yind].xaddr) {
		// get the existing pixel data to handle the overlap
		pixels = gddram[yind].pixels;
	    } else {
		pixels = 0;
	    }

	    uint16_t imagePixels;
	    for (xind=0; xind < byteWidth; xind ++) {
		// image is offset in gddram, so need to merge it with the previous pixel data
		imagePixels = (uint16_t)pgm_read_byte(&image[yind*byteWidth+xind]) | pgm_read_byte((uint8_t *)&image[yind*byteWidth+xind]+1) << 8;

		pixels |= imagePixels >> (4-xoff) * 4;
		writeData((uint8_t)(pixels >> 8));
		writeData((uint8_t)pixels);
		pixels = imagePixels << 4*xoff;
	    }
	    writeData((uint8_t)(pixels >> 8));
	    writeData((uint8_t)pixels);
	    gddram[yind].pixels = pixels;
	    gddram[yind].xaddr = xind-1;
	}
    }
}
