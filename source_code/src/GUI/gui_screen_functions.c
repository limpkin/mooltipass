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
/*!  \file     gui_screen_functions.c
*    \brief    General user interface - screen functions
*    Created:  22/6/2014
*    Author:   Mathieu Stephan
*/
#include "smart_card_higher_level_functions.h"
#include "touch_higher_level_functions.h"
#include "gui_smartcard_functions.h"
#include "logic_fwflash_storage.h"
#include "gui_screen_functions.h"
#include "gui_basic_functions.h"
#include "logic_aes_and_comms.h"
#include "gui_pin_functions.h"
#include "logic_smartcard.h"
#include "logic_eeprom.h"
#include "node_mgmt.h"
#include "defines.h"
#include "oledmp.h"
#include "delays.h"
#include "anim.h"
#include "gui.h"

// Our current screen
uint8_t currentScreen = SCREEN_DEFAULT_NINSERTED;


/*! \fn     getCurrentScreen(void)
*   \brief  Get the current screen
*   \return The current screen
*/
uint8_t getCurrentScreen(void)
{
    return currentScreen;
}

/*! \fn     guiSetCurrentScreen(uint8_t screen)
*   \brief  Set current screen
*   \param  screen  The screen
*/
void guiSetCurrentScreen(uint8_t screen)
{
    currentScreen = screen;
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
            guiDisplayInformationOnScreen(ID_STRING_REMOVE_CARD);
            break;
        }
        case SCREEN_SETTINGS :
        {
            oledBitmapDrawFlash(0, 0, BITMAP_SETTINGS_SC, OLED_SCROLL_UP);
            break;
        }
        case SCREEN_MEMORY_MGMT :
        {
            guiDisplayInformationOnScreen(ID_STRING_MEMORYMGMT);
            break;
        }
        case SCREEN_DEFAULT_INSERTED_UNKNOWN :
        {
            guiDisplayInformationOnScreen(ID_STRING_CARDID_NFOUND);
            break;
        }
        default : break;
    }
}

