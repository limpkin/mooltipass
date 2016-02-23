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
#include "timer_manager.h"
#include <avr/pgmspace.h>
#include "oledmini.h"
#include "defines.h"
#include "spi.h"
// Current y offset in buffer
uint8_t miniOledBufferYOffset;
// Boolean to know if OLED on
uint8_t miniOledIsOn = FALSE;

// OLED initialization sequence
static const uint8_t mini_oled_init[] __attribute__((__progmem__)) = 
{
    SSD1305_CMD_DISPLAY_OFF,                0,                          // Display Off
    SSD1305_CMD_SET_DISPLAY_CLOCK_DIVIDE,   1,  0x10,                   // Display divide ratio of 0, Oscillator frequency of around 300kHz
    SSD1305_CMD_SET_MULTIPLEX_RATIO,        1,  0x1F,                   // Multiplex ratio of 32
    SSD1305_CMD_SET_DISPLAY_OFFSET,         1,  0x00,                   // Display offset 0
    SSD1305_CMD_SET_DISPLAY_START_LINE,     0,                          // Display start line 0
    SSD1305_CMD_SET_MEM_ADDRESSING_MODE,    1,  0x00,                   // Horizontal addressing mode
    SSD1305_CMD_SET_MASTER_CONFIGURATION,   1,  0x8E,                   // Select external Vcc supply
    SSD1305_CMD_SET_AREA_COLOR_MODE,        1,  0x05,                   // Set low power display mode
    SSD1305_CMD_SET_SEGMENT_REMAP_COL_131,  0,                          // Column address 131 is mapped to SEG0 
    SSD1305_CMD_COM_OUTPUT_REVERSED,        0,                          // Remapped mode. Scan from COM[N~1] to COM0
    SSD1305_CMD_SET_COM_PINS_CONF,          1,  0x12,                   // Alternative COM pin configuration
    SSD1305_CMD_SET_LUT,                    4,  0x3F,0x3F,0x3F,0x3F,    // Set Look up Table
    SSD1305_CMD_SET_CONTRAST_CURRENT,       1,  SSD1305_OLED_CONTRAST,  // Set current control (contrast)
    SSD1305_CMD_SET_PRECHARGE_PERIOD,       1,  0xD2,                   // Precharge period
    SSD1305_CMD_SET_VCOMH_VOLTAGE,          1,  0x08,                   // VCOM deselect level (around 0.5Vcc)
    SSD1305_CMD_ENTIRE_DISPLAY_NORMAL,      0,                          // Entire display in normal mode
    SSD1305_CMD_ENTIRE_DISPLAY_NREVERSED,   0,                          // Entire display not reversed
};


/*! \fn     miniOledWriteCommand(uint8_t reg)
 *  \brief  Write a command or register address to the display
 *  \param  reg     the command or register to write
 */
void miniOledWriteCommand(uint8_t reg)
{
    PORT_OLED_SS &= ~(1 << PORTID_OLED_SS);
    PORT_OLED_DnC &= ~(1 << PORTID_OLED_DnC);
    spiUsartTransfer(reg);
    PORT_OLED_SS |= (1 << PORTID_OLED_SS);
}

/*! \fn     miniOledWriteData(uint8_t data)
 *  \brief  Write a byte of data to the display
 *  \param  data    data to write
 */
void miniOledWriteData(uint8_t data)
{
    PORT_OLED_SS &= ~(1 << PORTID_OLED_SS);
    PORT_OLED_DnC |= (1 << PORTID_OLED_DnC); 
    spiUsartTransfer(data);
    PORT_OLED_SS |= (1 << PORTID_OLED_SS);
}

/*! \fn     miniOledOn(void)
 *  \brief  Turn the display on
 */
void miniOledOn(void)
{
    PORT_OLED_POW &= ~(1 << PORTID_OLED_POW);
    timerBased130MsDelay();
    miniOledWriteCommand(SSD1305_CMD_DISPLAY_NORMAL_MODE);
    miniOledIsOn = TRUE;
}

/*! \fn     miniOledInit(void)
 *  \brief  Initialize the OLED hardware and get it ready for use.
 */
