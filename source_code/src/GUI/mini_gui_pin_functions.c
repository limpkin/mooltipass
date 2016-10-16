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
/*!  \file     mini_gui_pin_functions.c
*    \brief    General user interface - pin functions
*    Created:  22/6/2014
*    Author:   Mathieu Stephan
*/
#include <string.h>
#include "smart_card_higher_level_functions.h"
#include "touch_higher_level_functions.h"
#include "gui_screen_functions.h"
#include "gui_basic_functions.h"
#include "usb_cmd_parser.h"
#include "logic_eeprom.h"
#include "mini_inputs.h"
#include "oledmini.h"
#include "defines.h"
#include "delays.h"
#include "anim.h"
#include "rng.h"
#ifdef MINI_VERSION


/*! \fn     guiDisplayPinOnPinEnteringScreen(uint8_t* current_pin, uint8_t selected_digit, uint8_t stringID)
*   \brief  Overwrite the digits on the current pin entering screen
*   \param  current_pin     Array containing the pin
*   \param  selected_digit  Currently selected digit
*   \param  stringID        String ID for text query
*/
void guiDisplayPinOnPinEnteringScreen(uint8_t* current_pin, uint8_t selected_digit, uint8_t stringID)
{
    // Display bitmap
    miniOledBitmapDrawFlash(0, 0, selected_digit+BITMAP_PIN_SLOT1, 0);
    miniOledSetMaxTextY(62);
    miniOledAllowTextWritingYIncrement();
    miniOledPutCenteredString(TWO_LINE_TEXT_FIRST_POS, readStoredStringToBuffer(stringID));
    miniOledPreventTextWritingYIncrement();
    miniOledResetMaxTextY();
    miniOledSetFont(FONT_PROFONT_14);
    for (uint8_t i = 0; i < 4; i++)
    {
        miniOledSetXY(64+17*i, 6);
        if (i != selected_digit)
        {
            miniOledPutch('*');
        }
        else
        {
            if (current_pin[i] >= 0x0A)
            {
                miniOledPutch(current_pin[i]+'A'-0x0A);
            }
            else
            {
                miniOledPutch(current_pin[i]+'0');
            }
        }
    }
    miniOledSetFont(FONT_DEFAULT);
    miniOledFlushEntireBufferToDisplay();
}

/*! \fn     guiGetPinFromUser(volatile uint16_t* pin_code, uint8_t stringID)
*   \brief  Ask the user to enter a PIN
*   \param  pin_code    Pointer to where to store the pin code
*   \param  stringID    String ID
*   \return If the user approved the request
*/
RET_TYPE guiGetPinFromUser(volatile uint16_t* pin_code, uint8_t stringID)
{
    // If we don't need a pin code, send default one
    #if defined(NO_PIN_CODE_REQUIRED)
        *pin_code = SMARTCARD_DEFAULT_PIN;
        return RETURN_OK;
    #endif    
    
    RET_TYPE ret_val = RETURN_NOK;
    uint8_t detection_result = 0;
    uint8_t selected_digit = 0;
    uint8_t finished = FALSE;
    uint8_t current_pin[4];
    
    // Set current pin to 0000 or random
    if (getMooltipassParameterInEeprom(RANDOM_INIT_PIN_PARAM) == FALSE)
    {
        memset((void*)current_pin, 0, 4);
    } 
    else
    {
        fillArrayWithRandomBytes(current_pin, sizeof(current_pin));
        for (uint8_t i = 0; i < sizeof(current_pin); i++)
        {
            current_pin[i] &= 0x0F;
        }
    }
        
    // Clear current detections
    miniWheelClearDetections();
    
    // Display current pin on screen
    guiDisplayPinOnPinEnteringScreen(current_pin, selected_digit, stringID);
    
    // While the user hasn't entered his pin
    while(!finished)
    {
        // Still process the USB commands
        usbProcessIncoming(USB_CALLER_PIN);
        // detection result
        detection_result = miniGetWheelAction(FALSE, FALSE);
        
        // Position increment / decrement
        if ((detection_result == WHEEL_ACTION_UP) || (detection_result == WHEEL_ACTION_DOWN))
        {
            if ((current_pin[selected_digit] == 0x0F) && (detection_result == WHEEL_ACTION_UP))
            {
                current_pin[selected_digit] = 0xFF;
            }
            else if ((current_pin[selected_digit] == 0) && (detection_result == WHEEL_ACTION_DOWN))
            {
                current_pin[selected_digit] = 0x10;
            }
            if (detection_result == WHEEL_ACTION_UP)
            {
                current_pin[selected_digit]++;
            } 
            else
            {
                current_pin[selected_digit]--;
            }
            guiDisplayPinOnPinEnteringScreen(current_pin, selected_digit, stringID);
        }
        
        // Return if card removed or timer expired
        if ((isSmartCardAbsent() == RETURN_OK) || (hasTimerExpired(TIMER_USERINT, TRUE) == TIMER_EXPIRED))
        {
            // Smartcard removed, no reason to continue
            ret_val = RETURN_NOK;
            finished = TRUE;
        }
            
        // Change digit position or return/proceed
        if (detection_result == WHEEL_ACTION_LONG_CLICK)
        {
            if (selected_digit > 0)
            {
                // When going back set pin digit to 0
                current_pin[selected_digit] = 0;
                current_pin[--selected_digit] = 0;
            }
            else
            {
                ret_val = RETURN_NOK;
                finished = TRUE;
            }
            guiDisplayPinOnPinEnteringScreen(current_pin, selected_digit, stringID);
        }
        else if (detection_result == WHEEL_ACTION_SHORT_CLICK)
        {
            if (selected_digit < 3)
            {
                selected_digit++;
            }
            else
            {
                ret_val = RETURN_OK;
                finished = TRUE;
            }
            guiDisplayPinOnPinEnteringScreen(current_pin, selected_digit, stringID);
        }
    }
    
    // Store the pin
    *pin_code = (uint16_t)(((uint16_t)(current_pin[0]) << 12) | (((uint16_t)current_pin[1]) << 8) | (current_pin[2] << 4) | current_pin[3]);
    
    // Set current pin to 0000 & set default font
    memset((void*)current_pin, 0, 4);
    miniOledSetFont(FONT_DEFAULT);
    
    // Return success status
    return ret_val;
}

