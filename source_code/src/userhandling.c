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
/*!  \file     userhandling.c
*    \brief    Logic for user handling
*    Created:  09/6/2014
*    Author:   Mathieu Stephan
*/
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <avr/eeprom.h>
#include <avr/io.h>
#include <string.h>
#include "smart_card_higher_level_functions.h"
#include "eeprom_addresses.h"
#include "userhandling.h"
#include "smartcard.h"
#include "node_mgmt.h"
#include "entropy.h"
#include "defines.h"
#include "oledmp.h"
#include "usb.h"
#include "aes.h"
#include "gui.h"

// Password check timer value
volatile uint16_t password_check_timer_value = 0;
// Password check timer running flag
volatile uint8_t password_check_timer_on = FALSE;
// Credential timer valid flag
volatile uint8_t credential_timer_valid = FALSE;
// Credential timer value
volatile uint16_t credential_timer_val = 0;
// Know if the smart card is inserted and unlocked
uint8_t smartcard_inserted_unlocked = FALSE;
// Current highest CTR value
uint8_t highest_ctr_val[FLASH_STORAGE_CTR_LEN];
// Current nonce
uint8_t current_nonce[AES256_CTR_LENGTH];
// Selected login child node address
uint16_t selected_login_child_node_addr;
// Selected login flag (the plugin selected a login)
uint8_t selected_login_flag = FALSE;
// Context valid flag (eg we know the current service / website)
uint8_t context_valid_flag = FALSE;
// Current context parent node address
uint16_t context_parent_node_addr;
// Node management handle
mgmtHandle nodeMgmtHandle;
// AES256 context variable
aes256CtrCtx_t aesctx;
// Parent node var
pNode temp_pnode;
// Child node var
cNode temp_cnode;


/*! \fn     setSmartCardInsertedUnlocked(void)
*   \brief  set the smartcard is inserted and unlocked
*/
void setSmartCardInsertedUnlocked(void)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        smartcard_inserted_unlocked = TRUE;
    }
}

/*! \fn     clearSmartCardInsertedUnlocked(void)
*   \brief  set the smartcard is removed (called by interrupt!)
*/
void clearSmartCardInsertedUnlocked(void)
{
    credential_timer_val = 0;
    context_valid_flag = FALSE;
    selected_login_flag = FALSE;
    credential_timer_valid = FALSE;
    smartcard_inserted_unlocked = FALSE;
}

/*! \fn     getSmartCardInsertedUnlocked(void)
*   \brief  know if the smartcard is inserted and unlocked
*   \return The state
*/
uint8_t getSmartCardInsertedUnlocked(void)
{
    return smartcard_inserted_unlocked;
}

/*! \fn     launchCredentialTimer(void)
*   \brief  Launch credential timer
*/
void launchCredentialTimer(void)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        credential_timer_val = CREDENTIAL_TIMER_VALIDITY;
        credential_timer_valid = TRUE;
    }
}

/*! \fn     userHandlingTick(void)
*   \brief  Function called every ms
*/
void userHandlingTick(void)
{
    if (password_check_timer_value != 0)
    {
        if (password_check_timer_value-- == 1)
        {
            password_check_timer_on = FALSE;
        }
    }
    if (credential_timer_val != 0)
    {
        if (credential_timer_val-- == 1)
        {
            credential_timer_valid = FALSE;
        }
    }
}

/*! \fn     searchForServiceName(uint8_t* name, uint8_t length)
*   \brief  Find a given service name
*   \param  name    Name of the service / website
*   \param  length  Length of the string
*   \return Address of the found node, NODE_ADDR_NULL otherwise
*/
uint16_t searchForServiceName(uint8_t* name, uint8_t length)
{
    uint16_t next_node_addr = nodeMgmtHandle.firstParentNode;
    (void)length;
    
    if (next_node_addr == NODE_ADDR_NULL)
    {
        return NODE_ADDR_NULL;
    } 
    else
    {        
        // Start going through the nodes
        do 
        {
            if (readParentNode(&nodeMgmtHandle, &temp_pnode, next_node_addr) != RETURN_OK)
            {
                return NODE_ADDR_NULL;
            }
            if (strcmp((char*)temp_pnode.service, (char*)name) == 0)
            {
                return next_node_addr;
            }
            next_node_addr = temp_pnode.nextParentAddress;
        } 
        while (next_node_addr != NODE_ADDR_NULL);
        
        // We didn't find the service
        return NODE_ADDR_NULL;
    }    
}

