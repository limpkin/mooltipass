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

/*!	\file 	bitsream.c
*	\brief	Bitmap streamer library. Allows a raw packed bitmap to be read as a sequence
*	        of pixels.
*	Created: 15/2/2014
*	Author: Darran Hunt
*/

#include <avr/pgmspace.h>
#include "bitstream.h"

/**
 * Initialise a bitstream ready for use
 * @param bs - pointer to the bitstream context to be used for the new bitmap
 * @param pixelDepth - number of bits per pixel in the bitmap
 * @param data - pointer to the bitmap data (16 bit words with packed pixels)
 * @param size - number of pixels in the bitmap data
 * @note the bitmap data must be packed into 16 bit words, and pixels are packed
 *       across words when they do not fully fit
 */
void bs_init(bitstream_t *bs, const uint8_t pixelDepth, const uint16_t *data, const uint16_t size)
{
    bs->bitsPerPixel = pixelDepth;
    bs->_datap = data;
    bs->_size = size;
    bs->mask = (1 << pixelDepth) - 1;
    bs->_wordsize = 16;
    bs->_bits = 0;
    bs->_word = 0xAA55;
    bs->_count = 0;
}

/**
 * Return the next data word from flash
 * @param bs - pointer to initialised bitstream context to get the next word from
 * @returns next data word, or 0 if end of data reached
 */
uint16_t bs_getNextWord(bitstream_t *bs)
{
    if (bs->_size > 0) {
	return (uint16_t)pgm_read_word(bs->_datap++);
    } else {
	return 0;
    }
}

/**
 * Return the next pixel from the bitmap
 * @param bs - pointer to initialised bitstream context to read the next pixel from
 * @param numPixes - the number of pixels to read,
 * @returns next pixel, or 0 if end of data reached
 */

uint16_t bs_read(bitstream_t *bs, uint8_t numPixels)
{
    uint16_t data=0;

    while (numPixels--) {
	data <<= bs->bitsPerPixel;
	if (bs->_size > 0) {
	    if (bs->_bits == 0) {
		bs->_word = bs_getNextWord(bs);
		bs->_bits = bs->_wordsize;
	    }
	    if (bs->_bits >= bs->bitsPerPixel) {
		bs->_bits -= bs->bitsPerPixel;
		data |= (bs->_word >> bs->_bits) & bs->mask;
	    } else {
		uint8_t offset = bs->bitsPerPixel - bs->_bits;
		data |= (bs->_word << offset & bs->mask);
		bs->_bits += bs->_wordsize - bs->bitsPerPixel;
		bs->_word = bs_getNextWord(bs);
		data |= bs->_word >> bs->_bits;
	    }
	    bs->_size--;
	}
    }
    return data;
}

/**
 * Returns the number of pixels available to read
 * @param bs - pointer to initialised bitstream context to check available pixels with
 * @returns number of pixels available to read
 */
uint16_t bs_available(bitstream_t *bs)
{
    return bs->_size;
}
