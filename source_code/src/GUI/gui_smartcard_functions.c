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
/*!  \file     gui_smartcard_functions.h
*    \brief    General user interface - smartcard functions
*    Created:  22/6/2014
*    Author:   Mathieu Stephan
*/ 
#include <util/delay.h>
#include <string.h>
#include "smart_card_higher_level_functions.h"
#include "touch_higher_level_functions.h"
#include "gui_screen_functions.h"
#include "gui_basic_functions.h"
#include "timer_manager.h"
#include "userhandling.h"
#include "aes256_ctr.h"
#include "defines.h"
#include "oledmp.h"
#include "anim.h"
#include "aes.h"
#include "gui.h"


/*! \fn     guiHandleSmartcardInserted(void)
*   \brief  Here is where are handled all smartcard insertion logic
*   \return RETURN_OK if user is authenticated
*/
RET_TYPE guiHandleSmartcardInserted(void)
{
    // By default, return to invalid screen
    uint8_t next_screen = SCREEN_DEFAULT_INSERTED_INVALID;
    // Low level routine: see what kind of card we're dealing with
    RET_TYPE detection_result = cardDetectedRoutine();
    // Return fail by default
    RET_TYPE return_value = RETURN_NOK;
    
    if ((detection_result == RETURN_MOOLTIPASS_PB) || (detection_result == RETURN_MOOLTIPASS_INVALID))
    {
        // Either it is not a card or our Manufacturer Test Zone write/read test failed
        guiDisplayInformationOnScreen(PSTR("PB with card"));
        printSmartCardInfo();
        removeFunctionSMC();
    }
    else if (detection_result == RETURN_MOOLTIPASS_BLOCKED)
    {
        // The card is block, no pin code tries are remaining
        guiDisplayInformationOnScreen(PSTR("Card blocked"));
        printSmartCardInfo();
        removeFunctionSMC();
    }
    else if (detection_result == RETURN_MOOLTIPASS_BLANK)
    {
        // This is a user free card, we can ask the user to create a new user
        if (guiAskForConfirmation(1, (confirmationText_t*)PSTR("Create new mooltipass user?")) == RETURN_OK)
        {
            // Display processing screen
            guiDisplayProcessingScreen();
            
            // Create a new user with his new smart card
            if (addNewUserAndNewSmartCard(SMARTCARD_DEFAULT_PIN) == RETURN_OK)
            {
                guiDisplayInformationOnScreen(PSTR("User added"));
                next_screen = SCREEN_DEFAULT_INSERTED_NLCK;
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
        // This a valid user smart card, we call a dedicated function
        if (validCardDetectedFunction() == RETURN_OK)
        {
            // Card successfully unlocked
            guiDisplayInformationOnScreen(PSTR("Card unlocked"));
            next_screen = SCREEN_DEFAULT_INSERTED_NLCK;
            return_value = RETURN_OK;
        }
        printSmartCardInfo();
    }
    
    _delay_ms(3000);
    guiSetCurrentScreen(next_screen);
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
    
    // In case it was not done, remove power and flags
    removeFunctionSMC();
    clearSmartCardInsertedUnlocked();
    
    // Clear encryption context
    memset((void*)temp_buffer, 0, AES_KEY_LENGTH/8);
    memset((void*)temp_ctr_val, 0, AES256_CTR_LENGTH);
    initEncryptionHandling(temp_buffer, temp_ctr_val);
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
    while ((hasTimerExpired(TIMER_USERINT, TRUE) == TIMER_RUNNING) && (card_detect_ret != RETURN_JDETECT))
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
        return guiHandleSmartcardInserted();
    }
}