/*! \fn     guiScreenLoop(uint8_t touch_detect_result)
*   \brief  Function called to handle screen changes
*   \param  touch_detect_result Touch detection result
*/
void guiScreenLoop(uint8_t touch_detect_result)
{
    uint8_t state_machine_val = currentScreen;
    
    // If no press, you can return!
    if (!(touch_detect_result & TOUCH_PRESS_MASK))
    {
        return;
    }
    
    // Prevent touches until the user lifts his finger
    touchInhibitUntilRelease();
    
    // Current screen is codded in the first 8 bytes, so we set the lowest 8 bytes to the detection quarter
    if (touch_detect_result & RETURN_WHEEL_PRESSED)
    {
        state_machine_val |= getWheelTouchDetectionQuarter();
    }
    else
    {
        state_machine_val |= 0x0F;
    }
    
    if (currentScreen == SCREEN_DEFAULT_NINSERTED)
    {
        // No smart card inserted, ask the user to insert one
        guiDisplayInsertSmartCardScreenAndWait();
    }
    else if (currentScreen == SCREEN_MEMORY_MGMT)
    {
        // Currently in memory management mode, tell the user to finish it via the plugin/app
        guiDisplayInformationOnScreen(ID_STRING_CLOSEMEMMGMT);
        userViewDelay();
        guiGetBackToCurrentScreen();
    }
    else if (currentScreen == SCREEN_DEFAULT_INSERTED_LCK)
    {
        // Locked screen and a detection happened, check that the user hasn't removed his card, launch unlocking process
        if ((cardDetectedRoutine() == RETURN_MOOLTIPASS_USER) && (validCardDetectedFunction() == RETURN_VCARD_OK))
        {
            // User approved his pin
            currentScreen = SCREEN_DEFAULT_INSERTED_NLCK;
        }
        
        // Go to the new screen
        guiGetBackToCurrentScreen();
    }
    else
    {
        switch(state_machine_val)
        {
            case (SCREEN_DEFAULT_INSERTED_NLCK|TOUCHPOS_WHEEL_BRIGHT) :
            {
                // User wants to lock his mooltipass
                currentScreen = SCREEN_DEFAULT_INSERTED_LCK;
                handleSmartcardRemoved();
                guiGetBackToCurrentScreen();
                break;
            }
            case (SCREEN_DEFAULT_INSERTED_NLCK|TOUCHPOS_WHEEL_TRIGHT) :
            {
                // User wants to go to the settings menu
                currentScreen = SCREEN_SETTINGS;
                guiGetBackToCurrentScreen();
                break;
            }
            case (SCREEN_DEFAULT_INSERTED_NLCK|TOUCHPOS_WHEEL_BLEFT) :
            {
                // User wants to go to the favorite menu
                favoritePickingLogic();
                guiGetBackToCurrentScreen();
                break;
            }
            case (SCREEN_DEFAULT_INSERTED_NLCK|TOUCHPOS_WHEEL_TLEFT) :
            {
                // User wants to go to the login menu
                loginSelectLogic();
                guiGetBackToCurrentScreen();
                break;
            }
            case (SCREEN_SETTINGS|TOUCHPOS_WHEEL_BLEFT) :
            {
                // User wants to delete his profile in flash / eeprom....
                if ((guiAskForConfirmation(1, (confirmationText_t*)readStoredStringToBuffer(ID_STRING_AREYOUSURE)) == RETURN_OK) && (removeCardAndReAuthUser() == RETURN_OK) && (guiAskForConfirmation(1, (confirmationText_t*)readStoredStringToBuffer(ID_STRING_AREYOURLSURE)) == RETURN_OK))
                {
                    uint8_t currentuserid = getCurrentUserID();
                    guiDisplayProcessingScreen();
                    deleteCurrentUserFromFlash();
                    eraseSmartCard();
                    
                    // Erase other smartcards
                    while (guiAskForConfirmation(1, (confirmationText_t*)readStoredStringToBuffer(ID_STRING_OTHECARDFUSER)) == RETURN_OK)
                    {
                        // Ask the user to insert other smartcards
                        guiDisplayInformationOnScreen(ID_STRING_INSERT_OTHER);
                        
                        // Wait for the user to remove and enter another smartcard
                        while (isCardPlugged() != RETURN_JRELEASED);
                        
                        // Wait for the user to insert a new smart card
                        while (isCardPlugged() != RETURN_JDETECT);
                        
                        // Check the card type & ask user to enter his pin, check that the new user id loaded by validCardDetectedFunction is still the same
                        if ((cardDetectedRoutine() == RETURN_MOOLTIPASS_USER) && (validCardDetectedFunction() == RETURN_VCARD_OK) && (currentuserid == getCurrentUserID()))
                        {
                            eraseSmartCard();                            
                        }
                    }
                    
                    // Delete LUT entries
                    guiDisplayProcessingScreen();
                    deleteUserIdFromSMCUIDLUT(currentuserid);
                    
                    // Go to invalid screen
                    currentScreen = SCREEN_DEFAULT_INSERTED_INVALID;
                }
                else
                {
                    handleSmartcardRemoved();
                    currentScreen = SCREEN_DEFAULT_INSERTED_LCK;                    
                }
                userViewDelay();
                guiGetBackToCurrentScreen();
                break;
            }
            case (SCREEN_SETTINGS|TOUCHPOS_WHEEL_BRIGHT) :
            {
                // User wants to clone his smartcard
                volatile uint16_t pin_code;
                RET_TYPE temp_rettype;
                
                // Reauth user
                if (removeCardAndReAuthUser() == RETURN_OK)
                {
                    // Ask for new pin
                    temp_rettype = guiAskForNewPin(&pin_code, ID_STRING_PIN_NEW_CARD);
                    if (temp_rettype == RETURN_NEW_PIN_OK)
                    {
                        // Start the cloning process
                        if (cloneSmartCardProcess(&pin_code) == RETURN_OK)
                        {
                            // Well it worked....
                        } 
                        else
                        {
                            currentScreen = SCREEN_DEFAULT_INSERTED_LCK;
                            guiDisplayInformationOnScreen(ID_STRING_TGT_CARD_NBL);
                        }
                        pin_code = 0x0000;
                    }
                    else if (temp_rettype == RETURN_NEW_PIN_DIFF)
                    {
                        currentScreen = SCREEN_DEFAULT_INSERTED_LCK;
                        guiDisplayInformationOnScreen(ID_STRING_PIN_DIFF);                        
                    }
                    else
                    {                        
                        guiGetBackToCurrentScreen();
                        break;
                    }
                }
                else
                {
                    currentScreen = SCREEN_DEFAULT_INSERTED_LCK;
                    guiDisplayInformationOnScreen(ID_STRING_FAILED);
                }
                userViewDelay();
                guiGetBackToCurrentScreen();
                break;
            }
            case (SCREEN_SETTINGS|TOUCHPOS_WHEEL_TLEFT) :
            {
                // User wants to go to the main menu
                currentScreen = SCREEN_DEFAULT_INSERTED_NLCK;
                guiGetBackToCurrentScreen();
                break;
            }
            case (SCREEN_SETTINGS|TOUCHPOS_WHEEL_TRIGHT) :
            {
                // User wants to change his PIN code
                
                // Reauth user
                if (removeCardAndReAuthUser() == RETURN_OK)
                {
                    // User approved his pin, ask his new one
                    volatile uint16_t pin_code;
                                        
                    if (guiAskForNewPin(&pin_code, ID_STRING_NEW_PINQ) == RETURN_NEW_PIN_OK)
                    {
                        // User successfully entered a new pin
                        writeSecurityCode(&pin_code);
                        // Inform of success
                        guiDisplayInformationOnScreen(ID_STRING_PIN_CHANGED);
                    }
                    else
                    {
                        // Inform of fail
                        guiDisplayInformationOnScreen(ID_STRING_PIN_NCGHANGED);
                    }
                    pin_code = 0x0000;
                    userViewDelay();
                }
                else
                {
                    currentScreen = SCREEN_DEFAULT_INSERTED_LCK;
                }
                guiGetBackToCurrentScreen();
                break;
            }
            default : break;
        }
    }    
}

