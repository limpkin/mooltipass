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
/*
 * interrupts.c
 *
 * Created: 11/01/2014 11:54:26
 *  Author: Mathieu Stephan
 */
#include <avr/interrupt.h>
#include "mooltipass.h"


/*!	\fn		ISR(TIMER1_COMPA_vect)
*	\brief	Interrupt called every ms
*/
ISR(TIMER1_COMPA_vect)												// Match on TCNT1 & OCR1 Interrupt Handler, 1 ms interrupt
{
	scanSMCDectect();										// Scan smart card detect
}

/*!	\fn 	init_interrupts(void)
*	\brief	Initialize the interrupts
*/
void initIRQ(void)
{
	/* Our 1ms interrupt to scan buttons */
	OCR1AH = 0x07;													// 1 msec interrupt (2000 - 1): 16M/8/2000 = 1kHz
	OCR1AL = 0xCF;													// 1 msec interrupt (2000 - 1): 16M/8/2000 = 1kHz
	TCCR1A = 0x00;		 											// Clear counter on match, clock divided by 8
	TCCR1B = (1 << WGM12) | (1 << CS11); 							// Clear counter on match, clock divided by 8
	TIMSK1 |= (1 << OCIE1A);										// Enable compare interrupt
}