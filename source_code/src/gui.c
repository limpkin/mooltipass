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
/*!  \file     gui.c
*    \brief    General user interface
*    Created:  22/6/2014
*    Author:   Mathieu Stephan
*/
#include <util/atomic.h>
#include <stdint.h>
#include "touch_higher_level_functions.h"
#include "defines.h"
#include "oledmp.h"
#include "touch.h"
#include "pwm.h"
#include "gui.h"

// Screen on timer
volatile uint16_t screenTimer = SCREEN_TIMER_DEL;
// Flag to switch off the lights
volatile uint8_t lightsTimerOffFlag = FALSE;
// Flag to switch off the screen
volatile uint8_t screenTimerOffFlag = FALSE;
// Our light timer for the top PCB LEDs
volatile uint16_t light_timer = 0;
// Bool to know if lights are on
uint8_t areLightsOn = FALSE;
// Bool to know if screen is on
uint8_t isScreenOn = TRUE;


/*!	\fn		guiTimerTick(void)
*	\brief	Function called every ms by interrupt
*/
void guiTimerTick(void)
{
    if (light_timer != 0)
    {
        if (light_timer-- == 1)
        {
            lightsTimerOffFlag = TRUE;
        }
    }
    if (screenTimer != 0)
    {
        if (screenTimer-- == 1)
        {
           screenTimerOffFlag = TRUE;
        }
    }
}

/*!	\fn		activateLightTimer(void)
*	\brief	Activate light timer
*/
void activateLightTimer(void)
{
    if (light_timer != LIGHT_TIMER_DEL)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            light_timer = LIGHT_TIMER_DEL;
        }
    }
}

/*!	\fn		activateScreenTimer(void)
*	\brief	Activate screen timer
*/
void activateScreenTimer(void)
{
    if (screenTimer != SCREEN_TIMER_DEL)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            screenTimer = SCREEN_TIMER_DEL;
        }
    }
}

/*!	\fn		activityDetectedRoutine(void)
*	\brief	What to do when user activity has been detected
*/
void activityDetectedRoutine(void)
{
    activateLightTimer();
    activateScreenTimer();
    
    // If the lights were off, turn them on!
    if (areLightsOn == FALSE)
    {
        setPwmDc(MAX_PWM_VAL);
        activateGuardKey();
        areLightsOn = TRUE;
    }
    
    // If the screen was off, turn it on!
    if (isScreenOn == FALSE)
    {
        oledOn();
        isScreenOn = TRUE;
    }    
}

void guiMainLoop(void)
{    
    RET_TYPE touch_detect_result = touchDetectionRoutine();
    
    // No activity, switch off LEDs and activate prox detection
    if (lightsTimerOffFlag == TRUE)
    {
        setPwmDc(0x0000);
        areLightsOn = FALSE;
        activateProxDetection();
        lightsTimerOffFlag = FALSE;
    }
    
    // No activity, switch off screen
    if (screenTimerOffFlag == TRUE)
    {
        oledOff();
        isScreenOn = FALSE;
        screenTimerOffFlag = FALSE;
    }
    
    // Touch interface
    if (touch_detect_result & TOUCH_PRESS_MASK)
    {
        activityDetectedRoutine();
        
        // If left button is pressed
        if (touch_detect_result & RETURN_LEFT_PRESSED)
        {
            #ifdef TOUCH_DEBUG_OUTPUT_USB
                usbPutstr_P(PSTR("LEFT touched\r\n"));
            #endif
        }
        
        // If right button is pressed
        if (touch_detect_result & RETURN_RIGHT_PRESSED)
        {
            #ifdef TOUCH_DEBUG_OUTPUT_USB
                usbPutstr_P(PSTR("RIGHT touched\r\n"));
            #endif
        }
        
        // If wheel is pressed
        if (touch_detect_result & RETURN_WHEEL_PRESSED)
        {
        }
    }
}