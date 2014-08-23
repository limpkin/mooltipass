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
/*!  \file     gui_basic_functions.c
*    \brief    General user interface - basic functions
*    Created:  22/6/2014
*    Author:   Mathieu Stephan
*/
#include "touch_higher_level_functions.h"
#include "gui_screen_functions.h"
#include "timer_manager.h"
#include "defines.h"
#include "oledmp.h"
#include "delays.h"
#include "pwm.h"
#include "gui.h"

// Touch logic: is touch wheel pressed in our algo
uint8_t touch_logic_press = FALSE;
// Touch logic: reference position of first touch
uint8_t touch_logic_ref_position;
// Bool to know if lights are on
uint8_t areLightsOn = FALSE;
// Bool to know if screen is on
uint8_t isScreenOn = TRUE;
// Current led mask for the PCB
uint8_t currentLedMask = 0;


/*! \fn     activityDetectedRoutine(void)
*   \brief  What to do when user activity has been detected
*/
void activityDetectedRoutine(void)
{
    #ifdef HARDWARE_V1
        return;
    #endif
    
    // Activate timers for automatic switch off
    activateTimer(TIMER_LIGHT, LIGHT_TIMER_DEL);
    activateTimer(TIMER_SCREEN, SCREEN_TIMER_DEL);
    
    // If the screen was off, turn it on!
    if (isScreenOn == FALSE)
    {
        oledOn();
        isScreenOn = TRUE;
        screenComingOnDelay();
    }
    
    // If the lights were off, turn them on!
    if (areLightsOn == FALSE)
    {
        setPwmDc(MAX_PWM_VAL);
        activateGuardKey();
        areLightsOn = TRUE;
    }
}

/*! \fn     getTouchedPositionAnswer(uint8_t led_mask)
*   \brief  Use the capacitive interface to get quarter position
*   \param  led_mask    Led mask for the touchdetection routine
*   \return Number between 0 and 5 for valid pos, -1 otherwise
*/
int8_t getTouchedPositionAnswer(uint8_t led_mask)
{
    #ifdef HARDWARE_V1
        _delay_ms(2000);
    #endif
    #if defined(ALWAYS_ACCEPT_REQUESTS) || defined(HARDWARE_V1)
        // First quarter is discarded, it means we want yes or no!
        if (led_mask & LED_MASK_WHEEL_TLEFT)
        {
            return TOUCHPOS_RIGHT;
        }
        else
        {
            return TOUCHPOS_WHEEL_TLEFT;
        }
    #endif

    RET_TYPE touch_detect_result;
    
    // Switch on lights
    activityDetectedRoutine();
    
    // Clear possible remaining detection
    touchDetectionRoutine(led_mask);
    touchClearCurrentDetections();
    
    // Wait for a touch press
    activateTimer(TIMER_USERINT, USER_INTER_DEL);
    do
    {
        // User interaction timeout or smartcard removed
        if ((hasTimerExpired(TIMER_USERINT, TRUE) == TIMER_EXPIRED) || (isSmartCardAbsent() == RETURN_OK))
        {
            return -1;
        }
        touch_detect_result = touchDetectionRoutine(led_mask) & TOUCH_PRESS_MASK;
    }
    while (!touch_detect_result);
    
    // Did the user press one of the two touch buttons?
    if (touch_detect_result & RETURN_LEFT_PRESSED)
    {
        return TOUCHPOS_LEFT;
    }
    else if (touch_detect_result & RETURN_RIGHT_PRESSED)
    {
        return TOUCHPOS_RIGHT;
    }
    else
    {
        return (int8_t)getWheelTouchDetectionQuarter();
    }
}

