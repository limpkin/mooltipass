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
/*!  \file     logic_eeprom.c
*    \brief    Firmware logic - eeprom related tasks
*    Created:  18/08/2014
*    Author:   Mathieu Stephan
*/
#include <avr/eeprom.h>
#include <string.h>
#include "smart_card_higher_level_functions.h"
#include "logic_fwflash_storage.h"
#include "gui_screen_functions.h"
#include "logic_aes_and_comms.h"
#include "gui_pin_functions.h"
#include "eeprom_addresses.h"
#include "logic_eeprom.h"
#include "hid_defines.h"
#include "node_mgmt.h"
#include "defines.h"
#include "rng.h"

// eeprom parameters init
static const uint8_t eeprom_param_init[] __attribute__((__progmem__)) =
{
    ID_KEYB_EN_US_LUT,      // KEYBOARD_LAYOUT_PARAM                Set English keyboard by default
    15,                     // USER_INTER_TIMEOUT_PARAM             Set 15secs user interaction timeout by default
    FALSE,                  // LOCK_TIMEOUT_ENABLE_PARAM            Disable timeout by default
    60,                     // LOCK_TIMEOUT_PARAM                   Set a 1 hour timeout
    6,                      // TOUCH_DI_PARAM                       Set default detection integrator (6 consecutive samples)
    0,                      // TOUCH_WHEEL_OS_PARAM_OLD             Not used anymore
    0x73,                   // TOUCH_PROX_OS_PARAM                  Set proximity sensing key settings
    FALSE,                  // OFFLINE_MODE_PARAM                   Disable offline mode by default
    FALSE,                  // SCREENSAVER_PARAM                    Disable screen saver by default
    0,                      // TOUCH_CHARGE_TIME_PARAM              Set datasheet default value for charge time
    0x21,                   // TOUCH_WHEEL_OS_PARAM0                Set touch wheel oversample (one bit gain)
    0x21,                   // TOUCH_WHEEL_OS_PARAM1                Set touch wheel oversample (one bit gain)
    0x21,                   // TOUCH_WHEEL_OS_PARAM2                Set touch wheel oversample (one bit gain)
    TRUE,                   // FLASH_SCREEN_PARAM                   Enable flashy screen by default
    TRUE,                   // USER_REQ_CANCEL_PARAM                Enable the possibility to cancel user requests from USB (NOT USED ANYMORE, but keep it until the apps don't use it anymore for its old puropose)
#ifdef POST_KICKSTARTER_UPDATE_SETUP
    FALSE,                  // TUTORIAL_BOOL_PARAM                  Disable the tutorial by default
#else
    TRUE,                   // TUTORIAL_BOOL_PARAM                  Enable the tutorial by default
#endif
    15,                     // SCREEN_SAVER_SPEED_PARAM             Speed of the screen saver
    TRUE,                   // LUT_BOOT_POPULATING                  Allow credential LUT populating at card insert
    TRUE,                   // KEY_AFTER_LOGIN_SEND_BOOL_PARAM      Allow key sending after login is manually entered
    KEY_TAB,                // KEY_AFTER_LOGIN_SEND_PARAM           The key to be sent after login is manually entered
    FALSE,                  // KEY_AFTER_PASS_SEND_BOOL_PARAM       Allow key sending after password is manually entered
    KEY_RETURN,             // KEY_AFTER_PASS_SEND_PARAM            The key to be sent after password is manually entered
    FALSE,                  // DELAY_AFTER_KEY_ENTRY_BOOL_PARAM     Bool to know if we add an extra delay when a character is typed
    5,                      // DELAY_AFTER_KEY_ENTRY_PARAM          How many ms are added after a key is typed
    FALSE,                  // INVERTED_SCREEN_AT_BOOT_PARAM        If we should invert the screen at boot
    0x80,                   // MINI_OLED_CONTRAST_CURRENT_PARAM     Default contrast current for the mini oled display
    0xFF,                   // MINI_LED_ANIM_MASK_PARAM             LED animation mask
    FALSE,                  // MINI_KNOCK_DETECT_ENABLE_PARAM       Knock detection enable
    8,                      // MINI_KNOCK_THRES_PARAM               Threshold for knock detection
    LF_EN_MASK,             // LOCK_UNLOCK_FEATURE_PARAM            Computer lock/unlock feature: enabled without win-l
    FALSE,                  // HASH_DISPLAY_FEATURE_PARAM           Display hash when unlocking the card
    FALSE,                  // RANDOM_INIT_PIN_PARAM                Random PIN when card inserted
};


