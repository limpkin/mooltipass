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

font_t fontsHQ[] = {
#ifdef FONT_CHECKBOOK_12
    { checkbook_12, checkbook_12_asciimap, CHECKBOOK_12_HEIGHT },
#endif
#ifdef FONT_CHECKBOOK_14
    { checkbook_14, checkbook_14_asciimap, CHECKBOOK_14_HEIGHT },
#endif
#ifdef FONT_PROFONT_10_100DPI
    { profont_10_100, profont_10_100_asciimap, PROFONT_10_100_HEIGHT },
#endif
#ifdef FONT_PROFONT_10_72DPI
    { profont_10_72, profont_10_72_asciimap, PROFONT_10_72_HEIGHT },
#endif
};

