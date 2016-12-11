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
/*!  \file     logic_smartcard.c
*    \brief    Firmware logic - smartcard related tasks
*    Created:  18/08/2014
*    Author:   Mathieu Stephan
*/
#include <string.h>
#include "smart_card_higher_level_functions.h"
#include "gui_smartcard_functions.h"
#include "logic_fwflash_storage.h"
#include "gui_screen_functions.h"
#include "logic_aes_and_comms.h"
#include "gui_pin_functions.h"
#include "logic_smartcard.h"
#include "logic_eeprom.h"
#include "hid_defines.h"
#include <avr/eeprom.h>
#include "aes256_ctr.h"
#include "mooltipass.h"
#include "oledmini.h"
#include "defines.h"
#include "delays.h"
#include "rng.h"


/*! \fn     unlockFeatureCheck(void)
*   \brief  After the card is unlocked this function can be called to prompt the user to use the unlock feature
*/
void unlockFeatureCheck(void)
{
    uint8_t lock_unlock_feature_uint = getMooltipassParameterInEeprom(LOCK_UNLOCK_FEATURE_PARAM);
    uint8_t loginString[NODE_CHILD_SIZE_OF_LOGIN];
    
    // As we do buffer reuse, double check it here for possible evolutions...
    #if (SMARTCARD_MTP_LOGIN_LENGTH/8) > NODE_CHILD_SIZE_OF_LOGIN
        #error "Reused loginString buffer isn't big enough"
    #endif

    // See if the lock / unlock feature is enabled, type password if so
    if ((setCurrentContext((uint8_t*)"_unlock_", SERVICE_CRED_TYPE) == RETURN_OK) && ((lock_unlock_feature_uint & LF_EN_MASK) != 0) && isUsbConfigured())
    {
        mp_lock_unlock_shortcuts = TRUE;

        // Set the first char to 0 as getLoginForContext uses it to know if there's a suggested login
        loginString[0] = 0;
        if (getLoginForContext((char*)loginString) == RETURN_OK)
        {
            if ((lock_unlock_feature_uint & LF_ENT_KEY_MASK) != 0)
            {
                /* We fetched the login (user approved), enter "enter" if feature enabled */
                usbKeyboardPress(KEY_RETURN, 0);
                timerBasedDelayMs(300);
            }            
            else if ((lock_unlock_feature_uint & LF_CTRL_ALT_DEL_MASK) != 0)
            {
                /* We fetched the login (user approved), enter ctrl-alt-del if feature enabled */
                usbKeyboardPress(KEY_DELETE, KEY_RIGHT_ALT|KEY_CTRL);
                timerBasedDelayMs(300);
            }

            /* If enabled, enter login: works because it takes less than 1s */
            if ((lock_unlock_feature_uint & LF_LOGIN_MASK) != 0)
            {
                loginString[NODE_CHILD_SIZE_OF_LOGIN-1] = 0;
                usbKeybPutStr((char*)loginString);
                usbKeyboardPress(KEY_TAB, 0);
            }

            /* Fetch the password */
            if (getPasswordForContext((char*)loginString) == RETURN_OK)
            {
                // If everything went well, type the password and press enter
                loginString[C_NODE_PWD_SIZE-1] = 0;
                usbKeybPutStr((char*)loginString);
                usbKeyboardPress(KEY_RETURN, 0);
            }
        }
    }
}

