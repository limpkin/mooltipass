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

/*!	\file 	low_level_utils.c
*	\brief	Low level MCU helper functions
*/

#include "low_level_utils.h"

/**
 * Set pin input or output mode with optional pullup for inputs
 * @param port - the port the pin is on
 * @param pin  - the pin mask for the pin (i.e. 1 << pin_number)
 * @param mode - INPUT or OUTPUT
 * @param pullup - pullup enabled for input if true
 */
void pinMode(uint8_t volatile *port, const uint8_t pin, uint8_t mode, bool pullup)
{
    uint8_t volatile *dataDir = port-1;

    if (mode == INPUT) {
	*dataDir &= ~pin;
	if (pullup) {
	    *port |= pin;
	} else {
	    *port &= ~pin;
	}
    } else if (mode == OUTPUT) {
	*dataDir |= pin;
    }
}


