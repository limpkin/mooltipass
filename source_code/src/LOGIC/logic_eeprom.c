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
    eeprom_write_byte((uint8_t*)EEP_NB_KNOWN_CARDS_ADDR, 0);
    eeprom_write_byte((uint8_t*)EEP_NB_KNOWN_USERS_ADDR, 0);
}

/*! \fn     getNumberOfKnownUsers(void)
*   \brief  Get the number of know users
*   \return The number of users
*/
static inline uint8_t getNumberOfKnownUsers(void)
{
    return eeprom_read_byte((uint8_t*)EEP_NB_KNOWN_USERS_ADDR);
}

/*! \fn     getNumberOfKnownCards(void)
*   \brief  Get the number of know cards
*   \return The number of cards
*/
static inline uint8_t getNumberOfKnownCards(void)
{
    return eeprom_read_byte((uint8_t*)EEP_NB_KNOWN_CARDS_ADDR);
}

/*! \fn     findUserId(uint8_t userid)
*   \brief  Find a given user ID
*   \param  userid  The user ID
*   \return Yes or No...
*/
RET_TYPE findUserId(uint8_t userid)
{
    uint8_t i;
    
    for (i = 0; i < getNumberOfKnownCards(); i++)
    {
        if (eeprom_read_byte((uint8_t*)EEP_SMC_IC_USER_MATCH_START_ADDR+i*SMCID_UID_MATCH_ENTRY_LENGTH+SMARTCARD_CPZ_LENGTH+AES256_CTR_LENGTH) == userid)
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
    uint8_t i;
    
    // Loop through the cards we know
    for (i = 0; i < getNumberOfKnownCards(); i++)
    {
        // Read one CPZ entry
        eeprom_read_block(temp_buffer, (void*)EEP_SMC_IC_USER_MATCH_START_ADDR+i*SMCID_UID_MATCH_ENTRY_LENGTH, SMARTCARD_CPZ_LENGTH);
        
        // Check if the CPZ we read and the CPZ that is passed are the same
        if (memcmp(temp_buffer, buffer, SMARTCARD_CPZ_LENGTH) == 0)
        {
            // We found the CPZ, store the aes ctr value & the user id
            eeprom_read_block(nonce, (void*)EEP_SMC_IC_USER_MATCH_START_ADDR+i*SMCID_UID_MATCH_ENTRY_LENGTH+SMARTCARD_CPZ_LENGTH, AES256_CTR_LENGTH);
            *userid = eeprom_read_byte((uint8_t*)EEP_SMC_IC_USER_MATCH_START_ADDR+i*SMCID_UID_MATCH_ENTRY_LENGTH+SMARTCARD_CPZ_LENGTH+AES256_CTR_LENGTH);
            return RETURN_OK;            
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
    uint8_t i;
    
    if (((getNumberOfKnownCards()+1)*SMCID_UID_MATCH_ENTRY_LENGTH) + EEP_SMC_IC_USER_MATCH_START_ADDR >= EEPROM_SIZE)
    {
        // Check that we still have space to store
        return RETURN_NOK;
    }
    else if (getUserIdFromSmartCardCPZ(buffer, temp_buffer, &i) == RETURN_OK)
    {
        // Check if we don't already know the smart card
        return RETURN_NOK;
    }
    else
    {
        if (findUserId(userid) != RETURN_OK)
        {
            // Increment the number of users
            eeprom_write_byte((uint8_t*)EEP_NB_KNOWN_USERS_ADDR, getNumberOfKnownUsers()+1);
        }
        eeprom_write_block((void*)buffer, (void*)EEP_SMC_IC_USER_MATCH_START_ADDR + getNumberOfKnownCards()*SMCID_UID_MATCH_ENTRY_LENGTH, SMARTCARD_CPZ_LENGTH);
        eeprom_write_block((void*)nonce, (void*)EEP_SMC_IC_USER_MATCH_START_ADDR + getNumberOfKnownCards()*SMCID_UID_MATCH_ENTRY_LENGTH + SMARTCARD_CPZ_LENGTH, AES256_CTR_LENGTH);
        eeprom_write_byte((uint8_t*)EEP_SMC_IC_USER_MATCH_START_ADDR + getNumberOfKnownCards()*SMCID_UID_MATCH_ENTRY_LENGTH + SMARTCARD_CPZ_LENGTH + AES256_CTR_LENGTH, userid);
        eeprom_write_byte((uint8_t*)EEP_NB_KNOWN_CARDS_ADDR, getNumberOfKnownCards()+1);
        return RETURN_OK;
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
    uint16_t pin1, pin2;
    uint8_t new_user_id;
    
    // When inserting a new user and a new card, we need to setup the following elements
    // - AES encryption key, stored in the smartcard
    // - AES next available CTR, stored in the user profile
    // - AES nonce, stored in the eeprom along with the user ID
    // - Smartcard CPZ, randomly generated and stored in our eeprom along with user id & nonce
    
    // Get new user id
    new_user_id = getNumberOfKnownUsers();
    
    // Check that we didn't attain the maximum number of users
    if (new_user_id >= NODE_MAX_UID)
    {
        return RETURN_NOK;
    }
    
    // Ask user for a new pin code
    if ((guiGetPinFromUser(&pin1, PSTR("New PIN ?")) != RETURN_OK) || (guiGetPinFromUser(&pin2, PSTR("Confirm PIN")) != RETURN_OK) || (pin1 != pin2))
    {
        return RETURN_NOK;
    }
    
    // The next part can take quite a while
    guiDisplayProcessingScreen();
    
    // Create user profile in flash, CTR is set to 0 by the library
    formatUserProfileMemory(new_user_id);

    // Initialize user flash context, that inits the node mgmt handle and the ctr value
    if (initUserFlashContext(new_user_id) != RETURN_OK)
    {
        return RETURN_NOK;
    }

    // Generate random CPZ value
    fillArrayWithRandomBytes(temp_buffer, SMARTCARD_CPZ_LENGTH);

    // Generate random nonce to be stored in the eeprom
    fillArrayWithRandomBytes(temp_nonce, AES256_CTR_LENGTH);
    
    // Store SMC CPZ & AES CTR <> user id, automatically update number of know cards / users
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
    writeSecurityCode(pin1);
    
    return RETURN_OK;
}
