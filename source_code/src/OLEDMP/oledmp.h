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

/* Copyright (c) 2014, Darran Hunt. All rights reserved. */

/*! \file   oledmp.h
*   \brief  OLED library header
*   Created: 15/2/2014
*   Author: Darran Hunt
*/

#ifndef OLEDMP_H_
#define OLEDMP_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <avr/pgmspace.h>
#include <spi.h>

#include "fonts.h"
#include "bitmap.h"
#include "bitstream.h"

/**************************************************
*    SSD1322 driver for 256x16x4 OLED display
*
*    Pin   Function     SPI connections
*    ----+------------+----------
*      1   VSS      GND
*      2   VBAT     3.3V-5V
*      3   NC
*      4   D0       SCLK    Serial Clock
*      5   D1       MOSI    Serial Data Input
*      6   D2       NC
*      7   D3       GND
*      8   D4       GND
*      9   D5       GND
*     10   D6       GND
*     11   D7       GND
*     12   #RD      GND
*     13   #WR      GND
*     14   DC       Data / Command 
*     15   #RESET   
*     16   #CS      Chip select
*
*     Note: All logic pins are 3.3V max.
*
*     BS1 BS0   Mode
*     --------+---------------------
*      0   0    "4 Line SPI" 8-bit + DC pin
*      0   1    "3 Line SPI" 9-bit DC is 9th bit
*      1   0    8-bit 8080 parallel
*      1   1    8-bit 6800 parallel
*
*      Note: SPI mode is write only (MOSI)
*
**************************************************/

/*
 * SSD1322 commands
 */
#define CMD_ENABLE_GRAY_SCALE_TABLE             0x00
#define CMD_SET_COLUMN_ADDR                     0x15
#define CMD_WRITE_RAM                           0x5C
#define CMD_READ_RAM                            0x5D
#define CMD_SET_ROW_ADDR                        0x75
#define CMD_SET_REMAP                           0xA0
#define CMD_SET_DISPLAY_START_LINE              0xA1
#define CMD_SET_DISPLAY_OFFSET                  0xA2
#define CMD_SET_DISPLAY_MODE_OFF                0xA4
#define CMD_SET_DISPLAY_MODE_ON                 0xA5
#define CMD_SET_DISPLAY_MODE_NORMAL             0xA6
#define CMD_SET_DISPLAY_MODE_INVERSE            0xA7
#define CMD_ENABLE_PARTIAL_DISPLAY              0xA8
#define CMD_EXIT_PARTIAL_DISPLAY                0xA9
#define CMD_SET_FUNCTION_SELECTION              0xAB
#define CMD_SET_DISPLAY_OFF                     0xAE
#define CMD_SET_DISPLAY_ON                      0xAF
#define CMD_SET_PHASE_LENGTH                    0xB1
#define CMD_SET_CLOCK_DIVIDER                   0xB3
#define CMD_DISPLAY_ENHANCEMENT                 0xB4
#define CMD_SET_GPIO                            0xB5
#define CMD_SET_SECOND_PRECHARGE_PERIOD         0xB6
#define CMD_SET_GRAY_SCALE_TABLE                0xB8
#define CMD_SET_PRECHARGE_VOLTAGE               0xBB
#define CMD_SET_DEFAULT_LINEAR_GRAY_SCALE_TABLE 0xB9
#define CMD_SET_VCOMH_VOLTAGE                   0xBE
#define CMD_SET_CONTRAST_CURRENT                0xC1
#define CMD_MASTER_CURRENT_CONTROL              0xC7
#define CMD_SET_MULTIPLEX_RATIO                 0xCA
#define CMD_DISPLAY_ENHANCEMENT_B               0xD1
#define CMD_SET_COMMAND_LOCK                    0xFD

/* CMD_SET_REMAP mode bit options */
#define OLED_REMAP_ADDR_INC_COL   0x00 // increment column after each write
#define OLED_REMAP_ADDR_INC_ROW   0x01 // increment row after each write
#define OLED_REMAP_COL_ADDR       0x02 // reamap column address to inverse
#define OLED_REMAP_NIBBLES        0x04 // Use big-endian for nibble map
#define OLED_REMAP_SCAN_TOP_DOWN  0x00 // Scan from top down
#define OLED_REMAP_SCAN_BOTTOM_UP 0x10 // Scan from bottom up

