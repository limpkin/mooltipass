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

#ifndef FONTS_H_
#define FONTS_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

// Font selection
#undef FONT_CHECKBOOK_12
//#define FONT_DEFAULT font_CHECKBOOK_12
#undef FONT_CHECKBOOK_14
//#define FONT_DEFAULT font_CHECKBOOK_14
#undef FONT_PROFONT_10
//#define FONT_DEFAULT font_PROFONT_10
#define FONT_MONO_5x7
#define FONT_DEFAULT font_MONO_5x7
#undef FONT_ROBOTO_MEDIUM_14
//#define FONT_DEFAULT font_ROBOTO_MEDIUM_14

typedef struct
{
    uint8_t width;          // Width of glyph data in pixels
    uint8_t xrect;          // x width of rectangle
    uint8_t yrect;          // y height of rectangle
    int8_t xoffset;         // x offset of glyph in rectangle
    int8_t yoffset;         // y offset of glyph in rectangle
    const uint8_t *glyph;   // glyph pixel data
} glyph_t;

typedef struct
{
    uint8_t height;         //*< height of font
    uint8_t fixedWidth;     //*< width of font, 0 = proportional font
    uint8_t depth;          //*< Number of bits per pixel
    const uint8_t *map;     //*< ASCII to font map
    union
    {
        const glyph_t *glyphs;   //*< variable width font data
        const uint8_t *bitmaps;  //*< fixed width font data
    } fontData;
} font_t;

typedef enum 
{
    #ifdef FONT_CHECKBOOK_12
        font_CHECKBOOK_12,
    #endif
    #ifdef FONT_CHECKBOOK_14
        font_CHECKBOOK_14,
    #endif
    #ifdef FONT_PROFONT_10
        font_PROFONT_10,
    #endif
    #ifdef FONT_PROFONT_10_100DPI
        font_PROFONT_10_100DPI,
    #endif
    #ifdef FONT_MONO_5x7
        font_MONO_5x7,
    #endif
    #ifdef FONT_ROBOTO_THIN_14
        font_ROBOTO_THIN_14,
    #endif
      #ifdef FONT_ROBOTO_MEDIUM_14
        font_ROBOTO_MEDIUM_14,
    #endif
    #ifdef FONT_BABYBLUE_MEDIUM_12
        font_BABYBLUE_MEDIUM_12,
    #endif
} font_e;

extern font_t fontsHQ[];

#endif
