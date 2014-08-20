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
#include "gui_screen_functions.h"
#include "logic_aes_and_comms.h"
#include "gui_pin_functions.h"
#include "eeprom_addresses.h"
#include "logic_eeprom.h"
#include "node_mgmt.h"
#include "defines.h"
#include "entropy.h"


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
*   \param  Pointer where to store the found user id
*   \return Success status of the operation
*/
RET_TYPE findAvailableUserId(uint8_t* userid)
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

/*! \fn     addNewUserAndNewSmartCard(uint16_t pin_code)
*   \brief  Add a new user with a new smart card
*   \param  pin_code The new pin code
*   \return success or not
*/
RET_TYPE addNewUserAndNewSmartCard(uint16_t pin_code)
{
    uint8_t temp_buffer[AES_KEY_LENGTH/8];
    uint8_t temp_nonce[AES256_CTR_LENGTH];
    uint8_t new_user_id;
    
    // When inserting a new user and a new card, we need to setup the following elements
    // - AES encryption key, stored in the smartcard
    // - AES next available CTR, stored in the user profile
    // - AES nonce, stored in the eeprom along with the user ID
    // - Smartcard CPZ, randomly generated and stored in our eeprom along with user id & nonce
    
    // The next part can take quite a while
    guiDisplayProcessingScreen();
    
    // Get new user id if possible
    if (findAvailableUserId(&new_user_id) == RETURN_NOK)
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
