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

#include <stdio.h>
#include "mooltipass.h"
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "low_level_utils.h"
#include "oledmp.h"
#include "bitstream.h"

// Make sure the USART SPI is selected
#if SPI_OLED != SPI_USART
#error "SPI not implemented"
#endif

#undef OLED_DEBUG
#ifdef OLED_DEBUG
#include "usb_serial.h"
#endif

// OLED specific port and pin definitions
#define OLED_PORT_CS	&PORT_OLED_SS
#define OLED_PORT_DC	&PORT_OLED_DnC
#define OLED_PORT_RESET &PORT_OLED_nR
#define OLED_PORT_POWER &PORT_OLED_POW
#define OLED_CS		(1<<PORTID_OLED_SS)
#define OLED_DC		(1<<PORTID_OLED_DnC)
#define OLED_nRESET	(1<<PORTID_OLED_nR)
#define OLED_POWER	(1<<PORTID_OLED_POW)

#define MIN_SEG 28		// minimum visable OLED 4-pixel segment
#define MAX_SEG 91		// maximum visable OLED 4-pixel segment

/*
 * Module Local globals
 */

static uint8_t oled_offset;
static uint8_t oled_bufHeight;

// pixel buffer to allow merging of adjacent image data.
// To conserve memory, only one GDDRAM word is kept per display line.
// The library assumes the display will be filled left to right, and
// hence it only needs to merge the rightmost data with the next write
// on that line.
static struct {
    uint8_t xaddr;
    uint16_t pixels;
} gddram[OLED_HEIGHT];

static font_t *oled_fontp;      //*< Current font
static uint8_t oled_cur_x;
static uint8_t oled_cur_y;
static uint8_t oled_foreground;
static uint8_t oled_background;

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
    CMD_SET_CONTRAST_CURRENT,        1, 0x7f, /* default is 0x7f*/
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
 * Initialise the OLED controller and prep the display
 * for use.
 */
void oled_begin(uint8_t font)
{
    oled_foreground = 15;
    oled_background = 0;
    oled_offset = 0;
    oled_bufHeight = OLED_HEIGHT;
    oled_fontp = NULL;

    oled_setFont(font);
    
    pinMode(OLED_PORT_CS, OLED_CS, OUTPUT, false);
    pinMode(OLED_PORT_DC, OLED_DC, OUTPUT, false);
    pinMode(OLED_PORT_RESET, OLED_nRESET, OUTPUT, false);
    pinMode(OLED_PORT_POWER, OLED_POWER, OUTPUT, false);
    pinHigh(OLED_PORT_POWER, OLED_POWER);
    pinHigh(OLED_PORT_CS, OLED_CS);

    oled_reset();
    oled_init();

    for (uint8_t ind=0; ind<OLED_HEIGHT; ind++) {
        gddram[ind].xaddr = 0;
        gddram[ind].pixels = 0;
    }

    oled_clear();
}

void oled_setColour(uint8_t colour)
{
    oled_foreground = colour & 0x0F;
}

/**
 * Set the contrast (brightness) level for the screen.
 * @param contrast - level from 0 (min) to 255 (max)
 */
void oled_setBackground(uint8_t colour)
{
    oled_background = colour & 0x0F;
}

/**
 * Set the contrast (brightness) level for the screen.
 * @param contrast - level from 0 (min) to 255 (max)
 */
void oled_setContrast(uint8_t contrast)
{
    oled_writeCommand(CMD_SET_CONTRAST_CURRENT);
    oled_writeData(contrast);
}


/**
 * print an character on the screen at the current X and
 * Y position. X and Y position is updated after the print
 * operation, with X wrapping if necessary.
 * @param ch - the character to print
 * @note '\n' is will increment the row position based
 *       on the current font height, and also reset x to 0.
 *       '\r' will reset x to 0.
 */
void oled_putch(char ch)
{
    if (ch == '\n') {
        oled_cur_y += oled_glyphHeight();
        oled_cur_x = 0;
    } else if (ch == '\r') {
        oled_cur_x = 0;
    } else {
        uint8_t width = oled_glyphWidth(ch);
#ifdef OLED_DEBUG
        usb_printf_P(PSTR("oled_putch('%c')\n"), ch);
#endif
        if ((oled_cur_x + width) > OLED_WIDTH) {
#ifdef OLED_DEBUG
            usb_printf_P(PSTR("wrap at y=%d x=%d, '%c' width %d\n"),oled_cur_y,oled_cur_x,ch,width);
#endif
            oled_cur_y += oled_glyphHeight();
            oled_cur_x = 0;
        }
        oled_cur_x += oled_glyphDraw(oled_cur_x, oled_cur_y, ch, oled_foreground, oled_background);
    }
}


