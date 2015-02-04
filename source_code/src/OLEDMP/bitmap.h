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

#ifndef BITMAP_H_
#define BITMAP_H_

typedef struct __attribute__((packed)) bitmap_s
{
    uint16_t width;     //*< width of image in pixels
    uint8_t height;     //*< height of image in pixels
    uint8_t depth;      //*< Number of bits per pixel
    uint8_t flags;      //*< Flags defining data format
    uint16_t dataSize;  //*< number of words in data
    uint16_t data[];    //*< pointer to the image data
} bitmap_t;


#endif