/*! \fn     searchForLoginInGivenParent(uint16_t parent_addr, uint8_t* name, uint8_t length)
*   \brief  Find a given login for a given parent
*   \param  parent_addr Parent node address
*   \param  name        Name of the login
*   \param  length      Length of the string
*   \return Address of the found node, NODE_ADDR_NULL otherwise
*/
uint16_t searchForLoginInGivenParent(uint16_t parent_addr, uint8_t* name, uint8_t length)
{
    uint16_t next_node_addr;
    
    // Read parent node
    if (readParentNode(&nodeMgmtHandle, &temp_pnode, parent_addr) != RETURN_OK)
    {
        return NODE_ADDR_NULL;
    }
    
    next_node_addr = temp_pnode.nextChildAddress;
    
    // Check that there's actually a child node
    if (next_node_addr == NODE_ADDR_NULL)
    {
        return NODE_ADDR_NULL;
    }
    
    // Start going through the nodes
    do
    {
        if (readChildNode(&nodeMgmtHandle, &temp_cnode, next_node_addr) != RETURN_OK)
        {
            return NODE_ADDR_NULL;
        }
        if (strcmp((char*)temp_cnode.login, (char*)name) == 0)
        {
            return next_node_addr;
        }
        next_node_addr = temp_cnode.nextChildAddress;
    }
    while (next_node_addr != NODE_ADDR_NULL);
    
    // We didn't find the login
    return NODE_ADDR_NULL;    
}

/*! \fn     setCurrentContext(uint8_t* name, uint8_t length)
*   \brief  Set our current context
*   \param  name    Name of the desired service / website
*   \param  length  Length of the string
*   \return If we found the context
*/
RET_TYPE setCurrentContext(uint8_t* name, uint8_t length)
{
    // Look for name inside our flash
    context_parent_node_addr = searchForServiceName(name, length);
    
    // Clear all flags
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        credential_timer_val = 0;
        context_valid_flag = FALSE;
        selected_login_flag = FALSE;
        credential_timer_valid = FALSE;
    }
    
    // Inform GUI of current context
    informGuiOfCurrentContext((char*)name);
    
    // Do we know this context ?
    if ((context_parent_node_addr != NODE_ADDR_NULL) && (smartcard_inserted_unlocked == TRUE))
    {
        USBDEBUGPRINTF_P(PSTR("Active: %s\n"), name);
        context_valid_flag = TRUE;
        return RETURN_OK;
    }
    else
    {
        USBDEBUGPRINTF_P(PSTR("Fail: %s\n"), name);
        return RETURN_NOK;
    }
}

/*! \fn     addNewContext(uint8_t* name, uint8_t length)
*   \brief  Add a new context
*   \param  name    Name of the desired service / website
*   \param  length  Length of the string
*   \return If we added the context
*/
RET_TYPE addNewContext(uint8_t* name, uint8_t length)
{
    // Check if the context doesn't already exist
    if (searchForServiceName(name, length) != NODE_ADDR_NULL)
    {
        return RETURN_NOK;
    }
    
    // Ask for user approval
    if(guiAskForDomainAddApproval((char*)name) == RETURN_OK)
    {
        userIdToFlags(&temp_pnode.flags, nodeMgmtHandle.currentUserId);
        memcpy((void*)temp_pnode.service, (void*)name, length);
        
        // Create parent node for service
        if (createParentNode(&nodeMgmtHandle, &temp_pnode) != RETURN_OK)
        {
            return RETURN_NOK;
        }
        
        return RETURN_OK;
    }
    else
    {
        return RETURN_NOK;
    }
}

