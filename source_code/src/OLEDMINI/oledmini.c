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
 *  Created: 15/2/2016
 *  Copyright [2016] [Mathieu Stephan]
 */
#include "logic_fwflash_storage.h"
#include "bitstreammini.h"
#include "timer_manager.h"
#include <avr/pgmspace.h>
#include "flash_mem.h"
#include "oledmini.h"
#include "defines.h"
#include <string.h>
#include "spi.h"
// Frame buffer, first byte is X0 Y7 (MSB) to X0 Y0 (LSB)
uint8_t miniOledFrameBuffer[SSD1305_OLED_WIDTH*SSD1305_OLED_HEIGHT/SSD1305_PAGE_HEIGHT];
// Current y offset in buffer
uint8_t miniOledBufferYOffset;
// Boolean to know if OLED on
uint8_t miniOledIsOn = FALSE;

// OLED initialization sequence
static const uint8_t mini_oled_init[] __attribute__((__progmem__)) = 
{
    1,  SSD1305_CMD_DISPLAY_OFF,                                        // Display Off
    2,  SSD1305_CMD_SET_DISPLAY_CLOCK_DIVIDE,   0x10,                   // Display divide ratio of 0, Oscillator frequency of around 300kHz
    2,  SSD1305_CMD_SET_MULTIPLEX_RATIO,        0x1F,                   // Multiplex ratio of 32
    2,  SSD1305_CMD_SET_DISPLAY_OFFSET,         0x00,                   // Display offset 0
    1,  SSD1305_CMD_SET_DISPLAY_START_LINE,                             // Display start line 0
    2,  SSD1305_CMD_SET_MASTER_CONFIGURATION,   0x8E,                   // Select external Vcc supply
    2,  SSD1305_CMD_SET_AREA_COLOR_MODE,        0x05,                   // Set low power display mode
    1,  SSD1305_CMD_SET_SEGMENT_REMAP_COL_131,                          // Column address 131 is mapped to SEG0 
    1,  SSD1305_CMD_COM_OUTPUT_REVERSED,                                // Remapped mode. Scan from COM[N~1] to COM0
    2,  SSD1305_CMD_SET_MEM_ADDRESSING_MODE,    0x00,                   // Horizontal addressing mode
    2,  SSD1305_CMD_SET_COM_PINS_CONF,          0x12,                   // Alternative COM pin configuration
    5,  SSD1305_CMD_SET_LUT,                    0x3F,0x3F,0x3F,0x3F,    // Set Look up Table
    2,  SSD1305_CMD_SET_CONTRAST_CURRENT,       SSD1305_OLED_CONTRAST,  // Set current control (contrast)
    2,  SSD1305_CMD_SET_PRECHARGE_PERIOD,       0xD2,                   // Precharge period
    2,  SSD1305_CMD_SET_VCOMH_VOLTAGE,          0x08,                   // VCOM deselect level (around 0.5Vcc)
    1,  SSD1305_CMD_ENTIRE_DISPLAY_NORMAL,                              // Entire display in normal mode
    1,  SSD1305_CMD_ENTIRE_DISPLAY_NREVERSED,                           // Entire display not reversed
};


/*! \fn     miniOledWriteCommand(uint8_t* data, uint8_t nbBytes)
 *  \brief  Write a command or register address to the display
 *  \param  data    Pointer to the data to be written
 *  \param  nbBytes Number of bytes to be written
 */
void miniOledWriteCommand(uint8_t* data, uint8_t nbBytes)
{
    PORT_OLED_SS &= ~(1 << PORTID_OLED_SS);
    PORT_OLED_DnC &= ~(1 << PORTID_OLED_DnC);
    while(nbBytes--)
    {
        spiUsartTransfer(*data);
        data++;
    }
    PORT_OLED_SS |= (1 << PORTID_OLED_SS);
}

/*! \fn     miniOledWriteSimpleCommand(uint8_t reg)
 *  \brief  Write a command or register address to the display
 *  \param  reg     the command or register to write
 */
