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
/*! \file   pwm.c
 *  \brief  PWM functions
 *  Copyright [2014] [Mathieu Stephan]
 */
#include <avr/io.h>
#include "mooltipass.h"
#include "defines.h"
#include "pwm.h"

// Our light timer
volatile uint16_t light_timer = 0;


/*!	\fn		initPwm()
*	\brief	Initialize PWM
*/
void initPwm(void)
{
    #ifndef HARDWARE_V1
        TC4H = 0x03;                                            // Set TOP to max value (0x03FF);
        OCR4C = 0xFF;                                           // Set TOP to max value (0x03FF);
        TC4H = 0xFF;                                            // Output off by default
        OCR4A = 0xFF;                                           // Output off by default
        DDR_LED_PWM |= (1 << PORTID_LED_PWM);                   // Enable port, 0 by default
        PORT_LED_PWM &= ~(1 << PORTID_LED_PWM);                 // Enable port, 0 by default
        TCCR4A = (1 << COM4A1) | (1 << COM4A0) |  (1 << PWM4A);	// Enhanced fast PWM mode, set OC4A on Compare Match, clear OC4A at BOTTOM
        TCCR4E = (1 << ENHC4);                                  // Enhanced (11 bits) PWM mode
        TCCR4B = (1 << CS40);						 			// No prescaling
    #endif
}

/*!	\fn		setPwmDc(uint16_t pwm_value)
*	\brief	Set PWM duty cycle
*	\param	pwm_value	The duty cycle
*/
void setPwmDc(uint16_t pwm_value)
{
    TC4H = ~(pwm_value >> 8);
	OCR4A = ~pwm_value;
}

/*!	\fn		lightTimerTick(void)
*	\brief	Function called every ms
*/
void lightTimerTick(void)
{
    if (light_timer != 0)
    {
        if (light_timer-- == 1)
        {
            setLightsOutFlag();
        }
    }
}

/*!	\fn		activateLightTimer(void)
*	\brief	Activate light timer
*/
void activateLightTimer(void)
{
    light_timer = LIGHT_TIMER_DEL;
}