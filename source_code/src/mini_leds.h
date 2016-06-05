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
/*! \file   mini_leds.c
 *  \brief  LEDs functions
 *  Created: 03/06/2016
 *  Copyright [2016] [Mathieu Stephan]
 */


#ifndef MINI_LEDS_H_
#define MINI_LEDS_H_

void miniLedsSetAnimation(uint8_t animation);
void miniSetLedStates(uint8_t leds);
void miniLedsAnimationTick(void);
void miniInitLeds(void);


#endif /* MINI_LEDS_H_ */