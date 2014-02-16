/* CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at src/license_cddl-1.0.txt
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at src/license_cddl-1.0.txt
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */

/* Copyright (c) 2014 Darran Hunt. All rights reserved. */

/*!	\file 	oledmp.c
*	\brief	Mooltipass SSD1322 256x64x16 OLED display library
*	Created: 15/2/2014
*	Author: Darran Hunt
*/

/**
 * @file OledMP Mooltipass 256x64x4 OLED display driver
 */

#include <util/delay.h>
#include <avr/pgmspace.h>

#include "oledmp.h"
#include "bitstream.h"

#define MIN_SEG 28		// minimum visable OLED 4-pixel segment
#define MAX_SEG 91		// maximum visable OLED 4-pixel segment

#define OLED_WIDTH 256
#define OLED_HEIGHT 64

#define pinLow(port, pin)	*port &= ~pin		// set a pin output low
#define pinHigh(port, pin)	*port |= pin		// set a pin output high

static uint8_t volatile *port_cs;
static uint8_t _cs;
static uint8_t volatile *port_dc;
static uint8_t _dc;
static uint8_t volatile *port_reset;
static uint8_t _reset;
static uint8_t volatile *port_power;
static uint8_t _power;
static char _printBuf[64];	// scratch buffer for printf
static uint8_t end_x;
static uint8_t end_y;
static uint8_t _offset;
static uint8_t _bufHeight;

// pixel buffer to allow merging of adjacent image data.
// To conserve memory, only one GDDRAM word is kept per display line.
// The library assumes the display will be filled left to right, and
// hence it only needs to merge the rightmost data with the next write
// on that line.
static struct {
    uint8_t xaddr;
    uint16_t pixels;
} gddram[LCDHEIGHT];

static font_t *_fontHQ;

static uint8_t cur_x;
static uint8_t cur_y;
static uint8_t foreground;
static uint8_t background;
static bool wrap;

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


void oled_begin(
	uint8_t volatile *cs_port,
	const uint8_t cs,
	uint8_t volatile *dc_port,
	const uint8_t dc,
	uint8_t volatile *reset_port,
	const uint8_t reset,
	uint8_t volatile *power_port,
	const uint8_t power,
	uint8_t font)
{
    port_cs = cs_port;
    _cs = 1<<cs;
    port_dc = dc_port;
    _dc = 1<<dc;
    port_reset = reset_port;
    _reset = 1<<reset;
    port_power = power_port;
    _power = 1<<power;

    foreground = 15;
    background = 0;
    wrap = true;
    _offset = 0;
    _bufHeight = OLED_HEIGHT;
    _fontHQ = NULL;

    oled_setFont(font);
    
    pinMode(port_cs, _cs, OUTPUT, false);
    pinMode(port_dc, _dc, OUTPUT, false);
    pinMode(port_reset, _reset, OUTPUT, false);
    pinMode(port_power, _power, OUTPUT, false);
    pinHigh(port_power, _power);
    pinHigh(port_cs, _cs);

    oled_reset();
    oled_init();

    for (uint8_t ind=0; ind<LCDHEIGHT; ind++) {
	gddram[ind].xaddr = 0;
	gddram[ind].pixels = 0;
    }

    oled_clear();
}

void oled_setColour(uint8_t colour)
{
    foreground = colour & 0x0F;
}

void oled_setBackground(uint8_t colour)
{
    background = colour & 0x0F;
}

void oled_setContrast(uint8_t contrast)
{
    oled_writeCommand(CMD_SET_CONTRAST_CURRENT);
    oled_writeData(contrast);
}

void oled_putstr(char *str)
{
    char ch;

    while ((ch=*str++) != 0) {
	if (ch == '\n') {
	    cur_y += oled_glyphHeight();
	    cur_x = 0;
	} else if (ch == '\r') {
	    // skip em
	} else {
	    uint8_t width = oled_glyphWidth(ch);
	    if (wrap && ((cur_x + width) > OLED_WIDTH)) {
		cur_y += oled_glyphHeight();
		cur_x = 0;
	    }
	    cur_x += oled_glyphDraw(cur_x, cur_y, ch, foreground, background);
	}
    }
}


int oled_printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int ret = vsnprintf(_printBuf, sizeof(_printBuf), fmt, ap);
      
    oled_putstr(_printBuf);

    return ret;
} 

int oled_printf_P(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int ret = vsnprintf_P(_printBuf, sizeof(_printBuf), (const char *)fmt, ap);
      
    oled_putstr(_printBuf);

    return ret;
} 

void oled_setXY(uint8_t col, uint8_t row)
{
    cur_x = col;
    cur_y = row;
}


/**
 * Initialise the OLED hardware and get it ready for use.
 */
void oled_init()
{

    for (uint16_t ind=0; ind < sizeof(oledInit); ) {
	oled_writeCommand(pgm_read_byte(&oledInit[ind++]));
	uint8_t dataSize = pgm_read_byte(&oledInit[ind++]);
	while (dataSize--) {
	    oled_writeData(pgm_read_byte(&oledInit[ind++]));
	}
    }

    pinLow(port_power, _power);	 // 12V power on
    oled_writeCommand(CMD_SET_DISPLAY_ON);
}

/**
 * Write a command or register address to the display
 * @param reg - the command or register to write
 */
void oled_writeCommand(uint8_t reg)
{
    pinLow(port_cs, _cs);
    pinLow(port_dc, _dc);
    spi_transfer(reg);
    pinHigh(port_cs, _cs);
}

/**
 * Write a byte of data to the display
 * @param data - data to write
 */
void oled_writeData(uint8_t data)
{
    pinLow(port_cs, _cs);
    pinHigh(port_dc, _dc);
    spi_transfer(data);
    pinHigh(port_cs, _cs);
}

