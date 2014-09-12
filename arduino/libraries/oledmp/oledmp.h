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

#ifndef OLEDMP_H_
#define OLEDMP_H_

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <Arduino.h>
#include <avr/pgmspace.h>
#include <usart_spi.h>

#include "fstr.h"
#include "fonts.h"
#include "bitmap.h"

/**************************************************
*    LM320Y-256064 (SSD1322 driver)
*
*    Pin   Function     SPI connections
*    ----+------------+----------
*      1   VSS		GND
*      2   VBAT		3.3V-5V
*      3   NC
*      4   D0		SCLK	Serial Clock
*      5   D1		MOSI	Serial Data Input
*      6   D2		NC
*      7   D3 		GND
*      8   D4		GND
*      9   D5		GND
*     10   D6		GND
*     11   D7		GND
*     12   #RD		GND
*     13   #WR		GND
*     14   DC		Data / Command 
*     15   #RESET	
*     16   #CS		Chip select
*
*     Note: All logic pins are 3.3V max.
*
*     BS1 BS0	Mode
*     --------+---------------------
*      0   0	"4 Line SPI" 8-bit + DC pin
*      0   1	"3 Line SPI" 9-bit DC is 9th bit
*      1   0	8-bit 8080 parallel
*      1   1	8-bit 6800 parallel
*
*      Note: SPI mode is write only (MOSI)
*
**************************************************/

#define LCDWIDTH                  256
#define LCDHEIGHT                 64

#define CMD_ENABLE_GRAY_SCALE_TABLE	0x00
#define CMD_SET_COLUMN_ADDR		0x15
#define CMD_WRITE_RAM			0x5C
#define CMD_READ_RAM			0x5D
#define CMD_SET_ROW_ADDR		0x75
#define CMD_SET_REMAP			0xA0
#define CMD_SET_DISPLAY_START_LINE	0xA1
#define CMD_SET_DISPLAY_OFFSET		0xA2
#define CMD_SET_DISPLAY_MODE_OFF	0xA4
#define CMD_SET_DISPLAY_MODE_ON		0xA5
#define CMD_SET_DISPLAY_MODE_NORMAL	0xA6
#define CMD_SET_DISPLAY_MODE_INVERSE	0xA7
#define CMD_ENABLE_PARTIAL_DISPLAY	0xA8
#define CMD_EXIT_PARTIAL_DISPLAY	0xA9
#define CMD_SET_FUNCTION_SELECTION	0xAB
#define CMD_SET_DISPLAY_OFF		0xAE
#define CMD_SET_DISPLAY_ON		0xAF
#define CMD_SET_PHASE_LENGTH		0xB1
#define CMD_SET_CLOCK_DIVIDER		0xB3
#define CMD_DISPLAY_ENHANCEMENT		0xB4
#define CMD_SET_GPIO			0xB5
#define CMD_SET_SECOND_PRECHARGE_PERIOD	0xB6
#define CMD_SET_GRAY_SCALE_TABLE	0xB8
#define CMD_SET_PRECHARGE_VOLTAGE	0xBB
#define CMD_SET_DEFAULT_LINEAR_GRAY_SCALE_TABLE	0xB9
#define CMD_SET_VCOMH_VOLTAGE		0xBE
#define CMD_SET_CONTRAST_CURRENT	0xC1
#define CMD_MASTER_CURRENT_CONTROL	0xC7
#define CMD_SET_MULTIPLEX_RATIO		0xCA
#define CMD_DISPLAY_ENHANCEMENT_B	0xD1
#define CMD_SET_COMMAND_LOCK		0xFD

void pinMode(uint8_t volatile *port, const uint8_t pin, uint8_t mode, bool pullup = false);

#ifndef INPUT
#define INPUT 0
#define OUTPUT 1
#endif

class BitmapStream {
public:
    BitmapStream(const uint8_t bitsPerPixel, const uint16_t *data, const uint16_t size);
    uint8_t read(void);
    uint16_t available(void);
    uint8_t mask;		// pixel mask for returned data
private:
    uint8_t bitsPerPixel;	// number of bits per pixel
    const uint16_t *_datap;	// Next data word to read
    uint16_t _size;		// number of pixels
    uint16_t getNextWord(void);
    uint8_t _bits;		// current offset in data word
    uint8_t _wordsize;		// number of bits per data word
    uint16_t _word;
    uint16_t _count;
};

class OledMP {
public:
    OledMP(USARTSPI &spi) : _spi(spi) {};
    void begin(uint8_t font=FONT_DEFAULT);
    void init(void);
    void writeCommand(uint8_t reg);
    void writeData(uint8_t data);
    void setColumnAddr(uint8_t start, uint8_t end);
    void setRowAddr(uint8_t start, uint8_t end);
    void fill(uint8_t colour);
    void clear();
    void reset();

    void bitmapDraw(uint8_t x, uint8_t y, uint16_t width, uint8_t height, uint8_t depth, const uint16_t *image);
    void bitmapDraw(uint8_t x, uint8_t y, const void *image);

    void setWindow(uint8_t x, uint8_t y, uint16_t xend, uint16_t yend);
    void setFont(uint8_t font);
    void setColour(uint8_t colour);
    void setContrast(uint8_t contrast);
    void setBackground(uint8_t colour);
    void setOffset(uint8_t offset);
    uint8_t getOffset(void);
    void setBufHeight(uint8_t rows);
    uint8_t getBufHeight(void);

    void setXY(uint8_t col, uint8_t row);

    uint8_t glyphWidth(char ch);
    uint8_t glyphHeight();
    uint8_t glyphDraw(int16_t x, int16_t y, char ch, uint16_t colour, uint16_t bg);

    void putstr(char *str);
    int printf(const char* fmt, ...);
    int printf(const __fstr *fmt, ...);

    uint8_t cur_x;
    uint8_t cur_y;
    uint8_t foreground;
    uint8_t background;
    bool wrap;

 private:
    uint8_t volatile *port_cs;
    uint8_t _cs;
    uint8_t volatile *port_dc;
    uint8_t _dc;
    uint8_t volatile *port_reset;
    uint8_t _reset;
    uint8_t volatile *port_power;
    uint8_t _power;
    char _printBuf[64];	// scratch buffer for printf
    uint8_t end_x;
    uint8_t end_y;
    uint8_t cur_col;
    uint8_t cur_row;
    uint8_t _offset;
    uint8_t _bufHeight;

    struct {
	uint8_t xaddr;
	uint16_t pixels;
    } gddram[LCDHEIGHT];

    uint8_t readByte();
    void writeByte(uint8_t data);

    font_t *_fontHQ;
    USARTSPI &_spi;
};

#endif