/**
 * print an progmem ASCIIZ string to the screen at the current X
 * and Y position. X and Y position is updated after the print
 * operation, with X wrapping if necessary.
 * @param str - pointer to the string in FLASH.
 */
void oled_putstr_P(const char *str)
{
    char ch;
    ch = pgm_read_byte(str++);
    while (ch != 0) {
        oled_putch(ch);
        ch = pgm_read_byte(str++);
    }
}


/**
 * print an ASCIIZ string to the screen at the current X and
 * Y position. X and Y position is updated after the print
 * operation, with X wrapping if necessary.
 * @param str - pointer to the string in RAM.
 */
void oled_putstr(const char *str)
{
    char ch;

    while ((ch=*str++) != 0) {
        oled_putch(ch);
    }
}


/**
 * print a printf formated string and arguments to the screen starting
 * at the current X and Y position. X and Y position is updated
 * after the print operation, with X wrapping if necessary.
 * @param fmt - pointer to the printf format string in RAM
 * @returns the number of characters printed
 * @note maxium output is limited to 64 characters
 */
int oled_printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char printBuf[64];	// scratch buffer for printf

    int ret = vsnprintf(printBuf, sizeof(printBuf), fmt, ap);
      
    oled_putstr(printBuf);

    va_end(ap);
    return ret;
} 

/**
 * print a printf formated string and arguments to the screen starting
 * at the current X and Y position. X and Y position is updated
 * after the print operation, with X wrapping if necessary.
 * @param fmt - pointer to the printf format string in progmem
 * @returns the number of characters printed
 * @note maxium output is limited to 64 characters per call
 */
int oled_printf_P(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char printBuf[64];	// scratch buffer for printf

    int ret = vsnprintf_P(printBuf, sizeof(printBuf), fmt, ap);
      
    oled_putstr(printBuf);

    va_end(ap);
    return ret;
} 


/**
 * Set the current X and Y position in the display
 * @param col - X (column position)
 * @param row - Y (row position)
 */
void oled_setXY(uint8_t col, uint8_t row)
{
    oled_cur_x = col;
    oled_cur_y = row;
}

/**
 * Set the current X position in the display
 * @param col - X (column position)
 */
void oled_setX(uint8_t col)
{
    oled_cur_x = col;
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

    pinLow(OLED_PORT_POWER, OLED_POWER);	 // 12V power on
    oled_writeCommand(CMD_SET_DISPLAY_ON);
}

/**
 * Write a command or register address to the display
 * @param reg - the command or register to write
 */
void oled_writeCommand(uint8_t reg)
{
    pinLow(OLED_PORT_CS, OLED_CS);
    pinLow(OLED_PORT_DC, OLED_DC);
    spi_transfer(reg);
    pinHigh(OLED_PORT_CS, OLED_CS);
}

/**
 * Write a byte of data to the display
 * @param data - data to write
 */
void oled_writeData(uint8_t data)
{
    pinLow(OLED_PORT_CS, OLED_CS);
    pinHigh(OLED_PORT_DC, OLED_DC);
    spi_transfer(data);
    pinHigh(OLED_PORT_CS, OLED_CS);
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
    oled_offset = offset;
}

/**
 * Get the current display offset (y origin).
 * @returns current y offset
 */
uint8_t oled_getOffset(void)
{
    return oled_offset;
}

void oled_setBufHeight(uint8_t rows)
{
    if (rows < OLED_HEIGHT) {
	return;
    }
    oled_writeCommand(CMD_SET_MULTIPLEX_RATIO);
    oled_writeData(rows & 0x7F);
    oled_bufHeight = rows;
}

uint8_t oled_getBufHeight(void)
{
    return oled_bufHeight;
}


/**
 * Set the font to use
 * @param font - new font to use
 */
