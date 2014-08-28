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

/*!
 * \file    bitstream.h
 * \brief   OLED library header
 * Created: 15/2/2014
 * Author:  Darran Hunt
 */

#ifndef BITSTREAM_H_
#define BITSTREAM_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct 
{
    uint8_t mask;               //*< pixel mask for returned data
    uint16_t width;             //*< number of pixels wide
    uint8_t height;             //*< number of pixels high
    uint8_t bitsPerPixel;       //*< number of bits per pixel
    const uint16_t *_datap;     //*< Next data word to read
    const uint8_t *_cdatap;     //*< Next compressed byte to read
    uint16_t _size;             //*< number of pixels
    uint8_t _bits;              //*< current offset in data word
    uint8_t _wordsize;          //*< number of bits per data word
    uint16_t _word;             //*< current bitmap word
    uint16_t _count;            //*< number of bytes / words read
    uint8_t _pixel;             //*< current pixel for RLE decompress
    uint8_t _flags;		//*< format flags.  E.g. RLE=1
    bool flash;			//*< true if data is in program memory
    uint16_t addr;		//*< address of data in SPI FLASH store.
} bitstream_t;

#define BS_RLE	0x01		//*< Bitmap is compressed using run-length compression

void bsInit(
    bitstream_t *bs,
    const uint8_t pixelDepth,
    const uint8_t flags,
    const uint16_t *data,
    const uint16_t width,
    const uint8_t height,
    bool flash,
    uint16_t addr);
uint16_t bsRead(bitstream_t *bs, uint8_t numPixels);
uint16_t bsAvailable(bitstream_t *bs);

#endif
