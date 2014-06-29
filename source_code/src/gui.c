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
#include <util/delay.h>
#include <stdint.h>
#include "touch_higher_level_functions.h"
#include "node_mgmt.h"
#include "defines.h"
#include "oledmp.h"
#include "touch.h"
#include "pwm.h"
#include "gui.h"
#include "usb.h"

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


/*! \fn     guiTimerTick(void)
*   \brief  Function called every ms by interrupt
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

/*! \fn     activateLightTimer(void)
*   \brief  Activate light timer
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

/*! \fn     activateScreenTimer(void)
*   \brief  Activate screen timer
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

/*! \fn     activityDetectedRoutine(void)
*   \brief  What to do when user activity has been detected
*/
void activityDetectedRoutine(void)
{
    #ifdef HARDWARE_V1
        return;
    #endif
    
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
    #ifdef HARDWARE_V1
        return;
    #endif
    
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
        //oledOff();
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

/*! \fn     getTouchUiYesNoAnswer(void))
*   \brief  Use the capacitive interface to get a yes or no
*   \return Yew or No
*/
RET_TYPE getTouchUiYesNoAnswer(void)
{
    #ifdef HARDWARE_V1
        _delay_ms(2000);
        return RETURN_OK;
    #endif

    RET_TYPE touch_detect_result;
    
    // Wait for all presses to be released
    while(touchDetectionRoutine() & TOUCH_PRESS_MASK);
    
    // Wait for a touch press
    touch_detect_result = touchDetectionRoutine() & TOUCH_PRESS_MASK;
    while (!((touch_detect_result & RETURN_LEFT_PRESSED) || (touch_detect_result & RETURN_RIGHT_PRESSED)))
    {
        touch_detect_result = touchDetectionRoutine() & TOUCH_PRESS_MASK;
    }
    
    if (touch_detect_result & RETURN_LEFT_PRESSED)
    {
        return RETURN_NOK;
    } 
    else
    {
        return RETURN_OK;
    }
}

/*! \fn     getTouchUiQuarterPosition(void))
*   \brief  Use the capacitive interface to get quarter position
*   \return Number between 0 and 3
*/
uint8_t getTouchUiQuarterPosition(void)
{
    #ifdef HARDWARE_V1
        _delay_ms(2000);
        return 0;
    #endif

    RET_TYPE touch_detect_result;
    uint8_t temp_position;
    
    // Wait for all presses to be released
    while(touchDetectionRoutine() & TOUCH_PRESS_MASK);
    
    // Wait for a touch press
    touch_detect_result = touchDetectionRoutine() & TOUCH_PRESS_MASK;
    while (!(touch_detect_result & RETURN_WHEEL_PRESSED))
    {
        touch_detect_result = touchDetectionRoutine() & TOUCH_PRESS_MASK;
    }
    
    // Get position
    readDataFromTS(AT42QT2120_ADDR, REG_AT42QT_SLIDER_POS, &temp_position);
    
    if (temp_position < 0x3F)
    {
        return 1;
    }
    else if (temp_position < 0x7F)
    {
        return 3;
    }
    else if (temp_position < 0xBF)
    {
        return 2;
    }
    else
    {
        return 0;
    } 
}

/*! \fn     informGuiOfCurrentContext(char* context)
*   \param  context String of the context
*   \brief  Inform the GUI of the current context
*/
void informGuiOfCurrentContext(char* context)
{
    return;
    // Display current context
    oledClear();
    oledSetXY(0, 20);
    printf_P(PSTR("Current context : "));
    printf(context);
    oledFlipBuffers(OLED_SCROLL_UP, 1);
}

/*! \fn     guiAskForDomainAddApproval(char* name)
*   \param  context String of the context
*   \brief  Ask for user approval to add a domain
*/
RET_TYPE guiAskForDomainAddApproval(char* name)
{    
    RET_TYPE return_value;
    
    // Switch on lights
    activityDetectedRoutine();
    
    // Draw asking bitmap & wait for user input
    oledBitmapDrawFlash(0, 0, 2, OLED_SCROLL_UP);
    oledWriteActiveBuffer();
    oledSetXY(85, 25);
    printf(name);
    oledWriteInactiveBuffer();
    return_value = getTouchUiYesNoAnswer();
    
    // Draw default bitmap
    oledBitmapDrawFlash(0, 0, 0, OLED_SCROLL_UP);
    
    return return_value;
}

/*! \fn     guiAskForLoginAddApproval(char* name)
*   \param  name    Login that needs to be added
*   \param  service Name of the current service
*   \brief  Ask for user approval to add a login
*/
RET_TYPE guiAskForLoginAddApproval(char* name, char* service)
{
    RET_TYPE return_value;
    
    // Switch on lights
    activityDetectedRoutine();
    
    // Draw asking bitmap & wait for user input
    oledBitmapDrawFlash(0, 0, 3, OLED_SCROLL_UP);
    oledWriteActiveBuffer();
    oledSetXY(85, 25);
    printf(name);
    oledSetXY(85, 45);
    printf("on ");    
    printf(service);
    oledWriteInactiveBuffer();
    return_value = getTouchUiYesNoAnswer();
    
    // Draw default bitmap
    oledBitmapDrawFlash(0, 0, 0, OLED_SCROLL_UP);
    
    return return_value;
}

/*! \fn     guiAskForPasswordSet(char* name)
*   \brief  Ask for user approval to set a password
*   \param  name        The login
*   \param  password    The new password
*   \param  service     Service Name
*/
RET_TYPE guiAskForPasswordSet(char* name, char* password, char* service)
{
    RET_TYPE return_value;
    
    // Switch on lights
    activityDetectedRoutine();
    
    // Draw asking bitmap & wait for user input
    oledBitmapDrawFlash(0, 0, 4, OLED_SCROLL_UP);
    oledWriteActiveBuffer();
    oledSetXY(85, 25);
    printf(name);
    oledSetXY(85, 45);
    printf("on ");
    printf(service);
    oledWriteInactiveBuffer();
    return_value = getTouchUiYesNoAnswer();
    
    // Draw default bitmap
    oledBitmapDrawFlash(0, 0, 0, OLED_SCROLL_UP);
    
    return return_value;
}

/*! \fn     guiAskForLoginSelect(mgmtHandle* h, pNode* p, cNode* c, uint16_t parentNodeAddress)
*   \brief  Ask for user login selection / approval
*   \param  h                   Pointer to management handle
*   \param  p                   Pointer to a parent node
*   \param  c                   Pointer to a child node
*   \param  parentNodeAddress   Address of the parent node
*   \return Valid child node address or 0 otherwise
*/
uint16_t guiAskForLoginSelect(mgmtHandle* h, pNode* p, cNode* c, uint16_t parentNodeAddress)
{
    uint16_t temp_address;
    uint16_t addresses[4];
    uint8_t i = 0;
    uint8_t j;
    
    // Switch on lights
    activityDetectedRoutine();
    
    // Draw asking bitmap
    oledBitmapDrawFlash(0, 0, 1, OLED_SCROLL_UP);
    oledWriteActiveBuffer();
    
    // Read the parent node
    if (readParentNode(h, p, parentNodeAddress) != RETURN_OK)
    {
        return NODE_ADDR_NULL;
    }
    
    // Read child address
    temp_address = p->nextChildAddress;
    
    // Check if there are stored credentials
    if (temp_address == NODE_ADDR_NULL)
    {
        return NODE_ADDR_NULL;
    }
    
    // Write domain name on screen
    oledSetXY(95, 30);
    printf((char*)p->service);
    
    // List logins on screen
    while ((temp_address != NODE_ADDR_NULL) && (i != 4))
    {
        if (i == 0)
        {
            oledSetXY(16, 0);
        }
        else if (i == 1)
        {
            oledSetXY(184, 0);
        }
        else if (i == 2)
        {
            oledSetXY(16, 54);
        }
        else
        {
            oledSetXY(184, 54);
        }
        
        // Read child node to get login
        if (readChildNode(h, c, temp_address) != RETURN_OK)
        {
            return NODE_ADDR_NULL;
        }
        
        // Print login on screen
        addresses[i] = temp_address;
        printf((char*)c->login);
        
        // Fetch next address
        temp_address = c->nextChildAddress;
        i++;
    }        
    
    // Get touched quarter and check its validity
    j = getTouchUiQuarterPosition();
    if (j >= i)
    {
        j = 0;
    }
    temp_address = addresses[j];
    
    // Draw default bitmap
    oledWriteInactiveBuffer();
    oledBitmapDrawFlash(0, 0, 0, OLED_SCROLL_UP);
    
    return temp_address;
}
