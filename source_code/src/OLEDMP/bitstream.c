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

/*!	\file 	bitstream.c
*	\brief	Bitmap streamer library. Allows a raw packed bitmap to be read as a sequence
*	        of pixels.
*	Created: 15/2/2014
*	Author: Darran Hunt
*/

#include <avr/pgmspace.h>
#include "bitstream.h"
#include "flash_mem.h"
#include "usb.h"

#undef DEBUG_BS

/**
 * Initialise a bitstream ready for use
 * @param bs - pointer to the bitstream context to be used for the new bitmap
 * @param pixelDepth - number of bits per pixel in the bitmap
 * @param flags - bitmap format options (e.g. BS_RLE for compressed bitmap)
 * @param data - pointer to the bitmap data (16 bit words with packed pixels)
 * @param size - number of pixels in the bitmap data
 * @note the bitmap data must be packed into 16 bit words, and pixels are packed
 *       across words when they do not fully fit
 */
void bsInit(
    bitstream_t *bs,
    const uint8_t pixelDepth,
    const uint8_t flags,
    const uint16_t *data,
    const uint16_t width,
    const uint8_t height,
    bool flash,
    uint16_t addr)
{
    bs->bitsPerPixel = pixelDepth;
    bs->width = width;
    bs->height = height;
    bs->_datap = data;
    bs->_cdatap = (const uint8_t *)data;
    bs->_size = width * height;
    bs->mask = (1 << pixelDepth) - 1;
    bs->_wordsize = 16;
    bs->_bits = 0;
    bs->_word = 0xAA55;
    bs->_count = 0;
    bs->_flags = flags;
    bs->flash = flash;
    bs->addr = addr;
#ifdef DEBUG_BS
    usbPrintf_P(PSTR("bitmap: data %p, depth %d, size %d, width %u, height %u\n"), data, pixelDepth, bs->_size, width, height);
    usbPrintf_P(PSTR("bitmap: addr %04X\n"), bs->addr);
#endif
}

/**
 * Return the next data word from flash
 * @param bs - pointer to initialised bitstream context to get the next word from
 * @returns next data word.
 * @note when no more data is available in the bitstream, calls to this function will
 * return 0. Also note that 0 does not indicate an end of the bitstream.
 */
static inline uint16_t bsGetNextWord(bitstream_t *bs)
{
    if (bs->_count < bs->_size) 
    {
        bs->_count++;
#ifdef OLED_FEATURE_PGM_MEMORY
        if (bs->flash) 
        {
            return (uint16_t)pgm_read_word(bs->_datap++);
        } else
#endif
        if (bs->addr)
        {
            uint16_t data;
            flashRawRead((uint8_t *)&data, bs->addr, sizeof(data));
            //usbPrintf_P(PSTR("bs: 0x%04x = 0x%04x\n"), bs->addr, data);
            bs->addr += 2;
            return data;
        }
        else
        {
            return *bs->_datap++;
        }
    } 
    else 
    {
        return 0;
    }
}

/**
 * Return the next data byte from flash
 * @param bs - pointer to initialised bitstream context to get the next word from
 * @returns next data word, or 0 if end of data reached
 */
static inline uint8_t bsGetNextByte(bitstream_t *bs)
{
    if (bs->_count < bs->_size) 
    {
        bs->_count++;
#ifdef OLED_FEATURE_PGM_MEMORY
        if (bs->flash) 
        {
            return pgm_read_byte(bs->_cdatap++);
        } else
#endif
        if (bs->addr)
        {
            uint8_t data;
            // XXX very ineficient. TODO add a local cache
            flashRawRead((uint8_t *)&data, bs->addr, sizeof(data));
            //usbPrintf_P(PSTR("bs: 0x%04x = 0x%02x\n"), bs->addr, data);
            bs->addr++;
            return data;
        }
        else 
        {
            return *bs->_cdatap++;
        }
    }
    else
    {
        return 0;
    }
}


/**
 * Return the next pixel from the bitmap
 * @param bs - pointer to initialised bitstream context to read the next pixel from
 * @param numPixes - the number of pixels to read,
 * @returns next pixel, or 0 if end of data reached
 * @note returned pixes are 4 bits each
 */
uint16_t bsRead(bitstream_t *bs, uint8_t numPixels)
{
    uint16_t data=0;
    if (bs->_flags & BS_RLE) 
    {
        while (numPixels--) 
        {
            data <<= 4;
            if (bs->_bits == 0) 
            {
                uint8_t byte = bsGetNextByte(bs);
                bs->_bits = (byte >> 4) + 1;
                bs->_pixel = byte & 0x0F;
            }
            if (bs->_bits) 
            {
                data |= bs->_pixel;
                bs->_bits--;
            }
        }
    }
    else
    {
        while (numPixels--) 
        {
            data <<= 4;
            if (bs->_bits == 0) 
            {
                bs->_word = bsGetNextWord(bs);
                bs->_bits = bs->_wordsize;
            }
            if (bs->_bits >= bs->bitsPerPixel) 
            {
                bs->_bits -= bs->bitsPerPixel;
                data |= (((bs->_word >> bs->_bits) & bs->mask) * 15) / bs->mask;
#ifdef DEBUG_BS
                usbPrintf_P(PSTR("pixel: 0x%x (bits=%d, word=0x%04x)\n"), (((bs->_word >> bs->_bits) & bs->mask) * 15) / bs->mask,
                        bs->_bits, bs->_word);
#endif
            }
            else 
            {
                uint8_t offset = bs->bitsPerPixel - bs->_bits;
                data |= (bs->_word << offset & bs->mask);
                bs->_bits += bs->_wordsize - bs->bitsPerPixel;
                bs->_word = bsGetNextWord(bs);
                data |= ((bs->_word >> bs->_bits) * 15) / bs->mask;
#ifdef DEBUG_BS
                usbPrintf_P(PSTR("pixel: 0x%x (bits=%d, word=0x%04x) new word\n"), (((bs->_word >> bs->_bits) & bs->mask) * 15) / bs->mask,
                        bs->_bits, bs->_word);
#endif
            }
        }
    }
    return data;
}

/**
 * Return the next pixel from the RLE compressed bitmap
 * @param bs - pointer to initialised bitstream context to read the next pixel from
 * @param numPixes - the number of pixels to read,
 * @returns next pixel, or 0 if end of data reached
 * @note returned pixes are 4 bits each
 */
uint16_t bsCompressedRead(bitstream_t *bs, uint8_t numPixels)
{
    uint16_t data=0;

    while (numPixels--) 
    {
        data <<= 4;
        if (bs->_bits == 0) 
        {
            uint8_t byte = bsGetNextByte(bs);
            bs->_bits = (byte >> 4) + 1;
            bs->_pixel = byte & 0x0F;
        }
        if (bs->_bits) 
        {
            data |= bs->_pixel;
            bs->_bits--;
        }
    }
    return data;
}

/**
 * Returns the number of pixels available to read
 * @param bs - pointer to initialised bitstream context to check available pixels with
 * @returns number of pixels available to read
 */
uint16_t bsAvailable(bitstream_t *bs)
{
    return bs->_size;
}
