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
#include "aes256_ctr.h"
#include "entropy.h"
#include "defines.h"
#include "delays.h"


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
        guiDisplayInformationOnScreen(ID_STRING_PB_CARD);
        printSmartCardInfo();
        removeFunctionSMC();
    }
    else if (detection_result == RETURN_MOOLTIPASS_BLOCKED)
    {
        // The card is blocked, no pin code tries are remaining
        guiDisplayInformationOnScreen(ID_STRING_CARD_BLOCKED);
        printSmartCardInfo();
        removeFunctionSMC();
    }
    else if (detection_result == RETURN_MOOLTIPASS_BLANK)
    {
        // This is a user free card, we can ask the user to create a new user inside the Mooltipass
        if (guiAskForConfirmation(1, (confirmationText_t*)readStoredStringToBuffer(ID_STRING_NEWMP_USER)) == RETURN_OK)
        {
            uint16_t pin_code;
            
            // Create a new user with his new smart card
            if ((guiAskForNewPin(&pin_code) == RETURN_OK) && (addNewUserAndNewSmartCard(pin_code) == RETURN_OK))
            {
                guiDisplayInformationOnScreen(ID_STRING_USER_ADDED);
                next_screen = SCREEN_DEFAULT_INSERTED_NLCK;
                setSmartCardInsertedUnlocked();
                return_value = RETURN_OK;
            }
            else
            {
                // Something went wrong, user wasn't added
                guiDisplayInformationOnScreen(ID_STRING_USER_NADDED);
            }
        }
        printSmartCardInfo();
    }
    else if (detection_result == RETURN_MOOLTIPASS_USER)
    {
        // This a valid user smart card, we call a dedicated function for the user to unlock the card
        if (validCardDetectedFunction() == RETURN_OK)
        {
            // Card successfully unlocked
            guiDisplayInformationOnScreen(ID_STRING_CARD_UNLOCKED);
            next_screen = SCREEN_DEFAULT_INSERTED_NLCK;
            return_value = RETURN_OK;
        }
        else
        {
            guiSetCurrentScreen(next_screen);
            guiGetBackToCurrentScreen();
            return return_value;
        }
        printSmartCardInfo();
    }
    
    userViewDelay();
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
    smartcardPowerDelay();
    
    // Launch Unlocking process
    if ((cardDetectedRoutine() == RETURN_MOOLTIPASS_USER) && (validCardDetectedFunction() == RETURN_OK))
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

/*! \fn     validCardDetectedFunction(void)
*   \brief  Function called when a valid mooltipass card is detected
*   \return success or not
*/
RET_TYPE validCardDetectedFunction(void)
{
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
        
        // Ask the user to enter his PIN and check it
        if (guiCardUnlockingProcess() == RETURN_OK)
        {
            // Unlocking succeeded
            readAES256BitsKey(temp_buffer);
            initUserFlashContext(temp_user_id);
            initEncryptionHandling(temp_buffer, temp_ctr_val);
            setSmartCardInsertedUnlocked();
            return RETURN_OK;
        }
        else
        {
            // Unlocking failed
            return RETURN_NOK;
        }
    }
    else
    {
        // Tell the user we don't know this card
        guiDisplayInformationOnScreen(ID_STRING_CARDID_NFOUND);
        userViewDelay();
        
        // Developer mode, enter default pin code
        #ifdef NO_PIN_CODE_REQUIRED
            mooltipassDetectedRoutine(SMARTCARD_DEFAULT_PIN);
            setSmartCardInsertedUnlocked();
        #else
            removeFunctionSMC();                            // Shut down card reader
        #endif
        
        // Report Fail
        return RETURN_NOK;
    }
}

/*! \fn     cloneSmartCardProcess(uint16_t pincode)
*   \brief  Clone a smartcard
*   \param  pincode The current pin code
*   \return success or not
*/
RET_TYPE cloneSmartCardProcess(uint16_t pincode)
{
    // Temp buffers to store AZ1 & AZ2
    uint8_t temp_az1[SMARTCARD_AZ_BIT_LENGTH/8];
    uint8_t temp_az2[SMARTCARD_AZ_BIT_LENGTH/8];    
    uint8_t temp_ctr_val[AES256_CTR_LENGTH];
    uint8_t temp_cpz[SMARTCARD_CPZ_LENGTH];
    uint8_t temp_user_id;
    
    // Check that the current smart card is unlocked
    if (getSmartCardInsertedUnlocked() != TRUE)
    {
        return RETURN_NOK;
    }
    
    // Read code protected zone
    readCodeProtectedZone(temp_cpz);
    
    // Retrieve nonce & user id
    if (getUserIdFromSmartCardCPZ(temp_cpz, temp_ctr_val, &temp_user_id) != RETURN_OK)
    {
        return RETURN_NOK;
    }
    
    // Extract current AZ1 & AZ2
    readSMC((SMARTCARD_AZ1_BIT_START + SMARTCARD_AZ_BIT_LENGTH)/8, (SMARTCARD_AZ1_BIT_START)/8, temp_az1);
    readSMC((SMARTCARD_AZ2_BIT_START + SMARTCARD_AZ_BIT_LENGTH)/8, (SMARTCARD_AZ2_BIT_START)/8, temp_az2);
    
    // Inform the user to remove his smart card
    guiDisplayInformationOnScreen(ID_STRING_REMOVE_CARD);
    
    // Wait for the user to remove his smart card
    while (isCardPlugged() != RETURN_JRELEASED);
    
    // Inform the user to insert a blank smart card
    guiDisplayInformationOnScreen(ID_STRING_INSERT_NCARD);
    
    // Wait for the user to insert a blank smart card
    while (isCardPlugged() != RETURN_JDETECT);
    
    // Check that we have a blank card
    if (cardDetectedRoutine() != RETURN_MOOLTIPASS_BLANK)
    {
        return RETURN_NOK;
    }
    
    // Erase AZ1 & AZ2 in the new card
    eraseApplicationZone1NZone2SMC(FALSE);
    eraseApplicationZone1NZone2SMC(TRUE);
    
    // Write AZ1 & AZ2
    writeSMC(SMARTCARD_AZ1_BIT_START, SMARTCARD_AZ_BIT_LENGTH, temp_az1);
    writeSMC(SMARTCARD_AZ2_BIT_START, SMARTCARD_AZ_BIT_LENGTH, temp_az2);
    
    // Write random bytes in the code protected zone
    fillArrayWithRandomBytes(temp_az1, SMARTCARD_CPZ_LENGTH);
    writeCodeProtectedZone(temp_az1);
    
    // Add smart card to our database
    writeSmartCardCPZForUserId(temp_az1, temp_ctr_val, temp_user_id);
    
    // Write new password
    writeSecurityCode(pincode);
    
    // Set the smart card inserted unlocked flag, cleared by interrupt
    setSmartCardInsertedUnlocked();
    
    // Inform the user that it is done
    guiDisplayInformationOnScreen(ID_STRING_DONE);
    
    return RETURN_OK;
}
