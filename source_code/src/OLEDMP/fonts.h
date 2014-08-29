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
#define FONT_PROFONT_10     7
#define FONT_CHECKBOOK_14   8
#define FONT_CHECKBOOK_24  15
#define FONT_DEFAULT FONT_PROFONT_10

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


/*
 * Fonts in SPI Flash
 * font header with height, width, depth, and map.
 * bitmaps or glyphs
 *
 * Proportional width font:
 *     flashFont_t, fixedWidth = 0
 *     count * glyph_t
 *     count * variable size glyph data indexed from glyph_t
 *
 * fixed width font:
 *     flashFont_t,	fixedWidth = width of font
 *     count * fixed font data
 */


typedef struct {
    uint8_t height;         //*< height of font
    uint8_t fixedWidth;     //*< width of font, 0 = proportional font
    uint8_t depth;          //*< Number of bits per pixel
    uint8_t count;          //*< number of characters
} fontHeader_t;

typedef struct
{
    fontHeader_t header;
    const uint8_t map[256]; //*< ASCII to font map
    glyph_t glyph[];
} flashFont_t;

#endif