/** DEFINES OLED SCREEN **/
#define	OLED_Shift			0x1C
#define OLED_Max_Column		0x3F			// 256/4-1
#define OLED_Max_Row		0x3F			// 64-1
#define	OLED_Brightness		0x0A
#define OLED_Contrast		0x9F
#define OLED_WIDTH			256
#define OLED_HEIGHT			64

/* Media file types */
#define MEDIA_BITMAP		1
#define MEDIA_FONT		2


// Text line justification options
enum {
    OLED_LEFT  = 0,
    OLED_RIGHT = 1,
    OLED_CENTRE = 2
} justify_e;

void oledBegin(uint8_t font);
void oledInit(void);
void oledWriteCommand(uint8_t reg);
void oledWriteData(uint8_t data);
void oledSetColumnAddr(uint8_t start, uint8_t end);
void oledSetRowAddr(uint8_t start, uint8_t end);
void oledFill(uint8_t colour);
void oledFillXY(uint8_t x, int16_t y, uint16_t width, uint8_t height, uint8_t colour);
void oledClear();
void oledClearLine(int16_t y);
void oledScrollClear(uint8_t options);
void oledScrollUp(uint8_t lines, bool clear);
void oledReset();
void oledOff(void);
void oledOn(void);

#define OLED_SCROLL_UP		1
#define OLED_SCROLL_DOWN	2
#define OLED_RAM_BITMAP		4

int16_t oledGetFileAddr(uint8_t fileId, uint16_t *addr);
void oledBitmapDrawRaw(uint8_t x, uint8_t y, bitstream_t *bs, uint8_t options);
int8_t oledBitmapDrawFlash(uint8_t x, uint8_t y, uint8_t fileId, uint8_t options);
#ifdef OLED_FEATURE_PGM_MEMORY
void oledBitmapDraw(uint8_t x, uint8_t y, const void *image, uint8_t options);
#endif

void oledInvertDisplay(void);
void oledSetDisplayStartLine(uint8_t line);
void oledMoveDisplayStartLine(int8_t offset);
void oledFlipBuffers(uint8_t mode, uint8_t delay);
void oledFlipDisplayedBuffer(void);
void oledFlipWriteBuffer(void);
void oledWriteActiveBuffer(void);
void oledWriteInactiveBuffer(void);

void oledSetScrollSpeed(double delay);
void oledSetWindow(uint8_t x, uint8_t y, uint16_t xend, uint8_t yend);
int8_t oledSetFont(uint8_t fontIndex);
void oledSetColour(uint8_t colour);
void oledSetContrast(uint8_t contrast);
void oledSetRemap(uint8_t mode);
void oledSetBackground(uint8_t colour);
void oledSetOffset(uint8_t offset);
uint8_t oledGetOffset(void);
void oledSetBufHeight(uint8_t rows);
uint8_t oledGetBufHeight(void);

void oledSetXY(uint8_t col, uint8_t row);
void oledSetX(uint8_t col);

void oledSetPixel(uint8_t x, uint8_t y, uint8_t colour);

uint8_t oledGlyphWidth(char ch, uint8_t *indp, glyph_t *glyphp);
uint8_t oledGlyphHeight();
uint8_t oledGlyphDraw(int16_t x, int16_t y, char ch, uint16_t colour, uint16_t bg);

void oledPutch(char ch);
uint16_t oledStrWidth(const char *str);
uint16_t oledStrWidth_P(const char *str);
uint16_t oledGetTextWidth(char *format, ...);
uint16_t oledGetTextWidth_P(char *format, ...);
void oledPutstrXY_P(int16_t x, uint8_t y, uint8_t justify, const char *str);
void oledPutstrXY(int16_t x, uint8_t y, uint8_t justify, const char *str);
void oledPutstr_P(const char *str);
void oledPutstr(const char *str);

#endif
