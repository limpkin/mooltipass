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
/*! \file   oledmini.h
 *  \brief  OLED library header
 *  Copyright [2016] [Mathieu Stephan]
 */
#include "fonts.h"
#include "bitmap.h"
#include "bitstreammini.h"


#ifndef OLEDMINI_H_
#define OLEDMINI_H_

/** SSD1305 COMMANDS **/
#define SSD1305_CMD_SET_LOW_COLUMN_START_ADDR       0x00
#define SSD1305_CMD_SET_HIGH_COLUMN_START_ADDR      0x10    
#define SSD1305_CMD_SET_MEM_ADDRESSING_MODE         0x20
#define SSD1305_CMD_SET_COLUMN_ADDR                 0x21
#define SSD1305_CMD_SET_PAGE_ADDR                   0x22
#define SSD1305_CMD_SET_DISPLAY_START_LINE          0x40
#define SSD1305_CMD_SET_CONTRAST_CURRENT            0x81
#define SSD1305_CMD_SET_BRIGHTNESS                  0x82
#define SSD1305_CMD_SET_LUT                         0x91
#define SSD1305_CMD_SET_BANK_COLOR_1TO16            0x92
#define SSD1305_CMD_SET_BANK_COLOR_17TO32           0x93
#define SSD1305_CMD_SET_SEGMENT_REMAP_COL_0         0xA0
#define SSD1305_CMD_SET_SEGMENT_REMAP_COL_131       0xA1
#define SSD1305_CMD_ENTIRE_DISPLAY_NORMAL           0xA4
#define SSD1305_CMD_ENTIRE_DISPLAY_ON               0xA5
#define SSD1305_CMD_ENTIRE_DISPLAY_NREVERSED        0xA6
#define SSD1305_CMD_ENTIRE_DISPLAY_REVERSED         0xA7
#define SSD1305_CMD_SET_MULTIPLEX_RATIO             0xA8
#define SSD1305_CMD_DIM_MODE_SETTING                0xAB
#define SSD1305_CMD_SET_MASTER_CONFIGURATION        0xAD
#define SSD1305_CMD_DISPLAY_DIM_MODE                0xAC
#define SSD1305_CMD_DISPLAY_OFF                     0xAE
#define SSD1305_CMD_DISPLAY_NORMAL_MODE             0xAF
#define SSD1305_CMD_SET_PAGE_START_ADDR             0xB0
#define SSD1305_CMD_COM_OUTPUT_NORMAL               0xC0
#define SSD1305_CMD_COM_OUTPUT_REVERSED             0xC8
#define SSD1305_CMD_SET_DISPLAY_OFFSET              0xD3
#define SSD1305_CMD_SET_DISPLAY_CLOCK_DIVIDE        0xD5
#define SSD1305_CMD_SET_AREA_COLOR_MODE             0xD8
#define SSD1305_CMD_SET_PRECHARGE_PERIOD            0xD9
#define SSD1305_CMD_SET_COM_PINS_CONF               0xDA
#define SSD1305_CMD_SET_VCOMH_VOLTAGE               0xDB
#define SSD1305_CMD_ENTER_READ_MODIFY_WRITE_MODE    0xE0
#define SSD1305_CMD_NOP                             0xE3
#define SSD1305_CMD_EXIT_READ_MODIFY_WRITE_MODE     0xEE