/*! \fn     mooltipassParametersInit(void)
*   \brief  mooltipass parameters init
*/
void mooltipassParametersInit(void)
{
    for (uint8_t i = 0; i < sizeof(eeprom_param_init); i++)
    {
        setMooltipassParameterInEeprom(FIRST_USER_PARAM + i, pgm_read_byte(&eeprom_param_init[i]));
    }   
}

/*! \fn     firstTimeUserHandlingInit(void)
*   \brief  First time required intialization
*/
void firstTimeUserHandlingInit(void)
{
    // Fill user IDs with 0xFF to indicate empty slots
    for (uint8_t i = 0; i < NB_MAX_SMCID_UID_MATCH_ENTRIES; i++)
    {
        eeprom_write_byte((uint8_t*)(EEP_SMC_IC_USER_MATCH_START_ADDR + (uint16_t)i*SMCID_UID_MATCH_ENTRY_LENGTH), 0xFF);
    }
}

/*! \fn     controlEepromParameter(uint8_t val, uint8_t lowerBound, uint8_t upperBound)
*   \brief  Control an Eeprom parameter
*   \param  val         The value to control
*   \param  lowerBound  Minimum value
*   \param  upperBound  Max value
*   \return Controlled value
*/
uint8_t controlEepromParameter(uint8_t val, uint8_t lowerBound, uint8_t upperBound)
{
    if (val > upperBound)
    {
        return upperBound;
    } 
    else if (val < lowerBound)
    {
        return lowerBound;
    }
    else
    {
        return val;
    }
}

/*! \fn     setMooltipassParameterInEeprom(uint8_t param, uint8_t val)
*   \brief  Set a Mooltipass parameter in eeprom
*   \param  param   The parameter (see eeprom_addresses.h)
*   \param  val     Value of the parameter
*/
void setMooltipassParameterInEeprom(uint8_t param, uint8_t val)
{
    if (param < USER_RESERVED_SPACE_IN_EEP)
    {
        eeprom_write_byte((uint8_t*)EEP_USER_DATA_START_ADDR + param, val);
    }
}

/*! \fn     getMooltipassParameterInEeprom(uint8_t param)
*   \brief  Get a Mooltipass parameter from eeprom
*   \param  param   The parameter (see our define)
*   \return The parameter
*/
uint8_t getMooltipassParameterInEeprom(uint8_t param)
{
    if (param < USER_RESERVED_SPACE_IN_EEP)
    {
        return eeprom_read_byte((uint8_t*)EEP_USER_DATA_START_ADDR + param);
    }
    else
    {
        return 0x00;
    }
}

/*! \fn     deleteUserIdFromSMCUIDLUT(uint8_t userid)
*   \brief  Delete all userid LUT entries
*   \param  userid  User ID to delete
*/
void deleteUserIdFromSMCUIDLUT(uint8_t userid)
{
    uint16_t temp_address;
    
    // Browse through the LUT entries
    for (uint8_t i = 0; i < NB_MAX_SMCID_UID_MATCH_ENTRIES; i++)
    {
        temp_address = EEP_SMC_IC_USER_MATCH_START_ADDR + (uint16_t)i*SMCID_UID_MATCH_ENTRY_LENGTH;
        
        // If we find our userid, replace it with 0xFF
        if (eeprom_read_byte((uint8_t*)(temp_address)) == userid)
        {
            eeprom_write_byte((uint8_t*)(temp_address), 0xFF);
        }
    }
}

/*! \fn     findAvailableUserId(uint8_t* userid)
*   \brief  Find an available user ID
*   \param  userid          Pointer to where to store the found user id
*   \param  nb_users_free   Pointer to where to store the number of free users slot
*   \return Success status of the operation
*/
RET_TYPE findAvailableUserId(uint8_t* userid, uint8_t* nb_users_free)
{
    uint8_t userIdArray[NODE_MAX_UID];
    uint8_t temp_userid;
    uint8_t i;
    
    // Set the user id array to false
    memset(userIdArray, FALSE, NODE_MAX_UID);
    
    // Browse through our LUT and find taken User IDs
    for (i = 0; i < NB_MAX_SMCID_UID_MATCH_ENTRIES; i++)
    {
        // Read current user id
        temp_userid = eeprom_read_byte((uint8_t*)(EEP_SMC_IC_USER_MATCH_START_ADDR + (uint16_t)i*SMCID_UID_MATCH_ENTRY_LENGTH));
        
        // Check if it is valid and then store it
        if (temp_userid < NODE_MAX_UID)
        {
            userIdArray[temp_userid] = TRUE;
        }
    }

    // Browse through the found user IDs and count number of free user slots
    *nb_users_free = 0;
    for (i = 0; i < NODE_MAX_UID; i++)
    {
        if (userIdArray[i] == FALSE)
        {
            *nb_users_free = (*nb_users_free) + 1;
        }
    }
    
    // Browse through the found user IDs and report the first available one
    for (i = 0; i < NODE_MAX_UID; i++)
    {
        if (userIdArray[i] == FALSE)
        {
            *userid = i;
            return RETURN_OK;
        }
    }
    
    // Didn't find any available user ID
    return RETURN_NOK;
}

