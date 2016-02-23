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
/*! \file   oledmini.c
 *  \brief  Mooltipass SSD1305 128x32x1 OLED display library
 *  Created: 15/2/2014
 *  Copyright [2016] [Mathieu Stephan]
 */
 #include <avr/pgmspace.h>
 #include "oledmini.h"

/*
 * OLED initialisation sequence
 */
static const uint8_t oled_init[] __attribute__((__progmem__)) = 
{
    CMD_DISPLAY_OFF,                0,                      // Display Off
    CMD_SET_DISPLAY_CLOCK_DIVIDE,   1,  0x10,               // Display divide ratio of 0, Oscillator frequency of around 300kHz
    CMD_SET_MULTIPLEX_RATIO,        1,  0x1F,               // Multiplex ratio of 32
    CMD_SET_DISPLAY_OFFSET,         1,  0x00,               // Display offset 0
    CMD_SET_DISPLAY_START_LINE,     0,                      // Display start line 0
    CMD_SET_MEM_ADDRESSING_MODE,    1,  0x00,               // Horizontal addressing mode
    CMD_SET_MASTER_CONFIGURATION,   1,  0x8E,               // Select external Vcc supply
    CMD_SET_AREA_COLOR_MODE,        1,  0x05,               // Set low power display mode
    CMD_SET_SEGMENT_REMAP_COL_131,  0,                      // Column address 131 is mapped to SEG0 
    CMD_COM_OUTPUT_REVERSED,        0,                      // Remapped mode. Scan from COM[N~1] to COM0
    CMD_SET_COM_PINS_CONF,          1,  0x12,               // Alternative COM pin configuration
    CMD_SET_LUT,                    4,  0x3F,0x3F,0x3F,0x3F,// Set Look up Table
    CMD_SET_CONTRAST_CURRENT,       1,  OLED_Contrast,      // Set current control (contrast)
    CMD_SET_PRECHARGE_PERIOD,       1,  0xD2,               // Precharge period
    CMD_SET_VCOMH_VOLTAGE,          1,  0x08,               // VCOM deselect level (around 0.5Vcc)
    CMD_ENTIRE_DISPLAY_NORMAL,      0,                      // Entire display in normal mode
    CMD_ENTIRE_DISPLAY_NREVERSED,   0,                      // Entire display not reversed
};