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

#include "oledmp.h"
#include "anim.h"

static animFrame_t frameSides[] = {
    {   0,  0, BITMAP_LEFT  },
    { 218,  0, BITMAP_RIGHT },
};

static animFrame_t frameTickCross[] = {
    {  20, 24, BITMAP_CROSS },
    { 219, 24, BITMAP_TICK  }
};


// Yes / No question frame
static animFrame_t frameYesNo[] = {
    {   0,  0, BITMAP_SIDES  },
    {   0,  0, BITMAP_TICK_CROSS },
};

static animFrame_t framePinEntry[] = {
    {  25,  0, BITMAP_LEFT  },
    {   2, 26, BITMAP_CROSS  },
    { 195,  0, BITMAP_RIGHT  },
    {  80, 51, BITMAP_PIN_LINES },
    { 235, 23, BITMAP_RIGHT_ARROW },
};

/*
 * Frames
 *
 * A frame is a collection of bitmaps that are drawn at 
 * different x,y location to create a combined image.
 */

#define FRAME(name) { sizeof(name) / sizeof(name[0]), name }

static struct {
    uint8_t bitmapCount;    // *< The number of bitmaps in the frame
    animFrame_t *frame;     // *< Pointer to array of bitmaps
} animFrames[] = {
    FRAME(frameSides),
    FRAME(frameTickCross),
    FRAME(frameYesNo),
    FRAME(framePinEntry),
};

/**
 * Draw the specified frame at x,y.
 * @param x - x coordinate for top left corner
 * @param y - y coordinate for top left corner
 * @param options - display options:
 *                OLED_SCROLL_UP - scroll frame up
 *                OLED_SCROLL_DOWN - scroll frame down
 *                0 - don't make frame active (unless already drawing to active buffer)
 * @retval -1 failed, invalid frame id
 * @retval 0 success
 * @note top left of OLED display is 0,0 and bottom right is 255,63.
 */
int8_t animFrameDraw(uint8_t x, uint8_t y, uint8_t frameId, uint8_t options)
{
    if (frameId >= (sizeof(animFrames)/sizeof(animFrames[0])))
    {
        // outside range of available frames
        return -1;
    }

    uint8_t count = animFrames[frameId].bitmapCount;
    animFrame_t *frame = animFrames[frameId].frame;

    // construct the frame by drawing each bitmap from flash
    while (count--)
    {
        oledBitmapDrawFlash(x+frame->x, y+frame->y, frame->id, 0);
        frame++;
    }

    // Scroll option set?
    if (options)
    {
        oledFlipBuffers(options, 0);
    }

    return 0;
}

