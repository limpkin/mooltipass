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
/*!  \file     gui_mini_basic_functions.c
*    \brief    General user interface - basic functions
*    Created:  22/6/2014
*    Author:   Mathieu Stephan
*/
#include "touch_higher_level_functions.h"
#include "gui_screen_functions.h"
#include "timer_manager.h"
#include "logic_eeprom.h"
#include "mini_inputs.h"
#include "oledmini.h"
#include "defines.h"
#include "delays.h"
#include "anim.h"
#include "pwm.h"
#include "gui.h"
#ifdef MINI_VERSION

// Screen saver on bool
uint8_t screenSaverOn = FALSE;


/*! \fn     isScreenSaverOn(void)
*   \brief  Returns screen saver bool
*   \return screen saver bool
*/
uint8_t isScreenSaverOn(void)
{
    return screenSaverOn;
}

/*! \fn     activityDetectedRoutine(void)
*   \brief  What to do when user activity has been detected
*/
void activityDetectedRoutine(void)
{    
    // Activate timers for automatic switch off & user interaction timeout
    activateTimer(TIMER_SCREEN, SCREEN_TIMER_DEL);
    activateTimer(SLOW_TIMER_LOCKOUT, getMooltipassParameterInEeprom(LOCK_TIMEOUT_PARAM));
    activateTimer(TIMER_USERINT, ((uint16_t)controlEepromParameter(getMooltipassParameterInEeprom(USER_INTER_TIMEOUT_PARAM), MIN_USER_INTER_DEL/1000, MAX_USER_INTER_DEL/1000)) << 10);
    
    // If the screen was off, turn it on!
    if (miniOledIsScreenOn() == FALSE)
    {
        miniOledIsScreenOn();
        screenComingOnDelay();
    }
    
    // If we are in screen saver mode, exit it!
    if (screenSaverOn == TRUE)
    {
        screenSaverOn = FALSE;
    }
}

/*! \fn     getYesNoAnswerInput(uint8_t blocking)
*   \brief  Use the input interface to get user input
*   \param  blocking    Boolean to know if we should wait for input or timeout
*   \note   In case of a non blocking call, caller must call activityDetectedRoutine() & miniWheelClearDetections()
*   \return see mini_input_yes_no_ret_t
*/
RET_TYPE getYesNoAnswerInput(uint8_t blocking)
{
    #if defined(ALWAYS_ACCEPT_REQUESTS)
        return MINI_INPUT_RET_YES;
    #endif

    uint8_t incomingData[RAWHID_TX_SIZE];
    RET_TYPE detect_result;
    
    if (blocking == TRUE)
    {
        // Switch on lights
        activityDetectedRoutine();
        
        // Clear possible remaining detection
        miniWheelClearDetections();
    }
    
    // Wait for a touch press
    do
    {
        // User interaction timeout or smartcard removed
        if ((hasTimerExpired(TIMER_USERINT, TRUE) == TIMER_EXPIRED) || (isSmartCardAbsent() == RETURN_OK))
        {
            return MINI_INPUT_RET_TIMEOUT;
        }
        
        // Read usb comms as the plugin could ask to cancel the request
        if ((getMooltipassParameterInEeprom(USER_REQ_CANCEL_PARAM) != FALSE) && (usbRawHidRecv(incomingData) == RETURN_COM_TRANSF_OK))
        {
            if (incomingData[HID_TYPE_FIELD] == CMD_CANCEL_REQUEST)
            {
                // Request canceled
                return MINI_INPUT_RET_TIMEOUT;
            }
            else
            {
                // Another packet (that shouldn't be sent!), ask to retry later...
                usbSendMessage(CMD_PLEASE_RETRY, 0, incomingData);
            }
        }
        
        // Check if something has been pressed
        detect_result = miniGetWheelAction(FALSE, TRUE);
        if (detect_result == WHEEL_ACTION_SHORT_CLICK)
        {
            return MINI_INPUT_RET_YES;
        }
        else if (detect_result == WHEEL_ACTION_LONG_CLICK)
        {
            return MINI_INPUT_RET_BACK;
        }
    }
    while(blocking != FALSE);
    
    // Return MINI_INPUT_RET_NONE if nothing was pressed and no timeout occurred
    return MINI_INPUT_RET_NONE;
}    

/*! \fn     guiMainLoop(void)
*   \brief  Main user interface loop
*/
void guiMainLoop(void)
{
    RET_TYPE input_interface_result;
    uint8_t screenSaverOnCopy;
    uint8_t isScreenOnCopy;
    
    // Make a copy of the screen on & screensaver on bools
    screenSaverOnCopy = screenSaverOn;
    isScreenOnCopy = miniOledIsScreenOn();
    
    /* Get possible wheel action */
    input_interface_result = miniGetWheelAction(FALSE, FALSE);

    #if defined(HARDWARE_MINI_CLICK_V2)
    if ((miniOledIsScreenOn() == FALSE) && (scanAndGetDoubleZTap(FALSE) == RETURN_OK))
    {
        // knock detecting algo to wakup the device
        activityDetectedRoutine();
    }
    #endif
    
    // No activity, switch off screen
    if (hasTimerExpired(TIMER_SCREEN, TRUE) == TIMER_EXPIRED)
    {
        #ifndef MINI_DEMO_VIDEO
            guiDisplayGoingToSleep();
            userViewDelay();
            if (getMooltipassParameterInEeprom(SCREENSAVER_PARAM) != FALSE)
            {
                screenSaverOn = TRUE;
            }
            else
            {
                miniOledOff();
                guiGetBackToCurrentScreen();
            }
        #else
            miniOledBitmapDrawFlash(0, 0, BITMAP_MOOLTIPASS, OLED_SCROLL_UP);
        #endif
    }

    // If there was some activity and we are showing the screen saver
    if ((input_interface_result != WHEEL_ACTION_NONE) && (screenSaverOnCopy == TRUE))
    {
        guiGetBackToCurrentScreen();
    }

    if ((input_interface_result != WHEEL_ACTION_NONE) && (((isScreenOnCopy != FALSE) && (screenSaverOnCopy == FALSE)) || (getCurrentScreen() == SCREEN_DEFAULT_INSERTED_LCK)))
    {
        guiScreenLoop(input_interface_result);
    }
}
#endif