/*! \fn     getLoginForContext(uint8_t* buffer)
*   \brief  Get login for current context
*   \param  buffer  Buffer to store the login
*   \return If login was entered
*/
RET_TYPE getLoginForContext(char* buffer)
{
    if (context_valid_flag == FALSE)
    {
        // Context invalid
        return RETURN_NOK;
    } 
    else
    {
        // Credential timer off, ask for user to choose
        if (credential_timer_valid == FALSE)
        {
            selected_login_child_node_addr = guiAskForLoginSelect(&nodeMgmtHandle, &temp_pnode, &temp_cnode, context_parent_node_addr);
            
            // If a valid child node was selected
            if (selected_login_child_node_addr != NODE_ADDR_NULL)
            {
                selected_login_flag = TRUE;
                launchCredentialTimer();         
            }
        } 
        
        // If the user just approved!
        if ((credential_timer_valid == TRUE) && (selected_login_flag == TRUE))
        {
            // Read first child node
            if (readChildNode(&nodeMgmtHandle, &temp_cnode, selected_login_child_node_addr) != RETURN_OK)
            {
                return RETURN_NOK;
            }
            USBDEBUGPRINTF_P(PSTR("Get login "));
            strcpy((char*)buffer, (char*)temp_cnode.login);
            //usbKeybPutStr((char*)buffer); 
            return RETURN_OK;
        }
        else
        {
            return RETURN_NOK;
        }
    }
}

/*! \fn     getPasswordForContext(void)
*   \brief  Get password for current context
*   \return If password was entered
*/
RET_TYPE getPasswordForContext(char* buffer)
{
    if ((context_valid_flag == TRUE) && (credential_timer_valid == TRUE) && (selected_login_flag == TRUE))
    {
        // Fetch password from selected login and send it over USB
        if (readChildNode(&nodeMgmtHandle, &temp_cnode, selected_login_child_node_addr) != RETURN_OK)
        {
            return RETURN_NOK;
        }
        USBDEBUGPRINTF_P(PSTR("Get password "));
        strcpy((char*)buffer, (char*)temp_cnode.password);        
        //usbKeybPutStr((char*)buffer);     // XXX
        
        // Clear credential timer
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            credential_timer_val = 0;
            credential_timer_valid = FALSE;
        }
        return RETURN_OK; 
    } 
    else
    {
        return RETURN_NOK;
    }
}

/*! \fn     setLoginForContext(uint8_t* name, uint8_t length)
*   \brief  Set login for current context
*   \param  name    String containing the login
*   \param  length  String length
*   \return Operation success or not
*/
RET_TYPE setLoginForContext(uint8_t* name, uint8_t length)
{
    if (context_valid_flag == FALSE)
    {
        return RETURN_NOK;
    } 
    else
    {
        // Clear current flags
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            credential_timer_val = 0;
            selected_login_flag = FALSE;
            credential_timer_valid = FALSE;
        }
        
        // Look for given login in the flash
        selected_login_child_node_addr = searchForLoginInGivenParent(context_parent_node_addr, name, length);
                
        if (selected_login_child_node_addr != NODE_ADDR_NULL)
        {
            selected_login_flag = TRUE;
            return RETURN_OK;
        } 
        else
        {
            // If doesn't exist, ask user for confirmation to add to flash
            if (guiAskForLoginAddApproval((char*)name, (char*)temp_pnode.service) == RETURN_OK)
            {
                // Copy login into a temp cnode, and create it in the flash
                USBDEBUGPRINTF_P(PSTR("set login \"%s\"n"), name);
                memcpy((void*)temp_cnode.login, (void*)name, length);
                if(createChildNode(&nodeMgmtHandle, context_parent_node_addr, &temp_cnode) != RETURN_OK)
                {
                    return RETURN_NOK;
                }
                selected_login_child_node_addr = searchForLoginInGivenParent(context_parent_node_addr, name, length);
                selected_login_flag = TRUE;
                return RETURN_OK;
            } 
            else
            {
                return RETURN_NOK;
            }
        }
    }
}

/*! \fn     setPasswordForContext(uint8_t* password, uint8_t length)
*   \brief  Set password for current context
*   \param  password    String containing the password
*   \param  length      String length
*   \return Operation success or not
*/
RET_TYPE setPasswordForContext(uint8_t* password, uint8_t length)
{
    if ((selected_login_flag == FALSE) || (context_valid_flag == FALSE))
    {
        // Login not set
        return RETURN_NOK;
    } 
    else
    {
        // Read parent node
        if (readParentNode(&nodeMgmtHandle, &temp_pnode, context_parent_node_addr) != RETURN_OK)
        {
            return RETURN_NOK;
        }
        // Read child node
        if (readChildNode(&nodeMgmtHandle, &temp_cnode, selected_login_child_node_addr) != RETURN_OK)
        {
            return RETURN_NOK;
        }
        memcpy((void*)temp_cnode.password, (void*)password, length);
        // Ask for password changing approval
        if (guiAskForPasswordSet((char*)temp_cnode.login, (char*)password, (char*)temp_pnode.service) == RETURN_OK)
        {           
            // Update child node to store password
            if (updateChildNode(&nodeMgmtHandle, &temp_pnode, &temp_cnode, context_parent_node_addr, selected_login_child_node_addr) != RETURN_OK)
            {
                return RETURN_NOK;
            }
            USBDEBUGPRINTF_P(PSTR("set password \"%s\"\n"),password);
            return RETURN_OK;
        } 
        else
        {
            return RETURN_NOK;
        }
    }
}

