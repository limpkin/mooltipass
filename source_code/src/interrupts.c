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
 /*!	\file 	interrupts.c
 *		\brief	Interrupts
 */
#include <avr/interrupt.h>
#include "interrupts.h"
#include "smartcard.h"
#include "pwm.h"

// Number of milliseconds since power up
static volatile uint32_t msecTicks = 0;


/*!	\fn		ISR(TIMER1_COMPA_vect)
*	\brief	Interrupt called every ms
*/
ISR(TIMER1_COMPA_vect)												// Match on TCNT1 & OCR1 Interrupt Handler, 1 ms interrupt
{
    msecTicks++;
    lightTimerTick();                                               // Light timer
	scanSMCDectect();												// Scan smart card detect
}


/*!	\fn		millis()
*	\brief	Return the number of milliseconds since power up
*	\returns the number of milliseconds since power up
*/
uint32_t millis()
{
    uint8_t reg = SREG;
    uint32_t ms;
    
    cli();
    ms = msecTicks;
    SREG = reg;        // restore original interrupt state (may already be disabled)

    return ms;
}


/*!	\fn 	initIRQ(void)
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
