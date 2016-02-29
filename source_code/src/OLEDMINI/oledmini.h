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
#define SSD1305_OLED_CONTRAST                       0xDB
#define SSD1305_OLED_WIDTH                          128
#define SSD1305_OLED_HEIGHT                         32
#define SSD1305_PAGE_HEIGHT                         8
#define SSD1305_PAGE_HEIGHT_BIT_SHIFT               3
#define SSD1305_WIDTH_BIT_SHIFT                     7
#define SSD1305_X_OFFSET                            4

/** ONE LINE FUNCTIONS **/
#define miniOledNormalDisplay()                     oledWriteCommand(SSD1305_CMD_ENTIRE_DISPLAY_NREVERSED)
#define miniOledInvertedDisplay()                   oledWriteCommand(SSD1305_CMD_ENTIRE_DISPLAY_REVERSED)

/************ PROTOTYPES ************/
void miniOledClear(void);
void miniOledInitIOs(void);
void miniOledBegin(uint8_t font);
void miniOledWriteActiveBuffer(void);
void miniOledWriteInactiveBuffer(void);
void miniOledSetFont(uint8_t fontIndex);
void miniOledFlushEntireBufferToDisplay(void);
void miniOledPutstrXY(int16_t x, uint8_t y, uint8_t justify, const char* str);
void miniOledBitmapDrawFlash(uint8_t x, uint8_t y, uint8_t fileId, uint8_t options);
void miniOledBitmapDrawRaw(uint8_t x, uint8_t y, bitstream_mini_t* bs, uint8_t options);
void miniOledFlushBufferContents(uint8_t xstart, uint8_t xend, uint8_t ystart, uint8_t yend);
void miniOledDrawRectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t full);

#endif /* OLEDMINI_H_ */