void oled_setFont(uint8_t font)
{
    oled_fontp = &fontsHQ[font];
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
    for(y=0; y<64; y++) {
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
    oled_fill(oled_background);

    for (uint8_t ind=0; ind<OLED_HEIGHT; ind++) {
        gddram[ind].xaddr = 0;
        gddram[ind].pixels = 0;
    }

    oled_cur_x = 0;
    oled_cur_y = 0;
}

/**
 * Reset the OLED display.
 */
void oled_reset()
{
    pinLow(OLED_PORT_RESET, OLED_nRESET);
    _delay_ms(100);
    pinHigh(OLED_PORT_RESET, OLED_nRESET);
    _delay_ms(10);
}


/**
 * Return the width of the specified character in the current font.
 * @param ch - return the width of this character
 * @returns width of the glyph
 */
uint8_t oled_glyphWidth(char ch)
{
    if (oled_fontp) {
        uint8_t width = oled_fontp->fixedWidth;
        if (width) {
            return width;
        } else {
            uint8_t gind;
            if (ch >= ' ' && ch <= 0x7f) {
                gind = pgm_read_byte(&oled_fontp->map[ch - ' ']);
            } else {
                // default to a space
                gind = 0;
            }
            return (uint8_t)pgm_read_byte(&(oled_fontp->fontData.glyphs[gind].width));
        }
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
    if (oled_fontp) {
        return oled_fontp->height + (oled_fontp->fixedWidth ?  1 : 0);
    } else {
        return 0;
    }
}

static uint16_t glyph_getPixel(uint16_t x, uint16_t y, uint8_t width, uint8_t depth, const uint8_t *data)
{
    if (x >= width) {
        return 0;
    }

    static const uint8_t *cacheData = NULL;
    static uint8_t cacheByte=0;
    static uint8_t cacheInd=255;

    uint8_t ind = y*((width*depth+7)/8) + x*depth/8;
    uint8_t bit = 7 - (x*depth) % 8;
    uint8_t mask = (1<<depth) - 1;

    if ((cacheData != data) || (cacheInd != ind)) {
        cacheByte = data ? pgm_read_byte(&data[ind]) : 0;
        cacheInd = ind;
        cacheData = data;
    }

#ifdef OLED_DEBUG
    uint16_t pixel = ((cacheByte >> (bit - (depth-1)) & mask) * ((oled_foreground<<1) / depth)) >> 1;
    usb_printf_P(PSTR("    getPixel(x=%d,y=%d) bit=%d ind=%d data=0x%02x -> 0x%04x\n"), x, y, bit, ind, cacheByte, pixel);
    return pixel;
#else
    //return (cacheByte >> (bit - (depth-1)) & mask) << (4-depth);
    return ((cacheByte >> (bit - (depth-1)) & mask) * ((oled_foreground<<1) / depth)) >> 1;
#endif
}


/**
 * Draw a character glyph on the screen at x,y.
 * @param x - x position to start glyph (x=0 for left, x=256-glyphWidth() for right)
 * @param y - y position to start glyph (y=0 for top, y=64-glyphHeight() for bottom)
 * @param ch - the character to draw
 * @param colour - colour [deprecated]
 * @param bg - background colour [deprecated]
 * @returns width of the glyph
 * @note to conserve memory the pixel buffer kept in RAM is only 16 bits wide by 64.  It
 *       keeps track of the right-most GDDRAM cell written.
 *       This means the buffer will only work when writing new graphical data to the
 *       right of the last data written (e.g. when drawing a line of text).
 */
uint8_t oled_glyphDraw(int16_t x, int16_t y, char ch, uint16_t colour, uint16_t bg)
{
    uint8_t xoff = x - (x / 4) * 4;
    const uint8_t *glyph;
    uint8_t glyph_width;
    uint8_t glyph_height;
    uint8_t glyph_depth;
    int8_t glyph_offset;
    uint8_t gind;

#ifdef OLED_DEBUG
    usb_printf_P(PSTR("oled_glyphDraw(x=%d,y=%d,ch='%c')\n"), x, y, ch);
#endif

    if (oled_fontp == NULL) {
        return 0;
    }

    if (colour == bg) {
        bg = 0;
    }


    // get glyph index
    if (ch >= ' ' && ch <= 0x7f) {  // XXX replace with size of asciimap
        gind = pgm_read_byte(&oled_fontp->map[ch - ' ']);
        if (gind == 255) {
            // no character, use space
            gind = 0;
        }
    } else {
        // default to a space
        gind = 0;
    }

    glyph_depth = oled_fontp->depth;
    glyph_width = oled_fontp->fixedWidth;
    if (glyph_width) {
        // Fixed width font
        glyph_height = oled_fontp->height;
        glyph_offset = 0;
        glyph = oled_fontp->fontData.bitmaps + gind*((glyph_width+7)/8) * glyph_height;
#ifdef OLED_DEBUG
        usb_printf_P(PSTR("    glyph 0x%04x index %d\n"), 
                (uint16_t)oled_fontp->fontData.bitmaps, gind*((glyph_width+7)/8) * glyph_height);
#endif
    } else {
        // proportional width font
        glyph = (const uint8_t *)pgm_read_word(&oled_fontp->fontData.glyphs[gind].glyph);
        if (glyph == NULL) {
            // space character, just fill in the gddram buffer and output background pixels
            glyph_width = (uint8_t)pgm_read_byte(&(oled_fontp->fontData.glyphs[gind].width));
            glyph_height = oled_fontp->height;
            glyph_offset = 0;
        } else {
            glyph_width = (uint8_t)pgm_read_byte(&(oled_fontp->fontData.glyphs[gind].xrect));
            glyph_height = (uint8_t)pgm_read_byte(&(oled_fontp->fontData.glyphs[gind].yrect));
            glyph_offset = (int8_t)pgm_read_byte(&(oled_fontp->fontData.glyphs[gind].xoffset));
            x += glyph_offset;
            y += (int8_t) pgm_read_byte(&(oled_fontp->fontData.glyphs[gind].yoffset));
            if (x < 0) {
                x = 0;
            }
            if (y < 0) {
                y = 0;
            }
        }
    }
#ifdef OLED_DEBUG
    usb_printf_P(PSTR("    glyph width %d height %d depth %d\n"), glyph_width, glyph_height, glyph_depth);
#endif

    if ((y+glyph_height) > OLED_HEIGHT) {
        glyph_height = OLED_HEIGHT-y+1;
    }

    oled_setWindow(x, y, x+glyph_width-1, y+glyph_height-1);

    oled_writeCommand(CMD_WRITE_RAM);

    // XXX todo: fill unused character space with background
    // XXX todo: add support for n-bit depth fonts (1 to 4)
    //
    for (uint8_t yind=0; yind < glyph_height; yind++) {
        uint16_t xind = 0;
        uint16_t pixels = 0;
        uint8_t xcount = 0;
        if (xoff != 0) {
            // fill the rest of the 4-pixel word from the bitmap
            for (; xind < 4-xoff; xind++) {
                pixels = pixels << 4 | glyph_getPixel(xind, yind, glyph_width, glyph_depth, glyph);
            }

            // Fill existing pixels if available
            if ((x/4) == gddram[yind+y].xaddr) {
#ifdef OLED_DEBUG
                usb_printf_P(PSTR("    pixel 0x%04x | gddram[%d][%d] 0x%04x\n"), 
                        pixels, yind+y, gddram[yind+y].xaddr, gddram[yind+y].pixels);
#endif
                pixels |= gddram[yind+y].pixels;
            }
            oled_writeData((uint8_t)(pixels >> 8));
            oled_writeData((uint8_t)pixels);
#ifdef OLED_DEBUG
            usb_printf_P(PSTR("    pixels = 0x%04x, xoff=%d\n"), pixels, xoff);
#endif
            if (pixels != 0) {
                gddram[yind+y].pixels = pixels;
                gddram[yind+y].xaddr = (x/4)+xcount;
            }
            xcount++;
        }
        for (; xind < glyph_width; xind+=4) {
            for (uint8_t pind=0; pind<4; pind++) {
                pixels <<= 4;
                if (xind+pind < glyph_width) {
                    pixels |= glyph_getPixel(xind+pind, yind, glyph_width, glyph_depth, glyph);
                }
            }
            oled_writeData((uint8_t)(pixels >> 8));
            oled_writeData((uint8_t)pixels);
#ifdef OLED_DEBUG
            usb_printf_P(PSTR("    pixels = 0x%04x, yind=%d, xind=%d\n"), pixels, yind, xind);
#endif
            if (pixels != 0) {
                gddram[yind+y].pixels = pixels;
                gddram[yind+y].xaddr = (x/4)+xcount;
            }
            xcount++;
        }
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
                pixels = pixels << 4 | (bs_read(&bs,1) * 15) / scale;
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
                    uint8_t pix = bs_read(&bs,1);
                    pixels |= (pix * 15) / scale;
                }
            }
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


