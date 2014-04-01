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

/*!  \file     flash_mem.h
*    \brief    Mooltipass Flash IC Library Header
*    Created:  31/3/2014
*    Author:   Michael Neiderhauser
*/

#ifndef FLASH_MEM_H_
#define FLASH_MEM_H_

RET_TYPE sendDataToFlash(uint8_t opcodeSize, uint8_t* opcode, uint16_t bufferSize, uint8_t* buffer);
RET_TYPE waitForFlash(void);
RET_TYPE checkFlashID(void);
RET_TYPE initFlash(void);

// Erase Functions
RET_TYPE sectorZeroErase(uint8_t sectorNumber);
RET_TYPE sectorErase(uint8_t sectorNumber);
RET_TYPE blockErase(uint16_t blockNumber);
RET_TYPE pageErase(uint16_t pageNumber);

RET_TYPE writeDataToFlash(uint16_t pageNumber, uint16_t offset, uint16_t dataSize, uint8_t* data);
RET_TYPE readDataFromFlash(uint16_t pageNumber, uint16_t offset, uint16_t dataSize, uint8_t* data);

//void readCredentialBlock(uint16_t page_number, uint8_t block_id, uint8_t* buffer)__attribute__((deprecated));


#endif /* FLASH_MEM_H_ */