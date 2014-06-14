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

#include "fonts.h"
#include "fontdefs.h"

/* { height, fixedWidth, depth, &map, &glyph/&bitmaps } */
font_t fontsHQ[] = 
{
    #ifdef FONT_CHECKBOOK_12
        { CHECKBOOK_12_HEIGHT,   0, 2, checkbook_12_asciimap, { .glyphs=checkbook_12} },
    #endif
    #ifdef FONT_CHECKBOOK_14
        { CHECKBOOK_14_HEIGHT,   0, 2, checkbook_14_asciimap, { .glyphs=checkbook_14} },
    #endif
    #ifdef FONT_PROFONT_10
        { PROFONT_10_HEIGHT+1, 0, 2, profont_10_asciimap, { .glyphs=profont_10} },
    #endif
    #ifdef FONT_PROFONT_10_100DPI
        { PROFONT_10_100_HEIGHT, 0, 2, profont_10_100_asciimap, { .glyphs=profont_10_100} },
    #endif
    #ifdef FONT_MONO_5x7
        { FONT_MONO_5x7_HEIGHT,  5, 1, font_mono_5x7_asciimap, { .bitmaps=(uint8_t *)font_mono_5x7} },
    #endif
    #ifdef FONT_ROBOTO_MEDIUM_14
        { ROBOTO_MEDIUM_14_HEIGHT,  0, 2, roboto_medium_14_asciimap, { .glyphs=roboto_medium_14} },
    #endif
};