/*! \fn     handleSmartcardInserted(void)
*   \brief  Here is where are handled all smartcard insertion logic
*   \return RETURN_OK if user is authenticated
*/
RET_TYPE handleSmartcardInserted(void)
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
        guiDisplayInformationOnScreenAndWait(ID_STRING_PB_CARD);
        printSmartCardInfo();
        removeFunctionSMC();
    }
    else if (detection_result == RETURN_MOOLTIPASS_BLOCKED)
    {
        // The card is blocked, no pin code tries are remaining
        guiDisplayInformationOnScreenAndWait(ID_STRING_CARD_BLOCKED);
        printSmartCardInfo();
        removeFunctionSMC();
    }
    else if (detection_result == RETURN_MOOLTIPASS_BLANK)
    {
        // This is a user free card, we can ask the user to create a new user inside the Mooltipass
        if (guiAskForConfirmation(1, (confirmationText_t*)readStoredStringToBuffer(ID_STRING_NEWMP_USER)) == RETURN_OK)
        {
            volatile uint16_t pin_code;
            
            // Create a new user with his new smart card
            if ((guiAskForNewPin(&pin_code, ID_STRING_PIN_NEW_CARD) == RETURN_NEW_PIN_OK) && (addNewUserAndNewSmartCard(&pin_code) == RETURN_OK))
            {
                guiDisplayInformationOnScreenAndWait(ID_STRING_USER_ADDED);
                next_screen = SCREEN_DEFAULT_INSERTED_NLCK;
                setSmartCardInsertedUnlocked();
                return_value = RETURN_OK;
            }
            else
            {
                // Something went wrong, user wasn't added
                guiDisplayInformationOnScreenAndWait(ID_STRING_USER_NADDED);
            }
            pin_code = 0x0000;
        }
		else
		{			
			guiSetCurrentScreen(next_screen);
			guiGetBackToCurrentScreen();
			return return_value;
		}
        printSmartCardInfo();
    }
    else if (detection_result == RETURN_MOOLTIPASS_USER)
    {
        // Call valid card detection function
        uint8_t temp_return = validCardDetectedFunction(0, TRUE);
        
        // This a valid user smart card, we call a dedicated function for the user to unlock the card
        if (temp_return == RETURN_VCARD_OK)
        {
            unlockFeatureCheck();
            next_screen = SCREEN_DEFAULT_INSERTED_NLCK;
            return_value = RETURN_OK;
        }
        else if (temp_return == RETURN_VCARD_UNKNOWN)
        {
            // Unknown card, go to dedicated screen
            guiSetCurrentScreen(SCREEN_DEFAULT_INSERTED_UNKNOWN);
            guiGetBackToCurrentScreen();
            return return_value;
        }
        else
        {
            guiSetCurrentScreen(SCREEN_DEFAULT_INSERTED_LCK);
            guiGetBackToCurrentScreen();
            return return_value;
        }
        printSmartCardInfo();
    }
    
    guiSetCurrentScreen(next_screen);
    guiGetBackToCurrentScreen();
    return return_value;
}

/*! \fn     handleSmartcardRemoved(void)
*   \brief  Function called when smartcard is removed
*/
void handleSmartcardRemoved(void)
{
    uint8_t temp_ctr_val[AES256_CTR_LENGTH];
    uint8_t temp_buffer[AES_KEY_LENGTH/8];

    // Remove power and flags
    removeFunctionSMC();
    clearSmartCardInsertedUnlocked();
    
    // Clear encryption context
    memset((void*)temp_buffer, 0, AES_KEY_LENGTH/8);
    memset((void*)temp_ctr_val, 0, AES256_CTR_LENGTH);
    initEncryptionHandling(temp_buffer, temp_ctr_val);
}

/*! \fn     removeCardAndReAuthUser(void)
*   \brief  Re-authentication process
*   \return success or not
*/
RET_TYPE removeCardAndReAuthUser(void)
{
    uint8_t temp_cpz1[SMARTCARD_CPZ_LENGTH];    
    uint8_t temp_cpz2[SMARTCARD_CPZ_LENGTH];
    
    // Get current CPZ
    readCodeProtectedZone(temp_cpz1);
    
    // Disconnect smartcard
    handleSmartcardRemoved();
    
    // Wait a few ms
    timerBased130MsDelay();
    
    // Launch Unlocking process
    if ((cardDetectedRoutine() == RETURN_MOOLTIPASS_USER) && (validCardDetectedFunction(0, FALSE) == RETURN_VCARD_OK))
    {
        // Read other CPZ
        readCodeProtectedZone(temp_cpz2);
        
        // Check that they're actually the sames
        if (memcmp(temp_cpz1, temp_cpz2, SMARTCARD_CPZ_LENGTH) == 0)
        {
            return RETURN_OK;
        } 
        else
        {
            return RETURN_NOK;
        }
    }
    else
    {
        return RETURN_NOK;
    }
}

