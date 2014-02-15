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

#include "oledmp.h"
#include <util/delay.h>

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

/*
 * OLED initialisation sequence
 */
static const uint8_t oledInit[] __attribute__((__progmem__)) = {
    CMD_SET_COMMAND_LOCK,            1, 0x12, /* Unlock OLED driver IC*/
    CMD_SET_DISPLAY_OFF,             0,
    CMD_SET_CLOCK_DIVIDER,           1, 0x91,
    CMD_SET_MULTIPLEX_RATIO,         1, 0x3F, /*duty = 1/64*,64 COMS are enabled*/
    CMD_SET_DISPLAY_OFFSET,          1, 0x00,
    CMD_SET_DISPLAY_START_LINE,      1, 0x00, /*set start line position*/
    CMD_SET_REMAP,                   2, 0x14, // Horizontal address increment,
                                              // Disable Column Address Re-map,
					      // Enable Nibble Re-map,Scan from COM[N-1] to COM0,
					      // Disable COM Split Odd Even
                                        0x11, // Enable Dual COM mode
    CMD_SET_GPIO, 		     1, 0x00,
    CMD_SET_FUNCTION_SELECTION,      1, 0x01, /* selection external VDD */
    CMD_DISPLAY_ENHANCEMENT,         2, 0xA0, /* enables the external VSL*/
                                        0xfd, /* 0xfd,Enhanced low GS display quality;default is 0xb5(normal),*/
    CMD_SET_CONTRAST_CURRENT,        1, 0xff, /* default is 0x7f*/
    CMD_MASTER_CURRENT_CONTROL,      1, 0x0f,
    /* writeCommand(0xB9); GRAY TABLE,linear Gray Scale*/
    CMD_SET_PHASE_LENGTH,            1, 0xE2,  /*default is 0x74*/
    CMD_DISPLAY_ENHANCEMENT_B,       2, 0x82, 0x20,
#define ALTERNATE_OLED_VERSION
#ifdef ALTERNATE_OLED_VERSION
    CMD_SET_PRECHARGE_VOLTAGE,       1, 0x08, /* 0.3xVcc */
    CMD_SET_SECOND_PRECHARGE_PERIOD, 1, 0x0F, /* 15 clocks */
#else
    CMD_SET_PRECHARGE_VOLTAGE,       1, 0x1F, /* 0.6xVcc */
    CMD_SET_SECOND_PRECHARGE_PERIOD, 1, 0x08, /* default */
#endif
    CMD_SET_VCOMH_VOLTAGE,           1, 0x07, /*0.86xVcc;default is 0x04*/
    CMD_SET_DISPLAY_MODE_NORMAL,     0
};

BitmapStream::BitmapStream(const uint8_t pixelDepth, const uint16_t *data, const uint16_t size) :
    bitsPerPixel(pixelDepth), _datap(data), _size(size)
{
    mask = (1 << pixelDepth) - 1;
    _wordsize = 16;
    _bits = 0;
    _word = 0xAA55;
    _count = 0;
}

/**
 * Return the next data word from flash
 * @returns next data word, or 0 if end of data reached
 */
uint16_t BitmapStream::getNextWord()
{
    if (_size > 0) {
	return (uint16_t)pgm_read_word(_datap++);
    } else {
	return 0;
    }
}

/**
 * Return the next pixel from the bitmap
 * @returns next pixel, or 0 if end of data reached
 */

uint8_t BitmapStream::read(void)
{
    uint8_t data=0;
    if (_size > 0) {
	if (_bits == 0) {
	    _word = getNextWord();
	    _bits = _wordsize;
	}
	if (_bits >= bitsPerPixel) {
	    _bits -= bitsPerPixel;
	    data = (_word >> _bits);
	} else {
	    uint8_t offset = bitsPerPixel - _bits;
	    data = _word << offset;
	    _bits += _wordsize - bitsPerPixel;
	    _word = getNextWord();
	    data |= _word >> _bits;
	}
	_size--;
    }
    return data & mask;
}

/**
 * Returns the number of pixels available to read
 * @returns next pixel, or 0 if end of data reached
 */