/*! \fn     checkPasswordForContext(uint8_t* password, uint8_t length)
*   \brief  Check password for current context
*   \param  password    String containing the password
*   \param  length      String length
*   \return Operation success or not (see pass_check_return_t)
*/
RET_TYPE checkPasswordForContext(uint8_t* password, uint8_t length)
{
    // If timer is running
    if (password_check_timer_on == TRUE)
    {
        return RETURN_PASS_CHECK_BLOCKED;
    } 
    else
    {
        // Check if login set and context valid flag
        if ((selected_login_flag == FALSE) || (context_valid_flag == FALSE))
        {
            return RETURN_PASS_CHECK_NOK;
        } 
        else
        {
            // Check password in Flash
            // Read child node
            if (readChildNode(&nodeMgmtHandle, &temp_cnode, selected_login_child_node_addr) != RETURN_OK)
            {
                return RETURN_PASS_CHECK_NOK;
            }
            if (strcmp((char*)temp_cnode.password, (char*)password) == 0)
            {
                return RETURN_PASS_CHECK_OK;
            }
            else
            {
                ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
                {
                    password_check_timer_on = TRUE;
                    password_check_timer_value = CHECK_PASSWORD_TIMER_VAL;
                }
                return RETURN_PASS_CHECK_NOK;
            }
        }
    }
}

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
uint8_t getNumberOfKnownUsers(void)
{
    return eeprom_read_byte((uint8_t*)EEP_NB_KNOWN_USERS_ADDR);
}

/*! \fn     getNumberOfKnownCards(void)
*   \brief  Get the number of know cards
*   \return The number of cards
*/
uint8_t getNumberOfKnownCards(void)
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
    uint8_t temp_bool;
    uint8_t i,j;
    
    for (i = 0; i < getNumberOfKnownCards(); i++)
    {
        temp_bool = TRUE;
        for (j = 0; j < SMARTCARD_CPZ_LENGTH; j++)
        {
            if (buffer[j] != eeprom_read_byte((uint8_t*)EEP_SMC_IC_USER_MATCH_START_ADDR+i*SMCID_UID_MATCH_ENTRY_LENGTH+j))
            {
                temp_bool = FALSE;
            }
        }
        if (temp_bool == TRUE)
        {
            // We found the CPZ, store the aes ctr value & the user id
            for (j = 0; j < AES256_CTR_LENGTH; j++)
            {
                nonce[j] = eeprom_read_byte((uint8_t*)EEP_SMC_IC_USER_MATCH_START_ADDR+i*SMCID_UID_MATCH_ENTRY_LENGTH+SMARTCARD_CPZ_LENGTH+j);
            }
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
        for (i = 0; i < SMARTCARD_CPZ_LENGTH; i++)
        {
            // Store the CPZ
            eeprom_write_byte((uint8_t*)EEP_SMC_IC_USER_MATCH_START_ADDR + getNumberOfKnownCards()*SMCID_UID_MATCH_ENTRY_LENGTH + i, buffer[i]);
        }
        for (i = 0; i < AES256_CTR_LENGTH; i++)
        {
            // Store the AES CTR value
            eeprom_write_byte((uint8_t*)EEP_SMC_IC_USER_MATCH_START_ADDR + getNumberOfKnownCards()*SMCID_UID_MATCH_ENTRY_LENGTH + SMARTCARD_CPZ_LENGTH + i, nonce[i]);
        }
        eeprom_write_byte((uint8_t*)EEP_SMC_IC_USER_MATCH_START_ADDR + getNumberOfKnownCards()*SMCID_UID_MATCH_ENTRY_LENGTH + SMARTCARD_CPZ_LENGTH + AES256_CTR_LENGTH, userid);
        eeprom_write_byte((uint8_t*)EEP_NB_KNOWN_CARDS_ADDR, getNumberOfKnownCards()+1);
        return RETURN_OK;
    }
}