void miniOledWriteSimpleCommand(uint8_t reg)
{
    PORT_OLED_SS &= ~(1 << PORTID_OLED_SS);
    PORT_OLED_DnC &= ~(1 << PORTID_OLED_DnC);
    spiUsartTransfer(reg);
    PORT_OLED_SS |= (1 << PORTID_OLED_SS);
}

/*! \fn     miniOledWriteData(uint8_t* data, uint16_t nbBytes)
 *  \brief  Write a command or register address to the display
 *  \param  data    Pointer to the data to be written
 *  \param  nbBytes Number of bytes to be written
 */
void miniOledWriteData(uint8_t* data, uint16_t nbBytes)
{
    PORT_OLED_SS &= ~(1 << PORTID_OLED_SS);
    PORT_OLED_DnC |= (1 << PORTID_OLED_DnC);
    while(nbBytes--)
    {
        spiUsartTransfer(*data);
        data++;
    }
    PORT_OLED_SS |= (1 << PORTID_OLED_SS);
}

/*! \fn     miniOledSetColumnAddress(uint8_t columnStart, uint8_t columnEnd)
 *  \brief  Setup column start and end address
 *  \param  columnStart   Start column
 *  \param  columnEnd     End column
 */
void miniOledSetColumnAddress(uint8_t columnStart, uint8_t columnEnd)
{
    uint8_t data[3] = {SSD1305_CMD_SET_COLUMN_ADDR, columnStart + SSD1305_X_OFFSET, columnEnd + SSD1305_X_OFFSET};
    miniOledWriteCommand(data, sizeof(data));
}

/*! \fn     miniOledSetPageAddress(uint8_t pageStart, uint8_t pageEnd)
 *  \brief  Setup page start and end address
 *  \param  pageStart   Start page
 *  \param  pageEnd     End page
 */
void miniOledSetPageAddress(uint8_t pageStart, uint8_t pageEnd)
{
    uint8_t data[3] = {SSD1305_CMD_SET_PAGE_ADDR, pageStart, pageEnd};
    miniOledWriteCommand(data, sizeof(data));
}

/*! \fn     miniOledSetWindow(uint8_t columnStart, uint8_t columnEnd, uint8_t pageStart, uint8_t pageEnd)
 *  \brief  Set the window we want to update in the screen
 *  \param  columnStart Start column
 *  \param  columnEnd   End column
 *  \param  pageStart   Start page
 *  \param  pageEnd     End page
 */
void miniOledSetWindow(uint8_t columnStart, uint8_t columnEnd, uint8_t pageStart, uint8_t pageEnd)
{
    miniOledSetColumnAddress(columnStart, columnEnd);
    miniOledSetPageAddress(pageStart, pageEnd);
}

/*! \fn     miniOledFlushBufferContents(uint8_t x, uint8_t y)
 *  \brief  Flush buffer contents to the display
 *  \param  xstart  From which x to start flushing
 *  \param  xend    end x position
 *  \param  ystart  From which y to start flushing
 *  \param  yend    end y position
 */
void miniOledFlushBufferContents(uint8_t xstart, uint8_t xend, uint8_t ystart, uint8_t yend)
{
    // Compute page start & page end
    uint8_t page_start = ystart >> SSD1305_PAGE_HEIGHT_BIT_SHIFT;
    uint8_t page_end = yend >> SSD1305_PAGE_HEIGHT_BIT_SHIFT;
    
    // Set the correct display window
    miniOledSetWindow(xstart, xend, page_start, page_end);
    
    // Send data, we go low level to have better speed
    PORT_OLED_SS &= ~(1 << PORTID_OLED_SS);
    PORT_OLED_DnC |= (1 << PORTID_OLED_DnC);
    for (uint8_t page = page_start; page <= page_end; page++)
    {
        uint16_t buffer_shift = ((uint16_t)page) << SSD1305_WIDTH_BIT_SHIFT;
        for (uint8_t x = xstart; x <= xend; x++)
        {
            // TODO: switch sending techniques
            spiUsartTransfer(miniOledFrameBuffer[buffer_shift + x]);
            //spiUsartSendTransfer(miniOledFrameBuffer[(uint16_t)x + buffer_shift]);
        }
    }
    //spiUsartWaitEndSendTransfer();
    PORT_OLED_SS |= (1 << PORTID_OLED_SS); 
}