/*! \fn     guiAskForNewPin(uint16_t* new_pin, uint8_t message_id)
*   \brief  Ask user to enter a new PIN
*   \param  new_pin     Pointer to where to store the new pin
*   \param  message_id  Message ID
*   \return Success status, see new_pinreturn_type_t
*/
RET_TYPE guiAskForNewPin(volatile uint16_t* new_pin, uint8_t message_id)
{
    uint16_t other_pin;
    
    // Ask the user twice for the new pin and compare them
    if ((guiGetPinFromUser(new_pin, message_id) == RETURN_OK) && (guiGetPinFromUser(&other_pin, ID_STRING_CONF_PIN) == RETURN_OK))
    {
        if (*new_pin == other_pin)
        {
            return RETURN_NEW_PIN_OK;
        } 
        else
        {
            return RETURN_NEW_PIN_DIFF;
        }
    }
    else
    {
        return RETURN_NEW_PIN_NOK;
    }
}

/*! \fn     guiDisplayProcessingScreen(void)
*   \brief  Inform the user the mooltipass is busy
*/
void guiDisplayProcessingScreen(void)
{
    guiDisplayInformationOnScreen(ID_STRING_PROCESSING);
}

/*! \fn     guiDisplayInformationOnScreen(uint8_t stringID)
*   \brief  Display text information on screen
*   \param  stringID    String ID to display
*/
void guiDisplayInformationOnScreen(uint8_t stringID)
{
    oledClear();
    oledPutstrXY(10, 24, OLED_CENTRE, readStoredStringToBuffer(stringID));
    oledBitmapDrawFlash(2, 17, BITMAP_INFO, 0);
    oledFlipBuffers(0,0);
}

/*! \fn     guiDisplayRawString(uint8_t stringID)
*   \brief  Display raw text at current position on string
*   \param  stringID    String ID to display
*/
void guiDisplayRawString(uint8_t stringID)
{
    oledPutstr(readStoredStringToBuffer(stringID));
}

/*! \fn     guiDisplayLoginOrPasswordOnScreen(char* text)
*   \brief  Display a login or password on screen
*   \param  text    Text to display
*/
void guiDisplayLoginOrPasswordOnScreen(char* text)
{
    oledClear();
    oledPutstrXY(10, 24, OLED_CENTRE, text);
    oledBitmapDrawFlash(2, 17, BITMAP_INFO, 0);
    oledFlipBuffers(0,0);
    getTouchedPositionAnswer(0);
}

/*! \fn     guiDisplaySmartcardUnlockedScreen(uint8_t* username)
*   \brief  Display the smartcard unlocked screen
*   \param  username    The username (if there's one)
*/
void guiDisplaySmartcardUnlockedScreen(uint8_t* username)
{
    uint8_t temp_Y = 24;
    
    // Clear screen, check that the username is valid
    oledClear();
    if ((username[0] != 0) && (username[0] != 0xFF))
    {
        temp_Y -= 16;
        oledPutstrXY(10, 24, OLED_CENTRE, readStoredStringToBuffer(ID_STRING_YOUR_USERNAME));
        oledPutstrXY(10, 40, OLED_CENTRE, (const char*)username);
    }
    oledPutstrXY(10, temp_Y, OLED_CENTRE, readStoredStringToBuffer(ID_STRING_CARD_UNLOCKED));
    oledBitmapDrawFlash(2, 17, BITMAP_INFO, 0);
    oledFlipBuffers(0,0);    
}

/*! \fn     guiDisplayGoingToSleep(void)
*   \brief  Going to sleep code
*/
void guiDisplayGoingToSleep(void)
{
    oledClear();
    oledPutstrXY(10, 24, OLED_CENTRE, readStoredStringToBuffer(ID_STRING_GOINGTOSLEEP));
    oledBitmapDrawFlash(2, 17, BITMAP_ZZZ, 0);
    oledFlipBuffers(0,0);    
}

/*! \fn     guiAskForConfirmation(const char* string)
*   \brief  Ask for user confirmation for different things
*   \param  nb_args     Number of text lines (must be either 1 2 or 4)
*   \param  text_object Pointer to the text object if more than 1 line, pointer to progrem string if not
*   \return User confirmation or not
*/
RET_TYPE guiAskForConfirmation(uint8_t nb_args, confirmationText_t* text_object)
{
    // Draw asking bitmap
    oledClear();
    oledBitmapDrawFlash(0, 0, BITMAP_YES_NO, 0);
    
    // If more than one line
    if (nb_args == 1)
    {
        // Yeah, that's a bit dirty
        oledPutstrXY(0, 24, OLED_CENTRE, (char*)text_object);
    }
    else
    {
        while (nb_args--)
        {
            oledPutstrXY(0, 4 + (nb_args << 4), OLED_CENTRE, text_object->lines[nb_args]);
        }
    }
    
    // Display result
    oledFlipBuffers(0,0);
    
    // Wait for user input
    if(getTouchedPositionAnswer(LED_MASK_WHEEL) == TOUCHPOS_RIGHT)
    {
        return RETURN_OK;
    }
    else
    {
        return RETURN_NOK;
    }
}