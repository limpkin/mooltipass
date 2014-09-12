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

// Font selection
#undef FONT_CHECKBOOK_12
#define FONT_CHECKBOOK_14
#define FONT_PROFONT_10_100DPI
#define FONT_PROFONT_10_72DPI
#define FONT_DEFAULT font_PROFONT_10_100DPI
//#define FONT_DEFAULT font_PROFONT_10_100DPI

typedef struct {
    uint8_t width;		// Width of glyph data in pixels
    uint8_t xrect;		// x width of rectangle
    uint8_t yrect;		// y height of rectangle
    int8_t xoffset;		// x offset of glyph in rectangle
    int8_t yoffset;		// y offset of glyph in rectangle
    const uint8_t *glyph;	// glyph pixel data
} glyph_t;

typedef struct {
    const glyph_t *glyphs;
    const uint8_t *map;
    uint8_t height;
} font_t;

#ifdef FONT_CHECKBOOK_12
#include "font_checkbook_12.h"
#endif
#ifdef FONT_CHECKBOOK_14
#include "font_checkbook_14.h"
#endif
#ifdef FONT_PROFONT_10_100DPI
#include "font_profont_10_100dpi.h"
#endif
#ifdef FONT_PROFONT_10_72DPI
#include "font_profont_10_72dpi.h"
#endif

typedef enum {
#ifdef FONT_CHECKBOOK_12
    font_CHECKBOOK_12,
#endif
#ifdef FONT_CHECKBOOK_14
    font_CHECKBOOK_14,
#endif
#ifdef FONT_PROFONT_10_100DPI
    font_PROFONT_10_100DPI,
#endif
#ifdef FONT_PROFONT_10_72DPI
    font_PROFONT_10_72DPI,
#endif
} font_e;

extern font_t fontsHQ[];

#endif
