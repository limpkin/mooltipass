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
#include <string.h>
#include "smart_card_higher_level_functions.h"
#include "touch_higher_level_functions.h"
#include "timer_manager.h"
#include "userhandling.h"
#include "node_mgmt.h"
#include "smartcard.h"
#include "defines.h"
#include "oledmp.h"
#include "touch.h"
#include "anim.h"
#include "pwm.h"
#include "gui.h"
#include "usb.h"

// Our current screen
uint8_t currentScreen = SCREEN_DEFAULT_NINSERTED;
// Touch logic: is touch wheel pressed in our algo
uint8_t touch_logic_press = FALSE;
// Touch logic: reference position of first touch
uint8_t touch_logic_ref_position;
// Bool to know if lights are on
uint8_t areLightsOn = FALSE;
// Current led mask for the PCB
uint8_t currentLedMask = 0;
// Bool to know if screen is on
uint8_t isScreenOn = TRUE;



/*! \fn     activityDetectedRoutine(void)
*   \brief  What to do when user activity has been detected
*/
void activityDetectedRoutine(void)
{
    #ifdef HARDWARE_V1
        return;
    #endif
    
    activateTimer(TIMER_LIGHT, LIGHT_TIMER_DEL);
    activateTimer(TIMER_SCREEN, SCREEN_TIMER_DEL);
    
    // If the screen was off, turn it on!
    if (isScreenOn == FALSE)
    {
        oledOn();
        _delay_ms(130);
        isScreenOn = TRUE;
    }
    
    // If the lights were off, turn them on!
    if (areLightsOn == FALSE)
    {
        setPwmDc(MAX_PWM_VAL);
        activateGuardKey();
        areLightsOn = TRUE;
    }   
}