/*! \fn     validCardDetectedFunction(uint16_t* suggested_pin, uint8_t hash_allow_flag)
*   \brief  Function called when a valid mooltipass card is detected
*   \param  suggested_pin   If different than 0, try to unlock with this PIN (pointer)
*   \param  hash_allow_flag Set to allow hash display if option is enabled
*   \return Unlock status (see valid_card_det_return_t)
*/
RET_TYPE validCardDetectedFunction(uint16_t* suggested_pin, uint8_t hash_allow_flag)
{
    #ifdef MINI_VERSION
        uint8_t plateform_aes_key[AES_KEY_LENGTH/8];
    #endif
    uint8_t temp_ctr_val[AES256_CTR_LENGTH];
    uint8_t temp_buffer[AES_KEY_LENGTH/8];
    uint8_t temp_user_id;
    
    // Debug: output the number of known cards and users
    #ifdef GENERAL_LOGIC_OUTPUT_USB
        usbPrintf_P(PSTR("%d cards\r\n"), getNumberOfKnownCards());
        usbPrintf_P(PSTR("%d users\r\n"), getNumberOfKnownUsers());
    #endif
    
    // Read code protected zone to see if know this particular card
    readCodeProtectedZone(temp_buffer);
    
    // See if we know the card and if so fetch the user id & CTR nonce
    if (getUserIdFromSmartCardCPZ(temp_buffer, temp_ctr_val, &temp_user_id) == RETURN_OK)
    {
        // Debug: output user ID
        #ifdef GENERAL_LOGIC_OUTPUT_USB
            usbPrintf_P(PSTR("Card ID found with user %d\r\n"), temp_user_id);
        #endif

        // Display AESenc(CTR) if desired
        #ifdef MINI_VERSION
        if ((getMooltipassParameterInEeprom(HASH_DISPLAY_FEATURE_PARAM) != FALSE) && (hash_allow_flag != FALSE))
        {
            // Fetch AES key from eeprom: 30 bytes after the first 32bytes of EEP_BOOT_PWD, then last 2 bytes at EEP_LAST_AES_KEY2_2BYTES_ADDR
            eeprom_read_block(plateform_aes_key, (void*)(EEP_BOOT_PWD+(AES_KEY_LENGTH/8)), 30);
            eeprom_read_block(plateform_aes_key+30, (void*)EEP_LAST_AES_KEY2_2BYTES_ADDR, 2);

            // Display AESenc(CTRVAL)
            computeAndDisplayBlockSizeEncryptionResult(plateform_aes_key, temp_ctr_val, ID_STRING_HASH1);
        }
        #endif
        
        // Ask the user to enter his PIN and check it
        #ifdef UNLOCK_WITH_PIN_FUNCTIONALITY
        if (((suggested_pin != 0) && (mooltipassDetectedRoutine(suggested_pin) == RETURN_MOOLTIPASS_4_TRIES_LEFT)) || ((suggested_pin == 0) && (guiCardUnlockingProcess() == RETURN_OK)))
        #else
        (void)suggested_pin;
        if (guiCardUnlockingProcess() == RETURN_OK)
        #endif
        {
            // Unlocking succeeded
            readAES256BitsKey(temp_buffer);
            
            // Display AESenc(AESkey) if desired: as this check is made to make sure the device isn't compromised, it is OK to display it.
            #ifdef MINI_VERSION
            if ((getMooltipassParameterInEeprom(HASH_DISPLAY_FEATURE_PARAM) != FALSE) && (hash_allow_flag != FALSE))
            {
                // Fetch AES key from eeprom: 30 bytes after the first 32bytes of EEP_BOOT_PWD, then last 2 bytes at EEP_LAST_AES_KEY2_2BYTES_ADDR
                eeprom_read_block(plateform_aes_key, (void*)(EEP_BOOT_PWD+(AES_KEY_LENGTH/8)), 30);
                eeprom_read_block(plateform_aes_key+30, (void*)EEP_LAST_AES_KEY2_2BYTES_ADDR, 2);

                // Display AESenc(AESkey)
                computeAndDisplayBlockSizeEncryptionResult(plateform_aes_key, temp_buffer, ID_STRING_HASH2);
            }
            #endif

            // Init user flash context and encryption handling, set smartcard unlocked flag
            initUserFlashContext(temp_user_id);
            initEncryptionHandling(temp_buffer, temp_ctr_val);
            setSmartCardInsertedUnlocked();            
            return RETURN_VCARD_OK;
        }
        else
        {
            // Unlocking failed
            return RETURN_VCARD_NOK;
        }
    }
    else
    {        
        // Unknown card
        return RETURN_VCARD_UNKNOWN;
    }
}