/*! \fn     touchWheelIntefaceLogic(void)
*   \brief  Use the wheel and get -1 or +1 ticks
*   \param  touch_detection_result  Result from the touchdetectionroutine
*   \return -1 or +1 ticks
*/
int8_t touchWheelIntefaceLogic(RET_TYPE touch_detection_result)
{
    uint8_t temp_position;
    uint8_t up_lower_slot;
    uint8_t up_higher_slot;
    uint8_t down_lower_slot;
    uint8_t down_higher_slot;
    
    if (touch_detection_result & RETURN_WHEEL_PRESSED)
    {
        // Get touched position
        temp_position = getLastRawWheelPosition();
        
        // If it is the first touch, store reference position
        if (touch_logic_press == FALSE)
        {
            touch_logic_ref_position = temp_position;
            touch_logic_press = TRUE;
        }
        
        up_lower_slot = touch_logic_ref_position + WHEEL_TICK_INCREMENT;
        up_higher_slot = touch_logic_ref_position + 3*WHEEL_TICK_INCREMENT;
        down_higher_slot = touch_logic_ref_position - WHEEL_TICK_INCREMENT;
        down_lower_slot = touch_logic_ref_position - 3*WHEEL_TICK_INCREMENT;
        
        // Detect wrap arounds
        if (up_lower_slot > up_higher_slot)
        {
            if ((temp_position > up_lower_slot) || (temp_position < up_higher_slot))
            {
                touch_logic_ref_position += WHEEL_TICK_INCREMENT;
                return 1;
            }
        }
        else
        {
            if ((temp_position > up_lower_slot) && (temp_position < up_higher_slot))
            {
                touch_logic_ref_position += WHEEL_TICK_INCREMENT;
                return 1;
            }
        }
        
        // Same
        if (down_lower_slot > down_higher_slot)
        {
            if ((temp_position < down_higher_slot) || (temp_position > down_lower_slot))
            {
                touch_logic_ref_position -= WHEEL_TICK_INCREMENT;
                return -1;
            }
        }
        else
        {
            if ((temp_position < down_higher_slot) && (temp_position > down_lower_slot))
            {
                touch_logic_ref_position -= WHEEL_TICK_INCREMENT;
                return -1;
            }
        }
    }
    else if(touch_detection_result & RETURN_WHEEL_RELEASED)
    {
        touch_logic_press = FALSE;
    }
    
    return 0;
}

/*! \fn     guiMainLoop(void)
*   \brief  Main user interface loop
*/
void guiMainLoop(void)
{
    RET_TYPE touch_detect_result;
    uint8_t isScreenOnCopy;
    
    #ifdef HARDWARE_V1
        return;
    #endif
    
    // Set led mask depending on our current screen
    switch(getCurrentScreen())
    {
        case SCREEN_DEFAULT_NINSERTED :         currentLedMask = LED_MASK_LEFT|LED_MASK_RIGHT|LED_MASK_WHEEL; break;
        case SCREEN_DEFAULT_INSERTED_LCK :      currentLedMask = LED_MASK_LEFT|LED_MASK_RIGHT|LED_MASK_WHEEL; break;
        case SCREEN_DEFAULT_INSERTED_NLCK :     currentLedMask = LED_MASK_LEFT|LED_MASK_RIGHT; break;
        case SCREEN_DEFAULT_INSERTED_INVALID :  currentLedMask = LED_MASK_LEFT|LED_MASK_RIGHT|LED_MASK_WHEEL; break;
        case SCREEN_SETTINGS :                  currentLedMask = LED_MASK_LEFT|LED_MASK_RIGHT; break;
        default: break;
    }
    
    // Make a copy of the screen on bool
    isScreenOnCopy = isScreenOn;
    
    // Launch touch detection routine to check for interactions
    touch_detect_result = touchDetectionRoutine(currentLedMask);
    
    // No activity, switch off LEDs and activate prox detection
    if (hasTimerExpired(TIMER_LIGHT, TRUE) == TIMER_EXPIRED)
    {
        setPwmDc(0x0000);
        areLightsOn = FALSE;
        activateProxDetection();
    }
    
    // No activity, switch off screen
    if (hasTimerExpired(TIMER_SCREEN, TRUE) == TIMER_EXPIRED)
    {
        #ifndef HARDWARE_V1
            oledOff();
        #endif
        isScreenOn = FALSE;
    }
    
    // If the screen just got turned on, don't call the guiScreenLoop() function
    if ((touch_detect_result & TOUCH_PRESS_MASK) && (isScreenOnCopy != FALSE))
    {
        guiScreenLoop(touch_detect_result);
    }   
}