/*! \fn     findSmcUidLUTEmptySlot(uint16_t* found_address)
*   \brief  Find an empty SMC <> UID LUT slot
*   \param  found_address   Pointer to where to store the found address
*   \return Yes or No...
*/
RET_TYPE findSmcUidLUTEmptySlot(uint16_t* found_address)
{
    for (uint8_t i = 0; i < NB_MAX_SMCID_UID_MATCH_ENTRIES; i++)
    {
        // Store current address
        *found_address = EEP_SMC_IC_USER_MATCH_START_ADDR + (uint16_t)i*SMCID_UID_MATCH_ENTRY_LENGTH;
        
        // Check if user ID is above the max one
        if (eeprom_read_byte((uint8_t*)(*found_address)) >= NODE_MAX_UID)
        {
            return RETURN_OK;
        }
    }
    return RETURN_NOK;    
}

/*! \fn     outputLUTEntriesForGivenUser(uint8_t userID)
*   \brief  output CPZ CTR entries to USB for a given user
*   \param  userID  The user ID
*/
void outputLUTEntriesForGivenUser(uint8_t userID)
{
    uint8_t temp_buffer[SMARTCARD_CPZ_LENGTH+AES256_CTR_LENGTH];
    uint16_t current_address;
    uint8_t temp_userid;
    
    // Loop through the Look Up Tables entries
    for (uint8_t i = 0; i < NB_MAX_SMCID_UID_MATCH_ENTRIES; i++)
    {
        // Current address var
        current_address = EEP_SMC_IC_USER_MATCH_START_ADDR + (uint16_t)i*SMCID_UID_MATCH_ENTRY_LENGTH;
        
        // Read this LUT entry user ID
        temp_userid = eeprom_read_byte((uint8_t*)current_address);
        
        // Check that the read user ID is valid
        if (temp_userid == userID)
        {
            // Read one CPZ entry
            eeprom_read_block(temp_buffer, (void*)(current_address + 1), SMARTCARD_CPZ_LENGTH+AES256_CTR_LENGTH);
            usbSendMessage(CMD_CARD_CPZ_CTR_PACKET, SMARTCARD_CPZ_LENGTH+AES256_CTR_LENGTH, temp_buffer);
        }
    }
}

/*! \fn     getUserIdFromSmartCardCPZ(uint8_t* buffer, uint8_t* userid)
*   \brief  Get a user ID from card CPZ
*   \param  buffer      Buffer containing the CPZ
*   \param  nonce       pointer to where to store the ctr nonce
*   \param  userid      pointer to where to store the user id
*   \return If we found the CPZ
*/
RET_TYPE getUserIdFromSmartCardCPZ(uint8_t* buffer, uint8_t* nonce, uint8_t* userid)
{
    uint8_t temp_buffer[SMARTCARD_CPZ_LENGTH];
    uint16_t current_address;
    
    // Loop through the Look Up Tables entries
    for (uint8_t i = 0; i < NB_MAX_SMCID_UID_MATCH_ENTRIES; i++)
    {
        // Current address var
        current_address = EEP_SMC_IC_USER_MATCH_START_ADDR + (uint16_t)i*SMCID_UID_MATCH_ENTRY_LENGTH;
        
        // Read this LUT entry user ID
        *userid = eeprom_read_byte((uint8_t*)current_address);
        
        // Check that the read user ID is valid
        if (*userid < NODE_MAX_UID)
        {
            // Read one CPZ entry
            eeprom_read_block(temp_buffer, (void*)(current_address + 1), SMARTCARD_CPZ_LENGTH);
            
            // Check if the CPZ we read and the CPZ that is passed are the same
            if (memcmp(temp_buffer, buffer, SMARTCARD_CPZ_LENGTH) == 0)
            {
                // We found the CPZ, store the aes ctr value
                eeprom_read_block(nonce, (void*)(current_address + 1 + SMARTCARD_CPZ_LENGTH), AES256_CTR_LENGTH);
                return RETURN_OK;
            }
        }
    }
    
    return RETURN_NOK;
}

