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

#ifndef STORE_H_
#define STORE_H_

void storeInit(void);
uint8_t storeFreeSlot(uint8_t slotId);
uint8_t storeAllocateSlot(uint16_t size);
uint8_t storeWriteSlot(uint8_t slotId, uint16_t size, void *datap);
uint16_t storeReadSlot(uint8_t slotId, uint16_t offset, uint16_t size, void *datap);

#endif
