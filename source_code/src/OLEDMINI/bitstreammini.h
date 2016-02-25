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
/*! \file   bitstreammini.h
 *  \brief  Bitstream functions
 *  Created: 15/2/2016
 *  Copyright [2016] [Mathieu Stephan]
 */
#include <stdint.h>
#include <stdbool.h>

#ifndef BITSTREAMMINI_H_
#define BITSTREAMMINI_H_

/** BIT STREAM DEFINES **/
#define BITSTREAM_BUFFER_SIZE               16  // Bitstream buffer size
#define BITSTREAM_PIXELS_PER_BYTE_BITSHIFT  3   // Number of pixels per byte

/** STRUCTS **/
typedef struct
{
    uint8_t height;             // number of pixels high
    uint16_t width;             // number of pixels wide
    uint16_t dataSize;          // total data size
    uint16_t dataCounter;       // current counter
    uint16_t addr;              // address of data in SPI FLASH store
    uint8_t buffer[16];         // read ahead buffer
    uint8_t bufferInd;          // read ahead buffer index
} bitstream_mini_t;

/** PROTOTYPES **/
uint8_t miniBistreamGetNextByte(bitstream_mini_t* bs);
void miniBistreamInit(bitstream_mini_t* bs, uint8_t height, uint16_t width, uint16_t addr);

#endif /* BITSTREAMMINI_H_ */