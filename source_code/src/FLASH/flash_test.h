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

/*!  \file     flash_test.h
*    \brief    Mooltipass Flash IC Library Test Functions Header
*    Created:  31/3/2014
*    Author:   Michael Neiderhauser
*/

#ifndef FLASH_TEST_H_
#define FLASH_TEST_H_

#include "defines.h"
#include <stdint.h>

void initBuffer(uint8_t* buffer, uint16_t bufferSize, uint8_t policy);

RET_TYPE flashInitTest(void);

RET_TYPE flashWriteReadTest(uint8_t* bufferIn, uint8_t* bufferOut, uint16_t bufferSize);
RET_TYPE flashWriteReadOffsetTest(uint8_t* bufferIn, uint8_t* bufferOut, uint16_t bufferSize);

RET_TYPE flashErasePageTest(uint8_t* bufferIn, uint8_t* bufferOut, uint16_t bufferSize);
RET_TYPE flashEraseBlockTest(uint8_t* bufferIn, uint8_t* bufferOut, uint16_t bufferSize);

RET_TYPE flashEraseSectorXTest(uint8_t* bufferIn, uint8_t* bufferOut, uint16_t bufferSize);
RET_TYPE flashEraseSectorZeroTest(uint8_t* bufferIn, uint8_t* bufferOut, uint16_t bufferSize);

RET_TYPE flashTest(void);


// Flash Testing Defines
#define FLASH_TEST_INIT_BUFFER_POLICY_ZERO           0
#define FLASH_TEST_INIT_BUFFER_POLICY_ONE            1
#define FLASH_TEST_INIT_BUFFER_POLICY_INC            2
#define FLASH_TEST_INIT_BUFFER_POLICY_RND            3

#endif /* FLASH_TEST_H_ */