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
/*!  \file     gui_pin_functions.c
*    \brief    General user interface - pin functions
*    Created:  22/6/2014
*    Author:   Mathieu Stephan
*/
#include <string.h>
#include "smart_card_higher_level_functions.h"
#include "touch_higher_level_functions.h"
#include "gui_screen_functions.h"
#include "gui_basic_functions.h"
#include "defines.h"
#include "oledmp.h"
#include "delays.h"
#include "anim.h"


/*! \fn     guiDisplayPinOnPinEnteringScreen(uint8_t* current_pin, uint8_t selected_digit)
*   \brief  Overwrite the digits on the current pin entering screen
*   \param  current_pin     Array containing the pin
*   \param  selected_digit  Currently selected digit
*/
void guiDisplayPinOnPinEnteringScreen(uint8_t* current_pin, uint8_t selected_digit)
{
    oledFillXY(80, 28, 92, 24, 0x00);
    for (uint8_t i = 0; i < 4; i++)
    {
        oledSetXY(84+22*i, 30);
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

/*! \fn     guiGetPinFromUser(uint16_t* pin_code, uint8_t stringID)
*   \brief  Ask the user to enter a PIN
*   \param  pin_code    Pointer to where to store the pin code
*   \param  stringID    String ID
*   \return If the user approved the request
*/
RET_TYPE guiGetPinFromUser(uint16_t* pin_code, uint8_t stringID)
{
    // If we don't need a pin code, send default one
    #if defined(NO_PIN_CODE_REQUIRED) || defined(HARDWARE_V1)
        *pin_code = SMARTCARD_DEFAULT_PIN;
        return RETURN_OK;
    #endif
    
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
    oledBitmapDrawFlash(0, 0, BITMAP_PIN_ENTRY, 0);
    oledPutstrXY(0, 0, OLED_CENTRE, readStoredStringToBuffer(stringID));
    oledFlipBuffers(0,0);
    oledSetFont(FONT_CHECKBOOK_24);
    oledWriteActiveBuffer();
    
    // Display current pin on screen
    guiDisplayPinOnPinEnteringScreen(current_pin, selected_digit);
    
    // Clear possible remaining detection
    touchClearCurrentDetections();
    
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
        
        if (isSmartCardAbsent() == RETURN_OK)
        {
            // Smartcard removed, no reason to continue
            ret_val = RETURN_NOK;
            finished = TRUE;
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
            touchSensorInterruptLineDelay();
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
            touchSensorInterruptLineDelay();
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

/*! \fn     guiCardUnlockingProcess(void)
*   \brief  Function called for the user to unlock his smartcard
*   \return success status
*/
RET_TYPE guiCardUnlockingProcess(void)
{
    RET_TYPE temp_rettype;
    uint16_t temp_pin;
    
    while (1)
    {
        if (guiGetPinFromUser(&temp_pin, ID_STRING_INSERT_PIN) == RETURN_OK)
        {            
            // Try unlocking the smartcard
            temp_rettype = mooltipassDetectedRoutine(temp_pin);
            
            switch(temp_rettype)
            {
                case RETURN_MOOLTIPASS_4_TRIES_LEFT :
                {
                    // Smartcard unlocked
                    return RETURN_OK;
                }
                case RETURN_MOOLTIPASS_0_TRIES_LEFT :
                {
                    guiDisplayInformationOnScreen(ID_STRING_CARD_BLOCKED);
                    userViewDelay();
                    return RETURN_NOK;
                }
                case RETURN_MOOLTIPASS_PB :
                {
                    guiDisplayInformationOnScreen(ID_STRING_PB_CARD);
                    userViewDelay();
                    return RETURN_NOK;
                }
                default :
                {
                    // Both the enum and the defines allow us to do that
                    guiDisplayInformationOnScreen(ID_STRING_WRONGPIN1LEFT + temp_rettype - RETURN_MOOLTIPASS_1_TRIES_LEFT);
                    userViewDelay();
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