/*! \fn     guiMainLoop(void)
*   \brief  Main user interface loop
*/
void guiMainLoop(void)
{   
    #ifdef HARDWARE_V1
        return;
    #endif
    
    // Set led mask depending on current screen
    switch(currentScreen)
    {
        case SCREEN_DEFAULT_NINSERTED :         currentLedMask = LED_MASK_LEFT|LED_MASK_RIGHT|LED_MASK_WHEEL; break;
        case SCREEN_DEFAULT_INSERTED_LCK :      currentLedMask = LED_MASK_LEFT|LED_MASK_RIGHT|LED_MASK_WHEEL; break;
        case SCREEN_DEFAULT_INSERTED_NLCK :     currentLedMask = LED_MASK_LEFT|LED_MASK_RIGHT; break;
        case SCREEN_DEFAULT_INSERTED_INVALID :  currentLedMask = LED_MASK_LEFT|LED_MASK_RIGHT|LED_MASK_WHEEL; break;
        case SCREEN_SETTINGS :                  currentLedMask = LED_MASK_LEFT|LED_MASK_RIGHT; break;
        default: break;
    }
    
    RET_TYPE touch_detect_result = touchDetectionRoutine(currentLedMask);
    
    // No activity, switch off LEDs and activate prox detection    
    if (isTimerFlagPresent(TIMER_LIGHT, TRUE) == FLAG_PRESENT)
    {
        setPwmDc(0x0000);
        areLightsOn = FALSE;
        activateProxDetection();
    }
    
    // No activity, switch off screen
    if (isTimerFlagPresent(TIMER_SCREEN, TRUE) == FLAG_PRESENT)
    {
        #ifndef HARDWARE_V1
            oledOff();
        #endif
        isScreenOn = FALSE;
    }
    
    // Touch interface
    if (touch_detect_result & TOUCH_PRESS_MASK)
    {
        touch_detect_result &= ~RETURN_PROX_DETECTION;
        
        if (currentScreen == SCREEN_DEFAULT_NINSERTED)
        {
            // No smartcard inserted, ask the user to insert one
            guiDisplayInsertSmartCardScreenAndWait();
        }
        else if (currentScreen == SCREEN_DEFAULT_INSERTED_LCK)
        {
            uint16_t lapinou;
            
            // Locked screen, ask the user to enter his pin code
            if(guiGetPinFromUser(&lapinou) == RETURN_OK)
            {
                // User approved his pin
                currentScreen = SCREEN_DEFAULT_INSERTED_NLCK;
                
            }
            else
            {
                currentScreen = SCREEN_DEFAULT_INSERTED_LCK;
                
            }
            guiGetBackToCurrentScreen();
        }
        else if ((currentScreen == SCREEN_DEFAULT_INSERTED_NLCK) && (touch_detect_result & RETURN_WHEEL_PRESSED))
        {
            // Unlocked screen
            switch(getWheelTouchDetectionQuarter())
            {
                case TOUCHPOS_WHEEL_BRIGHT :
                {
                    // User wants to lock his mooltipass
                    currentScreen = SCREEN_DEFAULT_INSERTED_LCK;
                    break;
                }
                case TOUCHPOS_WHEEL_TRIGHT :
                {
                    // User wants to go to the settings menu
                    currentScreen = SCREEN_SETTINGS;
                    break;
                }
                default : break;
            }
            guiGetBackToCurrentScreen();
        }
        else if ((currentScreen == SCREEN_SETTINGS) && (touch_detect_result & RETURN_WHEEL_PRESSED))
        {
            // Unlocked screen
            switch(getWheelTouchDetectionQuarter())
            {
                case TOUCHPOS_WHEEL_TLEFT :
                {
                    // User wants to go to the settings menu
                    currentScreen = SCREEN_DEFAULT_INSERTED_NLCK;
                    break;
                }
                default : break;
            }
            guiGetBackToCurrentScreen();
        }
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
        return TOUCHPOS_WHEEL_TLEFT;
    #endif
    #ifdef ALWAYS_ACCEPT_REQUESTS
        return TOUCHPOS_WHEEL_TLEFT;
    #endif

    RET_TYPE touch_detect_result;
    
    // Switch on lights
    activityDetectedRoutine();
    
    // Wait for all presses to be released
    while(touchDetectionRoutine(led_mask) & TOUCH_PRESS_MASK);
    
    // Wait for a touch press
    activateTimer(TIMER_USERINT, USER_INTER_DEL);
    do 
    {
        // User interaction timeout
        if (isTimerFlagPresent(TIMER_USERINT, TRUE) == FLAG_PRESENT)
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

/*! \fn     guiDisplayPinOnPinEnteringScreen(uint8_t* current_pin, uint8_t selected_digit)
*   \brief  Overwrite the digits on the current pin entering screen
*   \param  current_pin     Array containing the pin
*   \param  selected_digit  Currently selected digit
*/
void guiDisplayPinOnPinEnteringScreen(uint8_t* current_pin, uint8_t selected_digit)
{    
    oledFillXY(80, 18, 92, 24, 0x00);
    for (uint8_t i = 0; i < 4; i++)
    {
        oledSetXY(84+22*i, 20);
        if (i != selected_digit)
        {
            oledPutch('*');
        }
        else
        {
            if (current_pin[i] >= 0x0A)
            {
                oledPutch(current_pin[i]+'A'-0x0A);
            } 
            else
            {
                oledPutch(current_pin[i]+'0');
            }
        }
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

/*! \fn     guiGetPinFromUser(void)
*   \brief  Ask the user to enter a PIN
*   \param  pin_code    Pointer to where to store the pin code
*   \return If the user approved the request
*/
RET_TYPE guiGetPinFromUser(uint16_t* pin_code)
{
    RET_TYPE ret_val = RETURN_NOK;
    uint8_t selected_digit = 0;
    uint8_t finished = FALSE;
    uint8_t current_pin[4];
    RET_TYPE temp_rettype;
    int8_t temp_int8;
    
    // Set current pin to 0000
    memset((void*)current_pin, 0, 4);
    
    // Draw pin entering bitmap
    oledClear();
    oledBitmapDrawFlash(25, 0, BITMAP_LEFT, 0);
    oledBitmapDrawFlash(2, 26, BITMAP_CROSS, 0);
    oledBitmapDrawFlash(195, 0, BITMAP_RIGHT, 0);
    oledBitmapDrawFlash(80, 41, BITMAP_PIN_LINES, 0);
    oledBitmapDrawFlash(235, 23, BITMAP_RIGHT_ARROW, 0);
    oledFlipBuffers(0,0);
    oledSetFont(15);
    oledWriteActiveBuffer();
    
    // Display current pin on screen
    guiDisplayPinOnPinEnteringScreen(current_pin, selected_digit);
    
    // Wait for all presses to be released
    while(touchDetectionRoutine(0) & TOUCH_PRESS_MASK);
    
    // While the user hasn't entered his pin
    while(!finished)
    {
        // Detect key touches
        temp_rettype = touchDetectionRoutine(0);
        // Send it to the touch wheel interface logic
        temp_int8 = touchWheelIntefaceLogic(temp_rettype);
        
        // Position increment / decrement
        if (temp_int8 != 0)
        {
            if ((current_pin[selected_digit] == 0x0F) && (temp_int8 == 1))
            {
                current_pin[selected_digit] = 0xFF;
            }
            else if ((current_pin[selected_digit] == 0) && (temp_int8 == -1))
            {
                current_pin[selected_digit] = 0x10;
            }
            current_pin[selected_digit] += temp_int8;
            guiDisplayPinOnPinEnteringScreen(current_pin, selected_digit);
        }
        
        if (temp_rettype & RETURN_LEFT_PRESSED)
        {
            if (selected_digit == 1)
            {
                oledFillXY(0, 22, 22, 22, 0x00);
                oledBitmapDrawFlash(2, 26, BITMAP_CROSS, 0);
            }
            if (selected_digit > 0)
            {
                selected_digit--;
            }
            else
            {
                ret_val = RETURN_NOK;
                finished = TRUE;
            }
            guiDisplayPinOnPinEnteringScreen(current_pin, selected_digit);
            oledBitmapDrawFlash(235, 23, BITMAP_RIGHT_ARROW, 0);
        }
        else if (temp_rettype & RETURN_RIGHT_PRESSED)
        {
            if (selected_digit == 2)
            {
                oledFillXY(235, 23, 20, 25, 0x00);
                oledBitmapDrawFlash(236, 26, BITMAP_TICK, 0);
            }
            if (selected_digit < 3)
            {
                selected_digit++;
            }
            else
            {
                ret_val = RETURN_OK;
                finished = TRUE;
            }
            guiDisplayPinOnPinEnteringScreen(current_pin, selected_digit);
            oledBitmapDrawFlash(0, 23, BITMAP_LEFT_ARROW, 0);
        }
    }
    
    // Reset default font
    oledSetFont(FONT_DEFAULT);
    oledWriteInactiveBuffer();
    
    // Store the pin
    *pin_code = (uint16_t)(((uint16_t)(current_pin[0]) << 12) | (((uint16_t)current_pin[1]) << 8) | (current_pin[2] << 4) | current_pin[3]);
    
    // Return success status
    return ret_val;
}

/*! \fn     guiGetBackToCurrentScreen(void)
*   \brief  Get back to the current screen
*/
void guiGetBackToCurrentScreen(void)
{
    switch(currentScreen)
    {
        case SCREEN_DEFAULT_NINSERTED :
        {
            oledBitmapDrawFlash(0, 0, BITMAP_HAD, OLED_SCROLL_UP);
            break;
        }
        case SCREEN_DEFAULT_INSERTED_LCK :
        {
            oledBitmapDrawFlash(0, 0, BITMAP_HAD, OLED_SCROLL_UP);
            break;
        }
        case SCREEN_DEFAULT_INSERTED_NLCK :
        {
            oledBitmapDrawFlash(0, 0, BITMAP_MAIN_SCREEN, OLED_SCROLL_UP);
            break;
        }
        case SCREEN_DEFAULT_INSERTED_INVALID :
        {
            guiDisplayInformationOnScreen(PSTR("Please Remove The Card"));
            break;
        }  
        case SCREEN_SETTINGS :
        {
            oledBitmapDrawFlash(0, 0, BITMAP_SETTINGS_SC, OLED_SCROLL_UP);
            break;            
        }
        default : break;
    }
}

/*! \fn     guiDisplayProcessingScreen(void)
*   \brief  Inform the user the mooltipass is busy
*/
void guiDisplayProcessingScreen(void)
{
    guiDisplayInformationOnScreen(PSTR("Processing..."));    
}

/*! \fn     informGuiOfCurrentContext(char* context)
*   \param  context String of the context
*   \brief  Inform the GUI of the current context
*/
void informGuiOfCurrentContext(char* context)
{
//     oledClear();
//     oledBitmapDrawFlash(0, 0, BITMAP_SIDES, 0);
//     oledPutstrXY_P(0, 4, OLED_CENTRE, PSTR("You are currently visiting:"));
//     oledPutstrXY(0, 30, OLED_CENTRE, context);
//     oledFlipBuffers(0,0);
}

/*! \fn     guiAskForDomainAddApproval(char* name)
*   \param  context String of the context
*   \brief  Ask for user approval to add a domain
*/
RET_TYPE guiAskForDomainAddApproval(char* name)
{
    // Draw asking bitmap & wait for user input
    oledClear();
    oledBitmapDrawFlash(0, 0, BITMAP_YES_NO, 0);
    oledPutstrXY_P(0, 4, OLED_CENTRE, PSTR("Confirm new credentials for:"));
    oledPutstrXY(0, 30, OLED_CENTRE, name);
    oledFlipBuffers(0,0);
    
    if(getTouchedPositionAnswer(LED_MASK_WHEEL) == TOUCHPOS_RIGHT)
    {
        return RETURN_OK;
    }
    else
    {
        return RETURN_NOK;
    }
}

/*! \fn     guiAskForLoginAddApproval(char* name)
*   \param  name    Login that needs to be added
*   \param  service Name of the current service
*   \brief  Ask for user approval to add a login
*/
RET_TYPE guiAskForLoginAddApproval(char* name, char* service)
{
    RET_TYPE return_value;
    
    // Draw asking bitmap & wait for user input
    oledClear();
    oledBitmapDrawFlash(0, 0, BITMAP_YES_NO, 0);
    oledPutstrXY_P(0, 4, OLED_CENTRE, PSTR("Add username:"));
    oledPutstrXY(0, 21, OLED_CENTRE, name);
    oledPutstrXY(0, 36, OLED_CENTRE, "on");
    oledPutstrXY(0, 52, OLED_CENTRE, service);
    oledFlipBuffers(0,0);
    
    if(getTouchedPositionAnswer(LED_MASK_WHEEL) == TOUCHPOS_RIGHT)
    {
        return RETURN_OK;
    }
    else
    {
        return RETURN_NOK;
    }
    
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
    
    // Draw asking bitmap & wait for user input
    oledClear();
    oledBitmapDrawFlash(0, 0, BITMAP_YES_NO, 0);
    oledPutstrXY_P(0, 4, OLED_CENTRE, PSTR("Change password for:"));
    oledPutstrXY(0, 21, OLED_CENTRE, name);
    oledPutstrXY(0, 36, OLED_CENTRE, "on");
    oledPutstrXY(0, 52, OLED_CENTRE, service);
    oledFlipBuffers(0,0);
    
    if(getTouchedPositionAnswer(LED_MASK_WHEEL) == TOUCHPOS_RIGHT)
    {
        return RETURN_OK;
    }
    else
    {
        return RETURN_NOK;
    }
    
    // Get back to other screen
   guiGetBackToCurrentScreen();
    
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
    uint16_t temp_child_address;
    uint16_t addresses[4];
    int8_t i = 0;
    int8_t j;
    
    // Read the parent node
    if (readParentNode(h, p, parentNodeAddress) != RETURN_OK)
    {
        return NODE_ADDR_NULL;
    }
    
    // Read child address
    temp_child_address = p->nextChildAddress;
    
    // Check if there are stored credentials
    if (temp_child_address == NODE_ADDR_NULL)
    {
        return NODE_ADDR_NULL;
    }
    
    // Check if there's only one child
    if (readChildNode(h, c, temp_child_address) != RETURN_OK)
    {
        return NODE_ADDR_NULL;
    }
    if (c->nextChildAddress == NODE_ADDR_NULL)
    {
        // Draw asking bitmap
        oledClear();
        oledBitmapDrawFlash(0, 0, BITMAP_YES_NO, 0);
        oledPutstrXY_P(0, 4, OLED_CENTRE, PSTR("Confirm login for"));
        oledPutstrXY(0, 21, OLED_CENTRE, (char*)p->service);
        oledPutstrXY_P(0, 36, OLED_CENTRE, PSTR("with these credentials:"));
        oledPutstrXY(0, 52, OLED_CENTRE, (char*)c->login);
        oledFlipBuffers(0,0);
        
        if(getTouchedPositionAnswer(LED_MASK_WHEEL) == TOUCHPOS_RIGHT)
        {
            // Get back to other screen
            guiGetBackToCurrentScreen();
            return temp_child_address;
        }
        else
        {
            // Get back to other screen
            guiGetBackToCurrentScreen();
            return NODE_ADDR_NULL;
        }
    } 
    else
    {
        uint8_t action_chosen = FALSE;
        
        while (action_chosen != TRUE)
        {
            // Draw asking bitmap
            oledClear();
            oledBitmapDrawFlash(0, 0, BITMAP_LOGIN, 0);
            
            // Write domain name on screen
            oledPutstrXY(0, 24, OLED_CENTRE, (char*)p->service);
            
            // List logins on screen
            while ((temp_child_address != NODE_ADDR_NULL) && (i != 4))
            {
                // Read child node to get login
                if (readChildNode(h, c, temp_child_address) != RETURN_OK)
                {
                    return NODE_ADDR_NULL;
                }
                
                // Print login on screen
                if (i == 0)
                {
                    //oledPutstrXY(72, 0, OLED_RIGHT, (char*)c->login);
                    oledPutstrXY(0, 4, OLED_LEFT, (char*)c->login);
                }
                else if (i == 1)
                {
                    //oledPutstrXY(184, 0, OLED_LEFT, (char*)c->login);
                    oledPutstrXY(255, 4, OLED_RIGHT, (char*)c->login);
                }
                else if (i == 2)
                {
                    //oledPutstrXY(72, 54, OLED_RIGHT, (char*)c->login);
                    oledPutstrXY(0, 48, OLED_LEFT, (char*)c->login);
                }
                else
                {
                    //oledPutstrXY(184, 54, OLED_LEFT, (char*)c->login);
                    oledPutstrXY(255, 48, OLED_RIGHT, (char*)c->login);
                }
                
                // Store address in array, fetch next address
                addresses[i] = temp_child_address;
                temp_child_address = c->nextChildAddress;
                i++;
            }
            
            // Display picture
            oledFlipBuffers(0,0);
            
            // Set temp_child_address to last address
            temp_child_address = addresses[i-1];
            
            // Get touched quarter and check its validity
            j = getTouchedPositionAnswer(0);
            if (j == -1)
            {
                // Time out, return nothing
                temp_child_address = NODE_ADDR_NULL;
                action_chosen = TRUE;
            }
            else if (j < i)
            {
                temp_child_address = addresses[j];
                action_chosen = TRUE;                
            }
            else if (j == TOUCHPOS_LEFT)
            {
                // Get back to the initial child
                while ((i--) > 1)
                {
                    temp_child_address = c->prevChildAddress;
                    readChildNode(h, c, temp_child_address);
                }
                // If there is a previous child, go back 4 indexes
                if (c->prevChildAddress != NODE_ADDR_NULL)
                {
                    i = 4;
                    while(i--)
                    {
                        temp_child_address = c->prevChildAddress;
                        readChildNode(h, c, temp_child_address);                      
                    }
                }
                i = 0;
            }
            else if ((j == TOUCHPOS_RIGHT) && (i == 4) && (c->nextChildAddress != NODE_ADDR_NULL))
            {
                // If there are more nodes to display
                temp_child_address = c->nextChildAddress;
                i = 0;
            }
            else
            {
                // Wrong position, get back to the initial child
                while ((i--) > 1)
                {
                    temp_child_address = c->prevChildAddress;
                    readChildNode(h, c, temp_child_address);
                }
            }
        }       
        
        // Get back to other screen
        guiGetBackToCurrentScreen();
    }    

    return temp_child_address;
}

/*! \fn     guiAskForConfirmation(const char* string)
*   \brief  Ask for user confirmation for different things
*   \param  string  Pointer to the string to display
*   \return User confirmation or not
*/
RET_TYPE guiAskForConfirmation(const char* string)
{
    // Draw asking bitmap & wait for user input
    oledClear();
    oledBitmapDrawFlash(0, 0, BITMAP_YES_NO, 0);
    oledPutstrXY_P(0, 24, OLED_CENTRE, string);
    oledFlipBuffers(0,0);
    
    if(getTouchedPositionAnswer(LED_MASK_WHEEL) == TOUCHPOS_RIGHT)
    {
        // Get back to other screen
        guiGetBackToCurrentScreen();
        return RETURN_OK;
    }
    else
    {
        // Get back to other screen
        guiGetBackToCurrentScreen();
        return RETURN_NOK;
    }  
}

/*! \fn     guiDisplayInformationOnScreen(const char* string)
*   \brief  Display text information on screen
*   \param  string  Pointer to the string to display
*/
void guiDisplayInformationOnScreen(const char* string)
{
    // Draw information bitmap & wait for user input
    oledClear();
    oledBitmapDrawFlash(2, 17, BITMAP_INFO, 0);
    oledPutstrXY_P(10, 24, OLED_CENTRE, string);
    oledFlipBuffers(0,0);
}

/*! \fn     guiHandleSmartcardInserted(RET_TYPE detection_result)
*   \brief  Here is where are handled all smartcard insertion logic
*   \return RETURN_OK if user is authentified
*/
RET_TYPE guiHandleSmartcardInserted(RET_TYPE detection_result)
{
    currentScreen = SCREEN_DEFAULT_INSERTED_INVALID;
    uint8_t temp_ctr_val[AES256_CTR_LENGTH];
    uint8_t temp_buffer[AES_KEY_LENGTH/8];
    RET_TYPE return_value = RETURN_NOK;
    uint8_t temp_user_id;
    
    guiDisplayInformationOnScreen(PSTR("Card connected"));
    
    if ((detection_result == RETURN_MOOLTIPASS_PB) || (detection_result == RETURN_MOOLTIPASS_INVALID))
    {
        guiDisplayInformationOnScreen(PSTR("PB with card"));
        printSmartCardInfo();
        removeFunctionSMC();
    }
    else if (detection_result == RETURN_MOOLTIPASS_BLOCKED)
    {
        guiDisplayInformationOnScreen(PSTR("Card blocked"));
        printSmartCardInfo();
        removeFunctionSMC();
    }
    else if (detection_result == RETURN_MOOLTIPASS_BLANK)
    {
        // Ask the user to setup his mooltipass card
        if (guiAskForConfirmation(PSTR("Create new mooltipass user?")) == RETURN_OK)
        {         
            guiDisplayProcessingScreen();
            // Create a new user with his new smart card
            if (addNewUserAndNewSmartCard(SMARTCARD_DEFAULT_PIN) == RETURN_OK)
            {
                guiDisplayInformationOnScreen(PSTR("User added"));
                currentScreen = SCREEN_DEFAULT_INSERTED_NLCK;
                setSmartCardInsertedUnlocked();
                return_value = RETURN_OK;
            }
            else
            {
                guiDisplayInformationOnScreen(PSTR("Couldn't add user"));
            }
        }
        printSmartCardInfo();
    }
    else if (detection_result == RETURN_MOOLTIPASS_USER)
    {
        // Here we should ask the user for his pin and call mooltipassDetectedRoutine
        readCodeProtectedZone(temp_buffer);
        #ifdef GENERAL_LOGIC_OUTPUT_USB
            usbPrintf_P(PSTR("%d cards\r\n"), getNumberOfKnownCards());
            usbPrintf_P(PSTR("%d users\r\n"), getNumberOfKnownUsers());
        #endif
                
        // See if we know the card and if so fetch the user id & CTR nonce
        if (getUserIdFromSmartCardCPZ(temp_buffer, temp_ctr_val, &temp_user_id) == RETURN_OK)
        {
            #ifdef GENERAL_LOGIC_OUTPUT_USB
                usbPrintf_P(PSTR("Card ID found with user %d\r\n"), temp_user_id);
            #endif
                    
            // Developer mode, enter default pin code
            #ifdef NO_PIN_CODE_REQUIRED
                mooltipassDetectedRoutine(SMARTCARD_DEFAULT_PIN);
                return_value = RETURN_OK;
                setSmartCardInsertedUnlocked();
                readAES256BitsKey(temp_buffer);
                initUserFlashContext(temp_user_id);
                currentScreen = SCREEN_DEFAULT_INSERTED_NLCK;
                initEncryptionHandling(temp_buffer, temp_ctr_val);
                guiDisplayInformationOnScreen(PSTR("Card unlocked"));
            #endif
        }
        else
        {
            guiDisplayInformationOnScreen(PSTR("Card ID not found"));
                    
            // Developer mode, enter default pin code
            #ifdef NO_PIN_CODE_REQUIRED
                mooltipassDetectedRoutine(SMARTCARD_DEFAULT_PIN);
                setSmartCardInsertedUnlocked();
            #else
                removeFunctionSMC();                            // Shut down card reader
            #endif
        }
        printSmartCardInfo();
    }    
    _delay_ms(3000);
    guiGetBackToCurrentScreen();
    return return_value;   
}

/*! \fn     guiHandleSmartcardRemoved(void)
*   \brief  Function called when smartcard is removed
*/
void guiHandleSmartcardRemoved(void)
{
    uint8_t temp_ctr_val[AES256_CTR_LENGTH];
    uint8_t temp_buffer[AES_KEY_LENGTH/8];
    
    // Clear encryption context
    memset((void*)temp_buffer, 0, AES_KEY_LENGTH/8);
    memset((void*)temp_ctr_val, 0, AES256_CTR_LENGTH);
    initEncryptionHandling(temp_buffer, temp_ctr_val);
    
    // Set correct screen
    guiDisplayInformationOnScreen(PSTR("Card removed"));
    currentScreen = SCREEN_DEFAULT_NINSERTED;
    _delay_ms(2000);
    guiGetBackToCurrentScreen();
}

/*! \fn     guiDisplayInsertSmartCardScreenAndWait(void)
*   \brief  Ask for the user to insert his smart card
*   \return RETURN_OK if the user inserted and unlocked his smartcard
*/
RET_TYPE guiDisplayInsertSmartCardScreenAndWait(void)
{
    RET_TYPE card_detect_ret = RETURN_JRELEASED;
    
    // Switch on lights
    activityDetectedRoutine();

    // Draw insert bitmap
    oledClear();
    oledBitmapDrawFlash(0, 0, BITMAP_INSERT, 0);
    oledFlipBuffers(0,0);
    
    // Activate timer
    activateTimer(TIMER_USERINT, USER_INTER_DEL);
    
    // Wait for either timeout or for the user to insert his smartcard
    while ((isTimerFlagPresent(TIMER_USERINT, TRUE) == FLAG_ABSENT) && (card_detect_ret != RETURN_JDETECT))
    {
        card_detect_ret = isCardPlugged();
        touchDetectionRoutine(0);
    }    
    
    // If the user didn't insert his smart card
    if (card_detect_ret != RETURN_JDETECT)
    {
        // Get back to other screen
        guiGetBackToCurrentScreen();
        return RETURN_NOK;
    } 
    else
    {
        return guiHandleSmartcardInserted(cardDetectedRoutine());
    }    
}