/*! \fn     findHighestCtrValueForSelectedUser(uint8_t* highest_ctr)
*   \brief  Find the highest CTR value for the selected user
*   \return Success status
*/
RET_TYPE findHighestCtrValueForSelectedUser(void)
{
    uint16_t next_pnode_addr = nodeMgmtHandle.firstParentNode;
    uint8_t found_child_node = FALSE;
    uint16_t next_cnode_addr;
    
    // Parent nodes loop
    while (next_pnode_addr != NODE_ADDR_NULL)
    {
        if (readParentNode(&nodeMgmtHandle, &temp_pnode, next_pnode_addr) != RETURN_OK)
        {
            return RETURN_NOK;
        }
        next_cnode_addr = temp_pnode.nextChildAddress;
        
        // Children nodes loop
        while (next_cnode_addr != NODE_ADDR_NULL)
        {
            if (readChildNode(&nodeMgmtHandle, &temp_cnode, next_cnode_addr) != RETURN_OK)
            {
                return RETURN_NOK;
            }
            if (TRUE)
            {
                memcpy(highest_ctr_val, temp_cnode.ctr, FLASH_STORAGE_CTR_LEN);
            }
            found_child_node = TRUE;
            next_cnode_addr = temp_cnode.nextChildAddress;
        }
        
        next_pnode_addr = temp_pnode.nextParentAddress;
    }
    
    // Empty memory, set 0
    if (found_child_node == FALSE)
    {
        for (uint8_t i = 0; i < FLASH_STORAGE_CTR_LEN; i++)
        {
            highest_ctr_val[i] = 0;
        }
    }
    else
    {
        // Increment max ctr val by 2
        
    }
    
    return RETURN_OK;
}

/*! \fn     initEncryptionHandling(uint8_t* aes_key, uint8_t* nonce)
*   \brief  Initialize our encryption/decryption part
*   \param  aes_key     Our AES256 key
*   \param  nonce       The nonce
*/
void initEncryptionHandling(uint8_t* aes_key, uint8_t* nonce)
{
    uint8_t i;
    
    for (i = 0; i < AES256_CTR_LENGTH; i++)
    {
        current_nonce[i] = nonce[i];
    }
    
    aes256CtrInit(&aesctx, aes_key, current_nonce, AES256_CTR_LENGTH);
    memset((void*)aes_key, 0, AES_KEY_LENGTH/8);
    findHighestCtrValueForSelectedUser();
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
    uint8_t i;
    
    // Get new user id
    new_user_id = getNumberOfKnownUsers();
    
    for(i = 0; i < SMARTCARD_CPZ_LENGTH; i++)
    {
        temp_buffer[i] = entropyRandom8();
    }    
    writeCodeProtectedZone(temp_buffer);                               // Write in the code protected zone
        
    for(i = 0; i < AES256_CTR_LENGTH; i++)
    {
        temp_nonce[i] = entropyRandom8();
    }
    
    // Create user profile in flash
    if (formatUserProfileMemory(new_user_id) != RETURN_OK)
    {
        return RETURN_NOK;
    }

    // Initialize node management handle
    if(initNodeManagementHandle(&nodeMgmtHandle, new_user_id) != RETURN_OK)
    {
        return RETURN_NOK;
    }
    
    // Store SMC CPZ & AES CTR <> user id, automatically update number of know cards / users
    if (writeSmartCardCPZForUserId(temp_buffer, temp_nonce, new_user_id) != RETURN_OK)
    {
        return RETURN_NOK;
    }
    
    // Generate a new AES key
    for (i = 0; i < (AES_KEY_LENGTH/8); i++)
    {
        temp_buffer[i] = entropyRandom8();
    }
    writeAES256BitsKey(temp_buffer);
    
    // Initialize encryption handling
    initEncryptionHandling(temp_buffer, temp_nonce);
    
    return RETURN_OK;
}

/*! \fn     initUserFlashContext(uint8_t user_id)
*   \brief  Initialize our flash context
*   \param  user_id The user ID
*   \return success or not
*/
RET_TYPE initUserFlashContext(uint8_t user_id)
{
    if(initNodeManagementHandle(&nodeMgmtHandle, user_id) != RETURN_OK)
    {
        return RETURN_NOK;
    }
    else
    {
        return RETURN_OK;
    }
}