/*! \fn     cloneSmartCardProcess(uint16_t* pincode)
*   \brief  Clone a smartcard
*   \param  pincode The current pin code
*   \return success or not
*/
RET_TYPE cloneSmartCardProcess(volatile uint16_t* pincode)
{
    /* 14/10/2016: we now copy the CPZ into the cloned card as well, as there's no need to uniquely identify the cards (and the software solution doesn't handle that case anyway) */
    
    // Temp buffers to store AZ1 & AZ2
    uint8_t temp_az1[SMARTCARD_AZ_BIT_LENGTH/8];
    uint8_t temp_az2[SMARTCARD_AZ_BIT_LENGTH/8];    
    //uint8_t temp_ctr_val[AES256_CTR_LENGTH];
    uint8_t temp_cpz[SMARTCARD_CPZ_LENGTH];
    //uint8_t temp_user_id;
    
    // Check that the current smart card is unlocked
    if (getSmartCardInsertedUnlocked() != TRUE)
    {
        return RETURN_NOK;
    }
    
    // Read code protected zone
    readCodeProtectedZone(temp_cpz);
    
    // Retrieve nonce & user id
    /*if (getUserIdFromSmartCardCPZ(temp_cpz, temp_ctr_val, &temp_user_id) != RETURN_OK)
    {
        return RETURN_NOK;
    }*/
    
    // Extract current AZ1 & AZ2
    readApplicationZone1(temp_az1);
    readApplicationZone2(temp_az2);
    
    // Inform the user to remove his smart card
    guiDisplayInformationOnScreen(ID_STRING_REMOVE_CARD);
    
    // Wait for the user to remove his smart card
    while (isCardPlugged() != RETURN_JRELEASED);
    
    // Inform the user to insert a blank smart card
    guiDisplayInformationOnScreen(ID_STRING_INSERT_NCARD);
    
    // Wait for the user to insert a blank smart card
    while (isCardPlugged() != RETURN_JDETECT);
    guiDisplayProcessingScreen();
    
    // Check that we have a blank card
    if (cardDetectedRoutine() != RETURN_MOOLTIPASS_BLANK)
    {
        return RETURN_NOK;
    }
    
    // Erase AZ1 & AZ2 in the new card
    eraseApplicationZone1NZone2SMC(FALSE);
    eraseApplicationZone1NZone2SMC(TRUE);
    
    // Write AZ1 & AZ2 & CPZ
    writeApplicationZone1(temp_az1);
    writeApplicationZone2(temp_az2);   
    writeCodeProtectedZone(temp_cpz);
    
    // No need to add smart card to our database as it is a perfect clone
    //writeSmartCardCPZForUserId(temp_az1, temp_ctr_val, temp_user_id);
    
    // Write new password
    writeSecurityCode(pincode);
    
    // Set the smart card inserted unlocked flag, cleared by interrupt
    setSmartCardInsertedUnlocked();
    
    // Inform the user that it is done
    guiDisplayInformationOnScreen(ID_STRING_DONE);
    
    return RETURN_OK;
}
