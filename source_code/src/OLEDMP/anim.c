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

/*!	\file 	anim.c
*	\brief	Animation support for oled bitmaps
*	Created: 20/7/2014
*	Author: Darran Hunt
*/

/* 
 * This is a static version of the frames that will
 * eventually be files in flash and include animation effects.
 *
 * For now, if oledBitmapDraw() is called with a file ID
 * of 128 or greater then animFrameDraw() will be called
 * with id set to the file ID - 128.
 */

#include <util/delay.h>
#include "touch_higher_level_functions.h"
#include "logic_aes_and_comms.h"
#include "timer_manager.h"
#include "logic_eeprom.h"
#include "oledmp.h"
#include "anim.h"
int16_t screensaver_anim_last_x=0, screensaver_anim_last_y=15;
int8_t screensaver_anim_xvel=3, screensaver_anim_yvel=2;
int16_t screensaver_anim_x=0, screensaver_anim_y=0;
#define ZZZ_WIDTH   20
#define ZZZ_HEIGHT  20


// Bounce a ball around...
void animScreenSaver(void)
{
    if (((screensaver_anim_x+screensaver_anim_xvel + ZZZ_WIDTH) > OLED_WIDTH) || (screensaver_anim_x+screensaver_anim_xvel < 0)) 
    {
        // bounce x
        screensaver_anim_xvel = -screensaver_anim_xvel;
    } 
    else 
    {
        screensaver_anim_x += screensaver_anim_xvel;
    }
    if (((screensaver_anim_y+screensaver_anim_yvel + ZZZ_HEIGHT) > OLED_HEIGHT) || (screensaver_anim_y+screensaver_anim_yvel < 0)) 
    {
        // bounce x
        screensaver_anim_yvel = -screensaver_anim_yvel;
    } 
    else 
    {
        screensaver_anim_y += screensaver_anim_yvel;
    }
        
    // Display different bitmap if locked
    uint8_t zzzbitmap = BITMAP_ZZZ_LOCKED;
    if (getSmartCardInsertedUnlocked() == TRUE)
    {
        zzzbitmap = BITMAP_ZZZ;
    }
    
    oledBitmapDrawFlash((uint8_t)screensaver_anim_x, (uint8_t)screensaver_anim_y, zzzbitmap, 0);
    oledDisplayOtherBuffer();
    timerBasedDelayMs(getMooltipassParameterInEeprom(SCREEN_SAVER_SPEED_PARAM));
    oledFillXY(screensaver_anim_last_x, screensaver_anim_last_y, ZZZ_WIDTH, ZZZ_HEIGHT, 0);

    screensaver_anim_last_x = screensaver_anim_x;
    screensaver_anim_last_y = screensaver_anim_y;
}
