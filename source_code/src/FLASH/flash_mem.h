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

/* Copyright (c) 2014, Michael Neiderhauser. All rights reserved. */
/* Copyright (c) 2014, Darran Hunt. All rights reserved. */

/*!  \file     flash_mem.h
*    \brief    Mooltipass Flash IC Library Header
*    Created:  31/3/2014
*    Author:   Michael Neiderhauser
*/

#ifndef FLASH_MEM_H_
#define FLASH_MEM_H_

#include <stdbool.h>

#undef DEBUG_FLASH
#ifdef DEBUG_FLASH
#define DPRINTF_P(args...) usbPrintf_P(args)
#else
#define DPRINTF_P(args...)
#endif

#define FLASH_FAMILY_MASK         0xE0	// bitmask for family field in flash device ID
#define FLASH_FAMILY_ID           0x20  // The flash family supported
#define FLASH_DENSITY_MASK        0x1F  // bitmask for the flash density in the device ID

#define FLASH_OP_PAGE_READ        0xD2	// read one page
#define FLASH_OP_PAGE_WRITE	  0x82	// write one flash page via memory buffer with auto erase
#define FLASH_OP_PAGE_ERASE	  0x81	// erase one flash page
#define FLASH_OP_READ             0x03	// random access continuous read (low freq)
#define FLASH_OP_BUF_LOAD         0x53	// load memory buffer from page
#define FLASH_OP_BUF_READ         0xD1	// read from the memory buffer (low freq)
#define FLASH_OP_BUF_CMP          0x60	// compare memory buffer to page
#define FLASH_OP_BUF_WRITE        0x84	// write to the memory buffer
#define FLASH_OP_BUF_ERASE_STORE  0x83	// write the buffer to a flash page, no erase
#define FLASH_OP_BUF_STORE        0x88	// write the buffer to a flash page, no erase
#define FLASH_OP_CHIP_ERASE       0xC7, 0x94, 0x80, 0x9A	// erase entire chip
#define FLASH_OP_GET_STATUS       0xD7	// Read status
#define FLASH_OP_SECTOR_ERASE     0x7C  // Erase a sector
#define FLASH_OP_BLOCK_ERASE      0x50  // Erase a block

#define FLASH_STATUS_BUSY         (1<<7)  // flash status busy bit

#define FLASH_SECTOR_0A           0x0800 // Internal identifier for Sector 0a operations
#define FLASH_SECTOR_0B		  0x1000 // Internal identifier for Sector 0b operations

int flashInit(void);
int flashCheckId(void);
int flashBufLoad(uint16_t page);
void flashBufRead(void *datap, uint16_t offset, uint16_t size);
void flashRawRead(void *datap, uint32_t addr, uint16_t size);
int flashPageRead(void *datap, uint16_t page, uint16_t offset, uint16_t size);
int flashBufStore(uint16_t page);
int flashBufEraseStore(uint16_t page);

void flashChipErase(void);
int flashPageErase(uint16_t page);
int flashBlockErase(uint16_t block);
int flashSectorErase(uint16_t sector);

void flashBufWrite(void *datap, uint16_t offset, uint16_t size);
void flashBufFill(void *datap, uint16_t offset, uint16_t size, uint16_t repeat);
void flashBufSet(uint8_t value, uint16_t offset, uint16_t size);

void flashBufWriteCached(void *datap, uint16_t page, uint16_t offset, uint16_t size);
void flashBufSetCache(uint16_t page);
int flashPageWrite(void *datap, uint16_t page, uint16_t offset, uint16_t size);
bool flashFlushCache(uint16_t page);

void flashPageHexDump(uint16_t page);
#endif