void miniOledInit(void)
{
    for (uint8_t ind=0; ind < sizeof(mini_oled_init); ) 
    {
        miniOledWriteCommand(pgm_read_byte(&mini_oled_init[ind++]));
        uint8_t dataSize = pgm_read_byte(&mini_oled_init[ind++]);
        while (dataSize--) 
        {
            miniOledWriteData(pgm_read_byte(&mini_oled_init[ind++]));
        }
    }

    // Clear buffers
    miniOledWriteInactiveBuffer();
    miniOledClear();
    miniOledWriteActiveBuffer();
    miniOledClear();

    // Switch on screen
    miniOledOn();
}

/*! \fn     miniOledReset(void)
 *  \brief  Reset the OLED display
 */
void miniOledReset(void)
{
    PORT_OLED_nR &= ~(1 << PORTID_OLED_nR);
    timerBased130MsDelay();
    PORT_OLED_nR |= (1 << PORTID_OLED_nR);
    timerBasedDelayMs(10);
}

/*! \fn     miniOledInitIOs(void)
 *  \brief  Initialize OLED controller ports
 */
void miniOledInitIOs(void)
{
    /* Setup OLED slave select as output, high */
    DDR_OLED_SS |= (1 << PORTID_OLED_SS);
    PORT_OLED_SS |= (1 << PORTID_OLED_SS);
    
    /*  Setup OLED Data/nCommand as output */
    DDR_OLED_DnC |= (1 << PORTID_OLED_DnC);
    
    /* Setup OLED Reset */
    DDR_OLED_nR |= (1 << PORTID_OLED_nR);
    miniOledReset();
    
    /* Setup Oled power, high */
    DDR_OLED_POW |= (1 << PORTID_OLED_POW);
    PORT_OLED_POW |= (1 << PORTID_OLED_POW);
}

/*! \fn     stockOledBegin(uint8_t font)
 *  \brief  Initialize the OLED controller and prep the display
 *  \param  font    UID of the font to use
 */
void miniOledBegin(uint8_t font)
{
    miniOledBufferYOffset = 0;
    miniOledSetFont(font);
    miniOledInit();

#ifdef ENABLE_PRINTF
    // Map stdout to use the OLED display.
    // This means that printf(), printf_P(), puts(), puts_P(), etc 
    // will all output to the OLED display.
    stdout = &oledStdout;
#endif
}

/*! \fn     miniOledWriteInactiveBuffer(void)
 *  \brief  Set write buffer to be the inactive (offscreen) buffer
 */
void miniOledWriteInactiveBuffer(void)
{
    //oled_writeBuffer = !oled_displayBuffer;
    //oled_writeOffset = OLED_HEIGHT;
}

/*! \fn     miniOledWriteActiveBuffer(void)
 *  \brief  Set write buffer to the be the active (onscreen) buffer
 */
void miniOledWriteActiveBuffer(void)
{
    //oled_writeBuffer = oled_displayBuffer;
    //oled_writeOffset = 0;
}

/*! \fn     miniOledClear(void)
 *  \brief  Clear the display by setting every pixel to the background color
 */
void miniOledClear(void)
{
}

/*! \fn     miniOledSetFont(uint8_t fontIndex)
 *  \brief  Set the font to use
 *  \param  font    New font to use
 */
void miniOledSetFont(uint8_t fontIndex)
{fontIndex++;
/*
    if (oledGetFileAddr(fontIndex, &oledFontAddr) != MEDIA_FONT)
    {        
        #ifdef OLED_DEBUG
            usbPrintf_P(PSTR("oled failed to set font %d\n"),fontIndex);
        #endif
        return;
    }
    fontId = fontIndex;
    oledFontPage = oledFontAddr / BYTES_PER_PAGE;
    oledFontOffset = oledFontAddr % BYTES_PER_PAGE;
    flashRawRead((uint8_t *)&currentFont, oledFontAddr, sizeof(currentFont));*/

#ifdef OLED_DEBUG
    usbPrintf_P(PSTR("found font at file index %d\n"),fontIndex);
    usbPrintf_P(PSTR("oled set font %d\n"),fontIndex);
    oledDumpFont();
#endif
}