/*! \fn     miniOledOn(void)
 *  \brief  Turn the display on
 */
void miniOledOn(void)
{
    PORT_OLED_POW &= ~(1 << PORTID_OLED_POW);
    timerBased130MsDelay();
    miniOledWriteSimpleCommand(SSD1305_CMD_DISPLAY_NORMAL_MODE);
    miniOledIsOn = TRUE;
}

/*! \fn     miniOledInit(void)
 *  \brief  Initialize the OLED hardware and get it ready for use.
 */
void miniOledInit(void)
{
    uint8_t dataBuffer[10];
    uint8_t dataSize, i;
    
    // Parse initialization sequence
    for (uint8_t ind=0; ind<sizeof(mini_oled_init);) 
    {
        i = 0;
        dataSize = pgm_read_byte(&mini_oled_init[ind++]);
        while (dataSize--) 
        {
            dataBuffer[i++] = pgm_read_byte(&mini_oled_init[ind++]);
        }
        miniOledWriteCommand(dataBuffer, i);
    }

    // Switch on screen
    miniOledClear();
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

/*! \fn     miniOledDrawRectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height)
 *  \brief  Draw a rectangle on the screen
 *  \param  x       X position
 *  \param  y       Y position
 *  \param  width   width
 *  \param  height  height
 *  \param  full    boolean for color or not
 */
void miniOledDrawRectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t full)
{
    // Compute page start & page end
    uint8_t page_start = y >> SSD1305_PAGE_HEIGHT_BIT_SHIFT;
    uint8_t page_end = (y+height-1) >> SSD1305_PAGE_HEIGHT_BIT_SHIFT;
    
    // Compute mask settings
    uint8_t f_bitshift_mask[] = {0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF};
    uint8_t f_bitshift = (y+height-1) & 0x07;
    uint8_t l_bitshift_mask[] = {0xFF, 0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80};
    uint8_t l_bitshift = y & 0x07;
    
    for(uint8_t page = page_start; page <= page_end; page++)
    {
        uint16_t buffer_shift = ((uint16_t)page) << SSD1305_WIDTH_BIT_SHIFT;
        for(uint8_t xpos = x; xpos < x+width; xpos++)
        {
            uint8_t or_mask = 0xFF;
            uint8_t and_mask = 0x00;
            if(page == page_start)
            {
                if(full == TRUE)
                {
                    or_mask = l_bitshift_mask[l_bitshift];
                }
                else
                {
                    and_mask = ~l_bitshift_mask[l_bitshift];
                }                
            }
            if(page == page_end)
            {
                if(full == TRUE)
                {
                    or_mask = or_mask & f_bitshift_mask[f_bitshift];
                }
                else
                {
                    and_mask = and_mask | ~f_bitshift_mask[f_bitshift];
                }
            }
            if(page != page_start && page != page_end)
            {
                if(full == TRUE)
                {
                    or_mask = 0xFF;
                }
                else
                {
                    and_mask = 0x00;
                }
            }
            if (full == TRUE)
            {
                miniOledFrameBuffer[buffer_shift+xpos] |= or_mask;
            }
            else
            {
                miniOledFrameBuffer[buffer_shift+xpos] &= and_mask;
            }
        }
    }
}

/*! \fn     miniOledClear(void)
 *  \brief  Clear the display by setting every pixel to the background color
 */