/** DEFINES OLED SCREEN **/
#define SSD1305_OLED_WIDTH                          128         // Display width
#define SSD1305_OLED_HEIGHT                         32          // Display height
#define SSD1305_OLED_HEIGHT_BITMASK                 0x1F        // Bitmask for the height
#define SSD1305_OLED_BUFFER_PAGE_HEIGHT             5           // Height of our buffer in pages
#define SSD1305_OLED_BUFFER_HEIGHT                  40          // Height of our buffer
#define SSD1305_PAGE_HEIGHT                         8           // One page is 8 pixels high
#define SSD1305_PAGE_HEIGHT_BIT_SHIFT               3           // 1 << 3 is 8
#define SSD1305_WIDTH_BIT_SHIFT                     7           // 1 << 7 is 128
#define SSD1305_X_OFFSET                            4           // X offset between the screen frame buffer and the display
#define SSD1305_Y_BUFFER_HEIGHT                     64          // Screen frame buffer is 64 pixels high
#define SSD1305_Y_BUFFER_HEIGHT_BITMASK             0x3F        // Bitmask for 64
#define SSD1305_SCROLL_SPEED_MS                     5           // Scrolling speed in ms
#define SSD1305_SCREEN_PAGE_HEIGHT                  4           // 4 pages is one screen height
#define SSD1305_SCREEN_PAGE_HEIGHT_BITMASK          0x03        // Bitmask for 4
#define SSD1305_TOTAL_PAGE_HEIGHT                   8           // 8 pages is one screen buffer height
#define SSD1305_TOTAL_PAGE_HEIGHT_BITMASK           0x07        // Bitmask for 8

/** ONE LINE FUNCTIONS **/
#define miniOledNormalDisplay()                     miniOledWriteSimpleCommand(SSD1305_CMD_ENTIRE_DISPLAY_NREVERSED)
#define miniOledInvertedDisplay()                   miniOledWriteSimpleCommand(SSD1305_CMD_ENTIRE_DISPLAY_REVERSED)

/** MACROS **/
#ifdef OLED_DEBUG_OUTPUT_USB
    #define OLEDDEBUGPRINTF_P(args...)              usbPrintf_P(args)
#else
    #define OLEDDEBUGPRINTF_P(args...)
#endif

/************ PROTOTYPES ************/
void miniOledOn(void);
void miniOledOff(void);
void miniOledResetXY(void);
void miniOledInitIOs(void);
RET_TYPE miniOledPutch(char ch);
void miniOledResetMaxTextY(void);
void miniOledBegin(uint8_t font);
void miniOledReverseDisplay(void);
RET_TYPE miniOledIsScreenOn(void);
void miniOledDumpCurrentFont(void);
uint8_t miniOledGlyphWidth(char ch);
void miniOledClearFrameBuffer(void);
void miniOledUnReverseDisplay(void);
void miniOledWriteActiveBuffer(void);
void miniInvertBufferAndFlushIt(void);
void miniOledDisplayOtherBuffer(void);
void miniOledSetMinTextY(uint8_t minY);
void miniOledSetMaxTextY(uint8_t maxY);
void miniOledWriteInactiveBuffer(void);
uint8_t miniOledPutstr(const char* str);
void miniOledSetFont(uint8_t fontIndex);
void miniOledSetXY(uint8_t x, int8_t y);
RET_TYPE miniOledIsDisplayReversed(void);
void miniOledCheckFlashStringsWidth(void);
void miniOledFlushWrittenTextToDisplay(void);
void miniOledWriteSimpleCommand(uint8_t reg);
void miniOledFlushEntireBufferToDisplay(void);
void miniOledAllowTextWritingYIncrement(void);
void miniOledPreventTextWritingYIncrement(void);
void miniOledDontFlushWrittenTextToDisplay(void);
void miniOledSetContrastCurrent(uint8_t current);
uint8_t miniOledPutCenteredString(uint8_t y, char* string);
uint8_t miniOledGlyphDraw(uint8_t x, uint8_t y, char ch);
void miniOledBitmapDrawRaw(int8_t x, uint8_t y, bitstream_mini_t* bs);
void miniOledWriteFrameBuffer(uint16_t offset, uint8_t* data, uint8_t nbBytes);
void displayCenteredCharAtPosition(char c, uint8_t x, uint8_t y, uint8_t font);
uint8_t miniOledPutstrXY(uint8_t x, uint8_t y, uint8_t justify, const char* str);
void miniOledBitmapDrawFlash(int8_t x, int8_t y, uint8_t fileId, uint8_t options);
void miniOledFlushBufferContents(uint8_t xstart, uint8_t xend, uint8_t ystart, uint8_t yend);
void miniOledDrawRectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t full);

#endif /* OLEDMINI_H_ */