/**
 * Set the current pixel data column start and end address
 * @param start - start column
 * @param end - end column
 */
void oled_setColumnAddr(uint8_t start, uint8_t end)
{
    oled_writeCommand(CMD_SET_COLUMN_ADDR);
    oled_writeData(start);
    oled_writeData(end);
}

/**
 * Set the current pixel data row start and end address
 * @param start - start row
 * @param end - end row
 */
void oled_setRowAddr(uint8_t start, uint8_t end)
{
    oled_writeCommand(CMD_SET_ROW_ADDR);
    oled_writeData(start);
    oled_writeData(end);
}

/**
 * Set the current pixel data window.
 * Data writes will update only this section of the display.
 * @param x - start row
 * @param y - start column
 * @param xend - end row
 * @param yend - end column
 */
void oled_setWindow(uint8_t x, uint8_t y, uint16_t xend, uint8_t yend)
{
    oled_setColumnAddr(MIN_SEG + x / 4, MIN_SEG + xend / 4);
    oled_setRowAddr(y, yend);
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
void oled_setOffset(uint8_t offset)
{
    oled_writeCommand(CMD_SET_DISPLAY_OFFSET);
    oled_writeData(offset);
    _offset = offset;
}

/**
 * Get the current display offset (y origin).
 * @returns current y offset
 */
uint8_t oled_getOffset(void)
{
    return _offset;
}

void oled_setBufHeight(uint8_t rows)
{
    if (rows < LCDHEIGHT) {
	return;
    }
    oled_writeCommand(CMD_SET_MULTIPLEX_RATIO);
    oled_writeData(rows & 0x7F);
    _bufHeight = rows;
}

uint8_t oled_getBufHeight(void)
{
    return _bufHeight;
}


/**
 * Set the font to use
 * @param font - new font to use
 */
void oled_setFont(uint8_t font)
{
    _fontHQ = &fontsHQ[font];
}


/**
 * Fill the display with the specified colour by setting
 * every pixel to the colour.
 * @param colour - fill the display with this colour.
 */
void oled_fill(uint8_t colour)
{
    uint8_t x,y;
    oled_setColumnAddr(MIN_SEG, MAX_SEG);	// SEG0 - SEG479
    oled_setRowAddr(0, 63);	

    colour = (colour & 0x0F) | (colour << 4);;

    oled_writeCommand(CMD_WRITE_RAM);
    for(y=0; y<64; y++)
    {
	for(x=0; x<64; x++) {
	    oled_writeData(colour);
	    oled_writeData(colour);
	}
    }
}

/**
 * Clear the display by setting every pixel to the background colour.
 */
void oled_clear()
{
    oled_fill(background);

    for (uint8_t ind=0; ind<LCDHEIGHT; ind++) {
	gddram[ind].xaddr = 0;
	gddram[ind].pixels = 0;
    }

    cur_x = 0;
    cur_y = 0;
    end_x = 0;
    end_y = 0;
}

/**
 * Reset the OLED display.
 */
void oled_reset()
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
uint8_t oled_glyphWidth(char ch)
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
uint8_t oled_glyphHeight()
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
uint8_t oled_glyphDraw(int16_t x, int16_t y, char ch, uint16_t colour, uint16_t bg)
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

    oled_setWindow(x, y, x+glyph_width-1, y+glyph_height-1);

    oled_writeCommand(CMD_WRITE_RAM);

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
		    oled_writeData((uint8_t)(pixels >> 8));
		    oled_writeData((uint8_t)pixels);
		    byteCount--;
		} else {
		    pixels <<= 4;
		}
	    }
	}

	if (byteCount != 0) {
	    // write final pixels
	    pixels <<= (3-xoff)*4;
	    oled_writeData((uint8_t)(pixels >> 8));
	    oled_writeData((uint8_t)pixels);
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
void oled_bitmapDrawRaw(uint8_t x, uint8_t y, uint16_t width, uint8_t height, uint8_t depth, const uint16_t *image)
{
    uint8_t xoff = x - (x / 4) * 4;
    uint8_t scale = (1<<depth) - 1;

    bitstream_t bs;
    bs_init(&bs, depth, image, width*height);

    oled_setWindow(x, y, x+width-1, y+height-1);

    oled_writeCommand(CMD_WRITE_RAM);

    for (uint8_t yind=0; yind < height; yind++) {
	uint16_t xind = 0;
	uint16_t pixels = 0;
	uint8_t xcount = 0;
	if (xoff != 0) {
	    // fill the rest of the 4-pixel word from the bitmap
	    for (; xind < 4-xoff; xind++) {
		pixels = pixels << 4 | (bs_read(&bs) * 15) / scale;
	    }

	    // Fill existing pixels if available
	    if ((x/4) == gddram[yind].xaddr) {
		pixels |= gddram[yind].pixels;
	    };
	    oled_writeData((uint8_t)(pixels >> 8));
	    oled_writeData((uint8_t)pixels);
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
		    uint8_t pix = bs_read(&bs);
		    pixels |= (pix * 15) / scale;
		}
	    }
	    // Fill existing pixels if available
	    if (((x+xind)/4) == gddram[yind].xaddr) {
		pixels |= gddram[yind].pixels;
	    };
	    oled_writeData((uint8_t)(pixels >> 8));
	    oled_writeData((uint8_t)pixels);
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
void oled_bitmapDraw(uint8_t x, uint8_t y, const void *image)
{
    const bitmap_t *bitmap = (const bitmap_t *)image;

    oled_bitmapDrawRaw(x, y, pgm_read_word(&bitmap->width), pgm_read_byte(&bitmap->height), pgm_read_byte(&bitmap->depth), bitmap->data);
}


