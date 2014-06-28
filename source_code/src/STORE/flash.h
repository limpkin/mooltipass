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

/*!  \file     flash.c
*    \brief    Mooltipass Flash extended functions
*    Created:  24/6/2014
*    Author:   Darran Hunt
*/

#ifndef FLASH_H_
#define FLASH_H_

#include <stdbool.h>

// Flash operations
#define FLASH_OP_BUF_LOAD         0x53  // load memory buffer from page
#define FLASH_OP_PAGE_WRITE       0x82  // write one flash page via memory buffer with auto erase
#define FLASH_OP_BUF_ERASE_STORE  0x83  // write the buffer to a flash page, no erase
#define FLASH_OP_BUF_WRITE        0x84	// write to the memory buffer
#define FLASH_OP_BUF_STORE        0x88  // write the buffer to a flash page, no erase

int flashRead(uint8_t *datap, uint32_t addr, uint16_t size);
void flashBufWrite(void *datap, uint16_t offset, uint16_t size);
int flashBufLoad(uint16_t page);
int flashBufStore(uint16_t page, bool erase);
int flashWritePage(void *datap, uint16_t page, uint16_t offset, uint16_t size);
int flashWrite(void *datap, uint32_t addr, uint16_t size);

#endif
