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
/*! \file   bitstreammini.c
 *  \brief  Bitstream functions
 *  Created: 15/2/2016
 *  Copyright [2016] [Mathieu Stephan]
 */
#include <avr/pgmspace.h>
#include "bitstreammini.h"
#include "flash_mem.h"
#include "usb.h"
/***********************************************************/
/*  This file is only used for the Mooltipass mini version */
#if defined(MINI_VERSION)


/*! \fn     miniBistreamInit(bitstream_mini_t* bs, uint8_t height, uint16_t width, uint16_t addr)
 *  \brief  Initialise a bitstream ready for use
 *  \param  bs      pointer to the bitstream context to be used for the new bitmap
 *  \param  height  data height
 *  \param  width   data width
 *  \param  addr    address of the bitmap in SPI flash
 */
void miniBistreamInit(bitstream_mini_t* bs, uint8_t height, uint16_t width, uint16_t addr)
{
    // In the data storage height can be any value but one y line is stored in blocks of 8bits (eg: 10pixels height > 2 bytes)
    bs->addr = addr;
    bs->width = width;
    bs->height = height;
    bs->dataCounter = 0;
    bs->bufferInd = sizeof(bs->buffer);
    bs->dataSize = (uint16_t)width * (((uint16_t)height+7) >> BITSTREAM_PIXELS_PER_BYTE_BITSHIFT);
}

/*! \fn     bsGetNextByte(bitstream_mini_t* bs)
 *  \brief  Return the next data byte from flash
 *  \param  bs      pointer to initialized bitstream context to get the next word from
 *  \return next data word, or 0 if end of data reached
 */
uint8_t miniBistreamGetNextByte(bitstream_mini_t* bs)
{
    // Are you requesting bytes when you've already read everything?
    if (bs->dataCounter < bs->dataSize) 
    {
        bs->dataCounter++;
        
        // Check if we need to fetch new data from the SPI flash
        if(bs->bufferInd >= sizeof(bs->buffer))
        {
            // Fetch new data from external flash
            flashRawRead(bs->buffer, bs->addr, sizeof(bs->buffer));
            bs->addr += sizeof(bs->buffer);
            bs->bufferInd = 0;
            //usbPrintf_P(PSTR("bistream buffer: %02x %02x %02x %02x %02x %02x"), bs->buffer[0], bs->buffer[1], bs->buffer[2], bs->buffer[3], bs->buffer[4], bs->buffer[5]);
        }
        
         return bs->buffer[bs->bufferInd++];
    }
    else
    {
        return 0;
    }
}
#endif
/***************************************************************/