void miniOledClear(void)
{
    memset(miniOledFrameBuffer, 0x00, sizeof(miniOledFrameBuffer));
    miniOledFlushBufferContents(0, SSD1305_OLED_WIDTH, 0, SSD1305_OLED_HEIGHT);
}

/*! \fn     miniOledGetFileAddr(uint8_t fileId, uint16_t *addr)
 *  \brief  Return the address and type of the media file for the specified file id
 *  \param  fileId  index of the media file to read
 *  \param  addr    pointer to address to fill
 *  \return -1 on error, else the type of the media file.
 *  \note   Media files start after string files, first is BITMAP_ID_OFFSET
 */
int16_t miniOledGetFileAddr(uint8_t fileId, uint16_t *addr)
{
    uint16_t type;

    if (getStoredFileAddr((uint16_t)fileId+BITMAP_ID_OFFSET, addr) == RETURN_NOK)
    {
        return -1;
    }
    
    flashRawRead((uint8_t *)&type, *addr, sizeof(type));
    *addr += sizeof(type);
    #ifdef OLED_DEBUG
        usbPrintf_P(PSTR("oledGetFileAddr file %d type 0x%x addr 0x%04x\n"), fileId, type, *addr);
    #endif

    return type;
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

/*! \fn     miniOledBitmapDrawRaw(uint8_t x, uint8_t y, bitstream_mini_t* bs, uint8_t options)
 *  \brief  Draw a rectangular bitmap on the screen at x,y
 *  \param  x       x position for the bitmap
 *  \param  y       y position for the bitmap (0=top, 63=bottom)
 *  \param  bs      pointer to the bitstream object
 *  \param  options display options:
 *                  OLED_SCROLL_UP - scroll bitmap up
 *                  OLED_SCROLL_DOWN - scroll bitmap down
 *                  0 - don't make bitmap active (unless already drawing to active buffer)
 */
void miniOledBitmapDrawRaw(uint8_t x, uint8_t y, bitstream_mini_t* bs, uint8_t options)
{
    (void)options;
    uint8_t start_x = x;
    uint8_t end_x = x + bs->width - 1;
    uint8_t start_page = y >> SSD1305_PAGE_HEIGHT_BIT_SHIFT;
    uint8_t end_page = (y + bs->height) >> SSD1305_PAGE_HEIGHT_BIT_SHIFT;
    
    for (uint8_t page = start_page; page <= end_page; page++)
    {
        //uint16_t buffer_shift = ((uint16_t)page) >> SSD1305_WIDTH_BIT_SHIFT;
        for (uint8_t x = start_x; x <= end_x; x++)
        {
            miniBistreamGetNextByte(bs);
        }
    }   
}

/*! \fn     miniOledBitmapDrawFlash(uint8_t x, uint8_t y, uint8_t fileId, uint8_t options)
 *  \brief  Draw a bitmap from a Flash storage slot
 *  \param  x       x position for the bitmap
 *  \param  y       y position for the bitmap (0=top, 63=bottom)
 *  \param  addr    address of the bitmap in flash
 *  \param  options display options:
 *                  OLED_SCROLL_UP - scroll bitmap up
 *                  OLED_SCROLL_DOWN - scroll bitmap down
 *                  0 - don't make bitmap active (unless already drawing to active buffer)
 */
void miniOledBitmapDrawFlash(uint8_t x, uint8_t y, uint8_t fileId, uint8_t options)
{
    bitstream_mini_t bs;
    bitmap_t bitmap;
    uint16_t addr;

    // Get address of our bitmap in the external flash
    if (miniOledGetFileAddr(fileId, &addr) != MEDIA_BITMAP)
    {
        return;
    }

    // Read bitmap header
    flashRawRead((uint8_t*)&bitmap, addr, sizeof(bitmap));
    
    // Initialize bitstream (pixel data starts right after the header)
    miniBistreamInit(&bs, bitmap.height, bitmap.width, addr+sizeof(bitmap));
    
    // Draw the bitmap
    miniOledBitmapDrawRaw(x, y, &bs, options);
}