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
/*!  \file     mini_gui_screen_functions.c
*    \brief    General user interface - screen functions
*    Created:  22/6/2014
*    Author:   Mathieu Stephan
*/
#include <string.h>
#include "smart_card_higher_level_functions.h"
#include "touch_higher_level_functions.h"
#include "gui_smartcard_functions.h"
#include "logic_fwflash_storage.h"
#include "gui_screen_functions.h"
#include "gui_basic_functions.h"
#include "logic_aes_and_comms.h"
#include "gui_pin_functions.h"
#include "logic_smartcard.h"
#include "timer_manager.h"
#include "logic_eeprom.h"
#include "mini_inputs.h"
#include "mooltipass.h"
#include "mini_leds.h"
#include "node_mgmt.h"
#include "oledmini.h"
#include "defines.h"
#include "delays.h"
#include "anim.h"
#include "gui.h"
#ifdef MINI_VERSION

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
    switch (currentScreen)
    {
        case SCREEN_DEFAULT_NINSERTED:
        case SCREEN_DEFAULT_INSERTED_LCK:
        {
            miniOledBitmapDrawFlash(0, 0, BITMAP_MOOLTIPASS, OLED_SCROLL_UP);
            break;
        }            
        case SCREEN_DEFAULT_INSERTED_INVALID:
        {
            guiDisplayInformationOnScreen(ID_STRING_REMOVE_CARD);
            break;
        }            
        case SCREEN_LOCK:
        case SCREEN_LOGIN:
        case SCREEN_FAVORITES:
        case SCREEN_SETTINGS:
        case SCREEN_SETTINGS_CHANGE_PIN:
        case SCREEN_SETTINGS_BACKUP:
        case SCREEN_SETTINGS_HOME:
        case SCREEN_SETTINGS_ERASE:
        {
            miniOledBitmapDrawFlash(0, 0, (currentScreen-SCREEN_LOCK)*NB_BMPS_PER_TRANSITION+BITMAP_MAIN_LOCK, OLED_SCROLL_UP);
            break;
        }
        case SCREEN_MEMORY_MGMT:
        {
            guiDisplayInformationOnScreen(ID_STRING_MEMORYMGMT);
            break;
        }
        case SCREEN_DEFAULT_INSERTED_UNKNOWN:
        {
            guiDisplayInformationOnScreen(ID_STRING_CARDID_NFOUND);
            break;
        }
        case SCREEN_DEFAULT_UPDATING:
        {
            miniOledBitmapDrawFlash(0, 0, BITMAP_UPDATING, OLED_SCROLL_FLIP);
            break;
        }
        default: break;
    }
}