/*! \fn     writeSmartCardCPZForUserId(uint8_t* buffer, uint8_t userid)
*   \brief  Add a CPZ<>User id entry, automatically update number of users / cards
*   \param  buffer      Buffer containing the CPZ
*   \param  nonce       Buffer containing the AES CTR nonce
*   \param  userid      user id
*   \return If we could add the entry
*/
RET_TYPE writeSmartCardCPZForUserId(uint8_t* buffer, uint8_t* nonce, uint8_t userid)
{
    uint8_t temp_buffer[AES256_CTR_LENGTH];
    uint16_t temp_address;
    uint8_t i;    
    
    // Check that we still have space to store & that we don't already know the smart card
    if ((findSmcUidLUTEmptySlot(&temp_address) == RETURN_OK) && (getUserIdFromSmartCardCPZ(buffer, temp_buffer, &i) == RETURN_NOK))
    {
        // Store user ID, CPZ & NONCE
        eeprom_write_byte((uint8_t*)temp_address, userid);
        eeprom_write_block((void*)buffer, (void*)(temp_address + 1), SMARTCARD_CPZ_LENGTH);
        eeprom_write_block((void*)nonce, (void*)(temp_address + 1 + SMARTCARD_CPZ_LENGTH), AES256_CTR_LENGTH);
        
        // Return success!
        return RETURN_OK;
    }
    else
    {
        // No space or card already existing
        return RETURN_NOK;
    }
}

/*! \fn     addNewUserForExistingCard(uint8_t* nonce)
*   \brief  Add a new user for an already unlocked card
*   \param  nonce   User nonce
*   \param  user_id Pointer to where to store the user id
*   \return success or not
*/
RET_TYPE addNewUserForExistingCard(uint8_t* nonce, uint8_t* user_id)
{
    uint8_t temp_buffer[SMARTCARD_CPZ_LENGTH];
    uint8_t temp_val;
    
    // Get new user id if possible
    if (findAvailableUserId(user_id, &temp_val) == RETURN_NOK)
    {
        return RETURN_NOK;
    }
    
    // Create user profile in flash, CTR is set to 0 by the library
    formatUserProfileMemory(*user_id);

    // Initialize user flash context, that inits the node mgmt handle and the ctr value
    initUserFlashContext(*user_id);

    // Read smartcard CPZ value
    readCodeProtectedZone(temp_buffer);
    
    // Store User ID <> SMC CPZ & AES CTR <> user id
    if (writeSmartCardCPZForUserId(temp_buffer, nonce, *user_id) != RETURN_OK)
    {
        return RETURN_NOK;
    }
    
    return RETURN_OK;    
}

/*! \fn     addNewUserAndNewSmartCard(volatile uint16_t* pin_code)
*   \brief  Add a new user with a new smart card
*   \param  pin_code The new pin code
*   \return success or not
*/
RET_TYPE addNewUserAndNewSmartCard(volatile uint16_t* pin_code)
{
    uint8_t temp_buffer[AES_KEY_LENGTH/8];
    uint8_t temp_nonce[AES256_CTR_LENGTH];
    uint8_t new_user_id, temp_val;
    
    // When inserting a new user and a new card, we need to setup the following elements
    // - AES encryption key, stored in the smartcard
    // - AES next available CTR, stored in the user profile
    // - AES nonce, stored in the eeprom along with the user ID
    // - Smartcard CPZ, randomly generated and stored in our eeprom along with user id & nonce
    
    // The next part can take quite a while
    guiDisplayProcessingScreen();
    
    // Get new user id if possible
    if (findAvailableUserId(&new_user_id, &temp_val) == RETURN_NOK)
    {
        return RETURN_NOK;
    }
    
    // Create user profile in flash, CTR is set to 0 by the library
    formatUserProfileMemory(new_user_id);

    // Initialize user flash context, that inits the node mgmt handle and the ctr value
    initUserFlashContext(new_user_id);

    // Generate random CPZ value
    fillArrayWithRandomBytes(temp_buffer, SMARTCARD_CPZ_LENGTH);

    // Generate random nonce to be stored in the eeprom
    fillArrayWithRandomBytes(temp_nonce, AES256_CTR_LENGTH);
    
    // Store User ID <> SMC CPZ & AES CTR <> user id
    if (writeSmartCardCPZForUserId(temp_buffer, temp_nonce, new_user_id) != RETURN_OK)
    {
        return RETURN_NOK;
    }
    
    // Write random bytes in the code protected zone in the smart card
    writeCodeProtectedZone(temp_buffer);
    
    // Generate a new random AES key, write it in the smartcard
    fillArrayWithRandomBytes(temp_buffer, AES_KEY_LENGTH/8);
    writeAES256BitsKey(temp_buffer);
    
    // Initialize encryption handling
    initEncryptionHandling(temp_buffer, temp_nonce);
    
    // Write new pin code
    writeSecurityCode(pin_code);
    
    return RETURN_OK;
}
