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

/*!	\file 	anim.h
*	\brief	Animation support for oled bitmaps
*	Created: 20/7/2014
*	Author: Darran Hunt
*/

#ifndef ANIM_H_
#define ANIM_H_

#include "logic_fwflash_storage.h"

#define FRAME_SIDES           0
#define FRAME_TICK_CROSS      1
#define FRAME_YES_NO          2
#define FRAME_PIN_ENTRY       3

// Bitmaps that oledBitmapDrawFlash() can show
#define BITMAP_HAD            0
#define BITMAP_LOGIN          1
#define BITMAP_LEFT           2
#define BITMAP_RIGHT          3
#define BITMAP_TICK           4
#define BITMAP_CROSS          5
#define BITMAP_INFO           6
#define BITMAP_INSERT         9
#define BITMAP_MAIN_SCREEN    10
#define BITMAP_SETTINGS_SC    11
#define BITMAP_LEFT_ARROW     12
#define BITMAP_RIGHT_ARROW    13
#define BITMAP_PIN_LINES      14

// These bitmaps are built from frames
#define BITMAP_SIDES          (0x80+FRAME_SIDES)
#define BITMAP_TICK_CROSS     (0x80+FRAME_TICK_CROSS)
#define BITMAP_YES_NO         (0x80+FRAME_YES_NO)
#define BITMAP_PIN_ENTRY      (0x80+FRAME_PIN_ENTRY)

// Animation frame bitmap instance that specifies a bitmap and its location
typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t id;
} animFrame_t;

int8_t animFrameDraw(uint8_t x, uint8_t y, uint8_t frameId, uint8_t options);

#endif