uint16_t BitmapStream::available(void)
{
    return _size;
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


/**
 * Set pin input or output mode with optional pullup for inputs
 * @param port - the port the pin is on
 * @param pin  - the pin mask for the pin (i.e. 1 << pin_number)
 * @param mode - INPUT or OUTPUT
 * @param pullup - pullup enabled for input if true
 */
void pinMode(uint8_t volatile *port, const uint8_t pin, uint8_t mode, bool pullup)
{
    uint8_t volatile *dataDir = port-1;

    if (mode == INPUT) {
	*dataDir &= ~pin;
	if (pullup) {
	    *port |= pin;
	} else {
	    *port &= ~pin;
	}
    } else if (mode == OUTPUT) {
	*dataDir |= pin;
    }
}

void OledMP::begin(uint8_t font)
{
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

    setFont(font);
    
    pinMode(port_cs, _cs, OUTPUT);
    pinMode(port_dc, _dc, OUTPUT);
    pinMode(port_reset, _reset, OUTPUT);
    pinMode(port_power, _power, OUTPUT);
    pinHigh(port_power, _power);
    pinHigh(port_cs, _cs);

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

    for (uint16_t ind=0; ind < sizeof(oledInit); ) {
	writeCommand(pgm_read_byte(&oledInit[ind++]));
	uint8_t dataSize = pgm_read_byte(&oledInit[ind++]);
	while (dataSize--) {
	    writeData(pgm_read_byte(&oledInit[ind++]));
	}
    }

    pinLow(port_power, _power);	 // 12V power on
    writeCommand(CMD_SET_DISPLAY_ON);
}

/**
 * Write a command or register address to the display
 * @param reg - the command or register to write
 */
void OledMP::writeCommand(uint8_t reg)
{
    pinLow(port_cs, _cs);
    pinLow(port_dc, _dc);
    _spi.transfer(reg);
    pinHigh(port_cs, _cs);
}

/**
 * Write a byte of data to the display
 * @param data - data to write
 */
void OledMP::writeData(uint8_t data)
{
    pinLow(port_cs, _cs);
    pinHigh(port_dc, _dc);
    _spi.transfer(data);
    pinHigh(port_cs, _cs);
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
    pinLow(port_reset, _reset);
    _delay_ms(100);
    pinHigh(port_reset, _reset);
    _delay_ms(10);
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
 * @note to conserve memory the pixel buffer kept in RAM is only 16 bits wide by 64.  It
 *       keeps track of the right-most GDDRAM cell written.
 *       This means the buffer will only work when writing new graphical data to the
 *       right of the last data written (e.g. when drawing a line of text).
 */
uint8_t OledMP::glyphDraw(int16_t x, int16_t y, char ch, uint16_t colour, uint16_t bg)
{
    uint8_t pix;
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
	if (gind == 255) {
	    // no character, use space
	    gind = 0;
	}
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

    // XXX todo: fill unused character space with background
    // XXX todo: add support for n-bit depth fonts (1 to 4)

    for (uint16_t yind=0; yind<glyph_height; yind++) {
	uint16_t pixels;
	// check for shared pixel data to the left
	if ((x/4) == gddram[y+yind].xaddr) {
	    // overlap
	    pixels = gddram[y+yind].pixels >> ((3-xoff)*4);	// 4 pixels
	} else {
	    pixels = 0;
	}

	// write a pixel row
	uint8_t byteCount = byteWidth;
	uint8_t bit;

	for (pix=0; pix<glyph_width; pix+=4) {
	    uint8_t pixel = glyph ? pgm_read_byte(glyph++) : 0;
	    for (bit=0; bit<4; bit++) {
		pixels &= 0xFFF0;
		if (pix+bit < glyph_width) {
		    pixels |= (pixel >> 4) & 0x0c;	// 2bit glyph pixel -> 4 bit oled pixel
		    pixel <<= 2;
		}
		if ((bit & 0x3) == (3-xoff)) {
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


	if ((x+glyph_width) & 0x3) {
	    gddram[y+yind].pixels = pixels;
	} else {
	    // rolled over 
	    gddram[y+yind].pixels = 0;
	}
	gddram[y+yind].xaddr = (x+glyph_width) / 4;
    }

    return (uint8_t)(glyph_width + glyph_offset) + 1;
}


/**
 * Draw a rectangular bitmap on the screen at x,y.
 * @param x - x position for the bitmap
 * @param y - y position for the bitmap (0=top, 63=bottom)
 * @param width - width of bitmap in pixels
 * @param height - height of bitmap in pixels
 * @param depth - number of bits per pixel (1 to 4)
 * @param image - pointer to the image data in flash
 * @note to conserve memory the pixel buffer kept in RAM is only 16 bits wide by 64.  It
 *       keeps track of the right-most GDDRAM cell written.
 *       This means the buffer will only work when writing new graphical data to the
 *       right of the last data written (e.g. when drawing a line of text).
 */
void OledMP::bitmapDraw(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t depth, const uint16_t *image)
{
    uint8_t xoff = x - (x / 4) * 4;
    uint8_t scale = (1<<depth) - 1;

    BitmapStream bms(depth, image, width*height);

    setWindow(x, y, x+width-1, y+height-1);

    writeCommand(CMD_WRITE_RAM);

    for (uint8_t yind=0; yind < height; yind++) {
	uint8_t xind = 0;
	uint16_t pixels = 0;
	uint8_t xcount = 0;
	if (xoff != 0) {
	    // fill the rest of the 4-pixel word from the bitmap
	    for (; xind < 4-xoff; xind++) {
		pixels = pixels << 4 | (bms.read() * 15) / scale;
	    }

	    // Fill existing pixels if available
	    if ((x/4) == gddram[yind].xaddr) {
		pixels |= gddram[yind].pixels;
	    };
	    writeData((uint8_t)(pixels >> 8));
	    writeData((uint8_t)pixels);
	    if (pixels != 0) {
		gddram[yind].pixels = pixels;
		gddram[yind].xaddr = (x/4)+xcount;
	    }
	    xcount++;
	}
	for (; xind < width; xind+=4) {
	    for (uint8_t pind=0; pind<4; pind++) {
		pixels <<= 4;
		if (xind+pind < width) {
		    uint8_t pix = bms.read();
		    pixels |= (pix * 15) / scale;
		}
	    }
	    // Fill existing pixels if available
	    if (((x+xind)/4) == gddram[yind].xaddr) {
		pixels |= gddram[yind].pixels;
	    };
	    writeData((uint8_t)(pixels >> 8));
	    writeData((uint8_t)pixels);
	    if (pixels != 0) {
		gddram[yind].pixels = pixels;
		gddram[yind].xaddr = (x/4)+xcount;
	    }
	    xcount++;
	}
    }
}

/**
 * Draw a rectangular bitmap on the screen at x,y.
 * @param x - x position for the bitmap
 * @param y - y position for the bitmap (0=top, 63=bottom)
 * @param image - pointer to a bitmap_t image data structure
 */
void OledMP::bitmapDraw(uint8_t x, uint8_t y, const void *image)
{
    const bitmap_t *bitmap = (const bitmap_t *)image;

    bitmapDraw(x, y, pgm_read_byte(&bitmap->width), pgm_read_byte(&bitmap->height), pgm_read_byte(&bitmap->depth), bitmap->data);
}