/*! \fn     guiScreenLoop(uint8_t input_interface_result)
*   \brief  Function called to handle screen changes
*   \param  input_interface_result Touch detection result
*/
void guiScreenLoop(uint8_t input_interface_result)
{
    // If no press, you can return!
    if ((input_interface_result == WHEEL_ACTION_NONE) || (currentScreen == SCREEN_DEFAULT_INSERTED_INVALID) || (currentScreen == SCREEN_DEFAULT_INSERTED_UNKNOWN) || (currentScreen == SCREEN_DEFAULT_UPDATING))
    {
        return;
    }

    if (currentScreen == SCREEN_DEFAULT_NINSERTED)
    {
        // No smart card inserted, ask the user to insert one
        guiDisplayInsertSmartCardScreenAndWait();
    }
    else if (currentScreen == SCREEN_MEMORY_MGMT)
    {
        // Currently in memory management mode, tell the user to finish it via the plugin/app
        guiDisplayInformationOnScreenAndWait(ID_STRING_CLOSEMEMMGMT);
        guiGetBackToCurrentScreen();
        miniWheelClearDetections();
    }
    else if (currentScreen == SCREEN_DEFAULT_INSERTED_LCK)
    {
        // Locked screen and a detection happened, check that the user hasn't removed his card, launch unlocking process
        if ((cardDetectedRoutine() == RETURN_MOOLTIPASS_USER) && (validCardDetectedFunction(0, TRUE) == RETURN_VCARD_OK))
        {
            // User approved his pin
            unlockFeatureCheck();
            currentScreen = SCREEN_DEFAULT_INSERTED_NLCK;
        }
            
        // Go to the new screen
        guiGetBackToCurrentScreen();
    }
    else
    {
        if (input_interface_result == WHEEL_ACTION_UP)
        {
            // We can do that because of defines and bitmap order (see logic_fw_flash_storage and gui.h)
            if (currentScreen == SCREEN_LOCK)
            {
                currentScreen = SCREEN_SETTINGS;
            } 
            else if (currentScreen == SCREEN_SETTINGS_CHANGE_PIN)
            {
                currentScreen = SCREEN_SETTINGS_ERASE;
            }
            else
            {
                currentScreen--;
            }
            // We can do that because of defines and bitmap order (see logic_fw_flash_storage and gui.h)
            for (uint8_t i = 0; i < NB_BMPS_PER_TRANSITION; i++)
            {
                miniOledBitmapDrawFlash(0, 0, (currentScreen-SCREEN_LOCK)*NB_BMPS_PER_TRANSITION+BITMAP_MAIN_LOCK+NB_BMPS_PER_TRANSITION-1-i, OLED_SCROLL_FLIP);
                timerBasedDelayMs(12);
            }
        }
        else if (input_interface_result == WHEEL_ACTION_DOWN)
        {
            // We can do that because of defines and bitmap order (see logic_fw_flash_storage and gui.h)
            for (uint8_t i = 0; i < NB_BMPS_PER_TRANSITION-1; i++)
            {
                miniOledBitmapDrawFlash(0, 0, (currentScreen-SCREEN_LOCK)*NB_BMPS_PER_TRANSITION+BITMAP_MAIN_LOCK+1+i, OLED_SCROLL_FLIP);
                timerBasedDelayMs(12);
            }
            if (currentScreen == SCREEN_SETTINGS)
            {
                currentScreen = SCREEN_LOCK;
            }
            else if (currentScreen == SCREEN_SETTINGS_ERASE)
            {
                currentScreen = SCREEN_SETTINGS_CHANGE_PIN;
            }
            else
            {
                currentScreen++;
            }
            miniOledBitmapDrawFlash(0, 0, (currentScreen-SCREEN_LOCK)*NB_BMPS_PER_TRANSITION+BITMAP_MAIN_LOCK, OLED_SCROLL_FLIP);              
        }
        else if (input_interface_result == WHEEL_ACTION_LONG_CLICK)
        {
            // Long press in main menu : lock, long press in settings menu: go back to login screen
            if ((currentScreen >= SCREEN_SETTINGS_CHANGE_PIN) && (currentScreen <= SCREEN_SETTINGS_ERASE))
            {
                currentScreen = SCREEN_LOGIN;
                miniOledBitmapDrawFlash(0, 0, (currentScreen-SCREEN_LOCK)*NB_BMPS_PER_TRANSITION+BITMAP_MAIN_LOCK, OLED_SCROLL_UP);
            }
        }
        else if (input_interface_result == WHEEL_ACTION_SHORT_CLICK)
        {
            switch(currentScreen)
            {
                case SCREEN_LOCK:
                {
                    // User wants to lock his mooltipass
                    currentScreen = SCREEN_DEFAULT_INSERTED_LCK;
                    handleSmartcardRemoved();
                    guiGetBackToCurrentScreen();
                    break;
                }
                case SCREEN_LOGIN:
                {
                    // User wants to go to the login menu
                    if (getStartingParentAddress() != NODE_ADDR_NULL)
                    {
                        loginSelectLogic();
                    }
                    else
                    {
                        guiDisplayInformationOnScreenAndWait(ID_STRING_NO_CREDS);
                    }
                    guiGetBackToCurrentScreen();
                    break;
                }
                case SCREEN_FAVORITES:
                {
                    // User wants to go to the favorite menu
                    #if defined(ENABLE_CREDENTIAL_MANAGEMENT) && defined(REPLACE_FAVORITES_WITH_CREDENTIAL_MANAGEMENT)
                        managementActionPickingLogic();
                    #else
                        favoritePickingLogic();
                    #endif
                    guiGetBackToCurrentScreen();
                    break;
                }
                case SCREEN_SETTINGS:
                {
                    #if defined(MINI_VERSION) && defined(PASSWORD_FOR_USB_AND_ADMIN_FUNCS)
                    if (admin_usb_functs_enabled != FALSE)
                    {
                        currentScreen = SCREEN_SETTINGS_CHANGE_PIN;
                        miniOledBitmapDrawFlash(0, 0, (currentScreen-SCREEN_LOCK)*NB_BMPS_PER_TRANSITION+BITMAP_MAIN_LOCK, OLED_SCROLL_UP);
                    }
                    #else
                    currentScreen = SCREEN_SETTINGS_CHANGE_PIN;
                    miniOledBitmapDrawFlash(0, 0, (currentScreen-SCREEN_LOCK)*NB_BMPS_PER_TRANSITION+BITMAP_MAIN_LOCK, OLED_SCROLL_UP);
                    #endif
                    break;
                }
                case SCREEN_SETTINGS_HOME:
                {
                    currentScreen = SCREEN_LOGIN;
                    miniOledBitmapDrawFlash(0, 0, (currentScreen-SCREEN_LOCK)*NB_BMPS_PER_TRANSITION+BITMAP_MAIN_LOCK, OLED_SCROLL_UP);
                    break;
                }
                case SCREEN_SETTINGS_CHANGE_PIN:
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
                            guiDisplayInformationOnScreenAndWait(ID_STRING_PIN_CHANGED);
                        }
                        else
                        {
                            // Inform of fail
                            guiDisplayInformationOnScreenAndWait(ID_STRING_PIN_NCGHANGED);
                        }
                        pin_code = 0x0000;
                    }
                    else
                    {
                        currentScreen = SCREEN_DEFAULT_INSERTED_LCK;
                    }
                    guiGetBackToCurrentScreen();
                    break;
                }
                case SCREEN_SETTINGS_BACKUP:
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
                                currentScreen = SCREEN_DEFAULT_INSERTED_INVALID;
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
                            return;
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
                case SCREEN_SETTINGS_ERASE:
                {
                    // User wants to delete his profile in flash / eeprom....
                    if ((guiAskForConfirmation(1, (confirmationText_t*)readStoredStringToBuffer(ID_STRING_AREYOUSURE)) == RETURN_OK) && (removeCardAndReAuthUser() == RETURN_OK) && (guiAskForConfirmation(1, (confirmationText_t*)readStoredStringToBuffer(ID_STRING_AREYOURLSURE)) == RETURN_OK))
                    {
                        uint8_t currentuserid = getCurrentUserID();
                        guiDisplayProcessingScreen();
                        deleteCurrentUserFromFlash();
                            
                        if (guiAskForConfirmation(1, (confirmationText_t*)readStoredStringToBuffer(ID_STRING_ERASE_TCARD)) == RETURN_OK)
                        {
                            guiDisplayProcessingScreen();
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
                                guiDisplayProcessingScreen();
                                    
                                // Check the card type & ask user to enter his pin, check that the new user id loaded by validCardDetectedFunction is still the same
                                if ((cardDetectedRoutine() == RETURN_MOOLTIPASS_USER) && (validCardDetectedFunction(0, FALSE) == RETURN_VCARD_OK) && (currentuserid == getCurrentUserID()))
                                {
                                    eraseSmartCard();
                                }
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
                        currentScreen = SCREEN_DEFAULT_INSERTED_LCK;
                    }
                    userViewDelay();
                    handleSmartcardRemoved();
                    guiGetBackToCurrentScreen();
                    break;
                }

                default: break;
            }                
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

/*! \fn     guiDisplayTextInformationOnScreen(char* text)
*   \brief  Display text information on screen
*   \param  text    Text to display
*/
void guiDisplayTextInformationOnScreen(char* text)
{
    miniOledClearFrameBuffer();
    miniOledPutCenteredString(THREE_LINE_TEXT_SECOND_POS, text);
    miniOledFlushEntireBufferToDisplay();
}

/*! \fn     guiDisplayInformationOnScreen(uint8_t stringID)
*   \brief  Display text information on screen
*   \param  stringID    String ID to display
*/
void guiDisplayInformationOnScreen(uint8_t stringID)
{
    guiDisplayTextInformationOnScreen(readStoredStringToBuffer(stringID));
}

/*! \fn     guiDisplayInformationOnScreenAndWait(uint8_t stringID)
*   \brief  Display text information on screen, wait a few seconds
*   \param  stringID    String ID to display
*/
void guiDisplayInformationOnScreenAndWait(uint8_t stringID)
{
    guiDisplayTextInformationOnScreen(readStoredStringToBuffer(stringID));
    userViewDelay();
}

/*! \fn     guiDisplayRawString(uint8_t stringID)
*   \brief  Display raw text at current position on string
*   \param  stringID    String ID to display
*/
void guiDisplayRawString(uint8_t stringID)
{
    miniOledPutstr(readStoredStringToBuffer(stringID));
}

/*! \fn     guiDisplayLoginOrPasswordOnScreen(char* text)
*   \brief  Display a login or password on screen
*   \param  text    Text to display
*/
void guiDisplayLoginOrPasswordOnScreen(char* text)
{
    #ifdef MINI_VERSION
    miniOledAllowTextWritingYIncrement();
    #endif
    guiDisplayTextInformationOnScreen(text);
    #ifdef MINI_VERSION
    miniOledPreventTextWritingYIncrement();
    #endif
    while (miniGetWheelAction(FALSE,FALSE) == WHEEL_ACTION_NONE);
}

/*! \fn     guiDisplaySmartcardUnlockedScreen(uint8_t* username)
*   \brief  Display the smartcard unlocked screen
*   \param  username    The username (if there's one)
*/
void guiDisplaySmartcardUnlockedScreen(uint8_t* username)
{
    uint8_t temp_Y = THREE_LINE_TEXT_SECOND_POS;
    
    // Clear screen, check that the username is valid
    miniOledClearFrameBuffer();
    if ((username[0] != 0) && (username[0] != 0xFF))
    {
        temp_Y = THREE_LINE_TEXT_FIRST_POS;
        miniOledPutCenteredString(THREE_LINE_TEXT_SECOND_POS, readStoredStringToBuffer(ID_STRING_YOUR_USERNAME));
        miniOledPutCenteredString(THREE_LINE_TEXT_THIRD_POS, (char*)username);
    }
    miniOledPutCenteredString(temp_Y, readStoredStringToBuffer(ID_STRING_CARD_UNLOCKED));
    miniOledFlushEntireBufferToDisplay();   
}

/*! \fn     guiDisplayGoingToSleep(void)
*   \brief  Going to sleep code
*/
void guiDisplayGoingToSleep(void)
{
    miniOledClearFrameBuffer();
    miniOledPutstrXY(24, THREE_LINE_TEXT_SECOND_POS, OLED_CENTRE, readStoredStringToBuffer(ID_STRING_GOINGTOSLEEP));
    miniOledBitmapDrawFlash(4, 0, BITMAP_ZZZ, 0);
    miniOledFlushEntireBufferToDisplay();
}

/*! \fn     guiAskForConfirmation(const char* string)
*   \brief  Ask for user confirmation for different things
*   \param  nb_args     Number of text lines (must be either 1 2 or 3/4 (depending on the MP version))
*   \param  text_object Pointer to the text object if more than 1 line, pointer to the string if not
*   \return User confirmation or not
*/
RET_TYPE guiAskForConfirmation(uint8_t nb_args, confirmationText_t* text_object)
{
    uint8_t flash_flag_set = FALSE;
    uint8_t flash_flag = FALSE;
    uint8_t flash_sm = 0;

    // LED animation
    #ifdef LEDS_ENABLED_MINI
        miniLedsSetAnimation(ANIM_PULSE_UP_RAMP_DOWN);
    #endif
    
    // Check if we want to flash the screen
    if ((nb_args & 0xF0) != 0)
    {
        flash_flag_set = TRUE;
        nb_args = nb_args & 0x0F;
        // Check that the user didn't disable it
        if (getMooltipassParameterInEeprom(FLASH_SCREEN_PARAM) != FALSE)
        {
            flash_flag = TRUE;
        }
    }

    // Variables for scrolling
    uint8_t string_y_indexes[3];
    uint8_t string_extra_chars[3];
    uint8_t string_offset_cntrs[3] = {0,0,0};
    // Display variables
    uint8_t approve_selected = TRUE;
        
    // Draw asking bitmap
    miniOledClearFrameBuffer();
    miniOledSetMaxTextY(SSD1305_OLED_WIDTH-15);
    miniOledBitmapDrawFlash(SSD1305_OLED_WIDTH-15, 0, BITMAP_APPROVE, 0);
        
    // Display lines. 
    // Note: line are truncated at the miniOled driver level when miniOledTextWritingYIncrement is set to FALSE (default)
    if (nb_args == 1)
    {
        miniOledPutCenteredString(THREE_LINE_TEXT_SECOND_POS, (char*)text_object);
    }
    else if (nb_args == 2)
    {
        string_y_indexes[0] = TWO_LINE_TEXT_FIRST_POS;
        string_y_indexes[1] = TWO_LINE_TEXT_SECOND_POS;
    }
    else
    {
        string_y_indexes[0] = THREE_LINE_TEXT_FIRST_POS;
        string_y_indexes[1] = THREE_LINE_TEXT_SECOND_POS;
        string_y_indexes[2] = THREE_LINE_TEXT_THIRD_POS; 
    }
        
    // For loop to display lines when there is more than one arg
    if (nb_args > 1)
    {
        for (uint8_t i = 0; i < nb_args; i++)
        {
            string_extra_chars[i] = strlen(text_object->lines[i]) - miniOledPutCenteredString(string_y_indexes[i], text_object->lines[i]);
        }
    }
        
    miniOledFlushEntireBufferToDisplay();
    miniOledResetMaxTextY();
    
    // Wait for user input
    RET_TYPE input_answer = MINI_INPUT_RET_NONE;
    RET_TYPE detect_result;
        
    // Switch on lights
    activityDetectedRoutine();
        
    // Clear possible remaining detection
    miniWheelClearDetections();
        
    // Arm timer for scrolling (caps timer that isn't relevant here)
    activateTimer(TIMER_CAPS, SCROLLING_DEL);

    // Arm timer for flashing
    activateTimer(TIMER_FLASHING, 500);
        
    // Loop while no timeout occurs or no button is pressed
    while (input_answer == MINI_INPUT_RET_NONE)
    {
        // User interaction timeout or smartcard removed
        if ((hasTimerExpired(TIMER_USERINT, TRUE) == TIMER_EXPIRED) || (isSmartCardAbsent() == RETURN_OK))
        {
            input_answer = MINI_INPUT_RET_TIMEOUT;
        }
            
        // Read usb comms as the plugin could ask to cancel the request
        if (usbCancelRequestReceived() == RETURN_OK)
        {
            input_answer = MINI_INPUT_RET_TIMEOUT;
        }

        // Screen flashing logic
        if ((hasTimerExpired(TIMER_FLASHING, TRUE) == TIMER_EXPIRED) && (flash_flag == TRUE) && (flash_sm < 4))
        {
            // Look at the flash_sm LSb to know what is the display state
            if ((flash_sm++ & 0x01) != 0x00)
            {
                miniOledNormalDisplay();
            } 
            else
            {
                miniOledInvertedDisplay();
            }
            // Re-arm timer
            activateTimer(TIMER_FLASHING, 500);
        }
            
        // Check if something has been pressed
        detect_result = miniGetWheelAction(FALSE, TRUE);
        if (detect_result == WHEEL_ACTION_SHORT_CLICK)
        {
            input_answer = MINI_INPUT_RET_YES;
        }
        else if (detect_result == WHEEL_ACTION_LONG_CLICK)
        {
            input_answer = MINI_INPUT_RET_BACK;
        }

        // Knock to approve
        #if defined(HARDWARE_MINI_CLICK_V2) && !defined(NO_ACCELEROMETER_FUNCTIONALITIES)
        if ((scanAndGetDoubleZTap(FALSE) == ACC_RET_KNOCK) && (flash_flag_set != FALSE))
        {
            input_answer = MINI_INPUT_RET_YES;
        }
        #else
        (void)flash_flag_set;
        #endif
            
        // Text scrolling
        if ((hasTimerExpired(TIMER_CAPS, FALSE) == TIMER_EXPIRED) && (nb_args > 1))
        {
            miniOledClearFrameBuffer();
            activateTimer(TIMER_CAPS, SCROLLING_DEL);
            miniOledSetMaxTextY(SSD1305_OLED_WIDTH-15);
            if(approve_selected == FALSE)
            {
                miniOledBitmapDrawFlash(SSD1305_OLED_WIDTH-15, 0, BITMAP_DENY, 0);
            }
            else
            {
                miniOledBitmapDrawFlash(SSD1305_OLED_WIDTH-15, 0, BITMAP_APPROVE, 0);
            }

            for (uint8_t i = 0; i < nb_args; i++)
            {
                if (string_extra_chars[i] > 0)
                {
                    miniOledPutCenteredString(string_y_indexes[i], (text_object->lines[i]) + string_offset_cntrs[i]);
                        
                    if (string_offset_cntrs[i]++ == string_extra_chars[i])
                    {
                        string_offset_cntrs[i] = 0;
                    }
                }
                else
                {
                    miniOledPutCenteredString(string_y_indexes[i], text_object->lines[i]);
                }
            }
            miniOledFlushEntireBufferToDisplay();
            miniOledResetMaxTextY();
        }

        // Approve / deny display change
        if (getWheelCurrentIncrement() != 0)
        {
            if(approve_selected == FALSE)
            {
                miniOledBitmapDrawFlash(SSD1305_OLED_WIDTH-15, 0, BITMAP_APPROVE, 0);
            }
            else
            {
                miniOledBitmapDrawFlash(SSD1305_OLED_WIDTH-15, 0, BITMAP_DENY, 0);
            }
            approve_selected = !approve_selected;
            miniOledFlushEntireBufferToDisplay();
        }
    } 
    
    // In case display was inverted, set it normally
    miniOledNormalDisplay();  
    
    if ((input_answer == MINI_INPUT_RET_YES) && (approve_selected != FALSE))
    {
        // LED animation
        #ifdef LEDS_ENABLED_MINI
            miniLedsSetAnimation(ANIM_NONE);
        #endif
        return RETURN_OK;
    }
    else if (input_answer == MINI_INPUT_RET_BACK)
    {
        // LED animation
        #ifdef LEDS_ENABLED_MINI
            miniLedsSetAnimation(ANIM_NONE);
        #endif
        return RETURN_BACK;
    }
    else
    {
        // LED animation
        #ifdef LEDS_ENABLED_MINI
            miniLedsSetAnimation(ANIM_NONE);
        #endif
        return RETURN_NOK;
    }
}
#endif