/*! \fn     guiCardUnlockingProcess(void)
*   \brief  Function called for the user to unlock his smartcard
*   \return success status
*/
RET_TYPE guiCardUnlockingProcess(void)
{
    uint8_t warning_displayed = FALSE;
    volatile uint16_t temp_pin;
    RET_TYPE temp_rettype;

    /* Display warning if only one security try left */
    if (getNumberOfSecurityCodeTriesLeft() == 1)
    {
        guiDisplayInformationOnScreenAndWait(ID_STRING_LAST_PIN_TRY);
        warning_displayed = TRUE;
    }
    
    while (1)
    {
        if (guiGetPinFromUser(&temp_pin, ID_STRING_INSERT_PIN) == RETURN_OK)
        {            
            // Try unlocking the smartcard
            temp_rettype = mooltipassDetectedRoutine(&temp_pin);
            
            switch(temp_rettype)
            {
                case RETURN_MOOLTIPASS_4_TRIES_LEFT :
                {
                    // Smartcard unlocked
                    temp_pin = 0x0000;
                    return RETURN_OK;
                }
                case RETURN_MOOLTIPASS_0_TRIES_LEFT :
                {
                    guiDisplayInformationOnScreenAndWait(ID_STRING_CARD_BLOCKED);
                    return RETURN_NOK;
                }
                case RETURN_MOOLTIPASS_1_TRIES_LEFT :
                {
                    /* If after a wrong try there's only one try left, ask user to remove his card as a security */
                    guiDisplayInformationOnScreenAndWait(ID_STRING_WRONGPIN1LEFT);
                    if(warning_displayed == FALSE)
                    {
                        // Inform the user to remove his smart card
                        guiDisplayInformationOnScreen(ID_STRING_REMOVE_CARD);
                        
                        // Wait for the user to remove his smart card
                        while(isSmartCardAbsent() != RETURN_OK);
                        
                        return RETURN_NOK;
                    }
                }
                case RETURN_MOOLTIPASS_PB :
                {
                    guiDisplayInformationOnScreenAndWait(ID_STRING_PB_CARD);
                    return RETURN_NOK;
                }
                default :
                {
                    // Both the enum and the defines allow us to do that
                    guiDisplayInformationOnScreenAndWait(ID_STRING_WRONGPIN1LEFT + temp_rettype - RETURN_MOOLTIPASS_1_TRIES_LEFT);
                    break;
                }
            }
        }
        else
        {
            // User cancelled the request
            return RETURN_NOK;
        }
    }
}
#endif