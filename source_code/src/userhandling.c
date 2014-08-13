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
#include "timer_manager.h"
#include "userhandling.h"
#include "smartcard.h"
#include "flash_mem.h"
#include "smartcard.h"
#include "node_mgmt.h"
#include "entropy.h"
#include "defines.h"
#include "oledmp.h"
#include "utils.h"
#include "usb.h"
#include "aes.h"
#include "gui.h"

// Know if the smart card is inserted and unlocked
uint8_t smartcard_inserted_unlocked = FALSE;
// Current nonce
uint8_t current_nonce[AES256_CTR_LENGTH];
// Selected login child node address
uint16_t selected_login_child_node_addr;
// Selected login flag (the plugin selected a login)
uint8_t selected_login_flag = FALSE;
// Context valid flag (eg we know the current service / website)
uint8_t context_valid_flag = FALSE;
// Next CTR value for our AES encryption
uint8_t nextCtrVal[USER_CTR_SIZE];
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
    context_valid_flag = FALSE;
    selected_login_flag = FALSE;
    activateTimer(TIMER_CREDENTIALS, 0);
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

/*! \fn     ctrPreEncryptionTasks(void)
*   \brief  CTR pre encryption tasks
*/
void ctrPreEncryptionTasks(void)
{
    uint16_t carry = CTR_FLASH_MIN_INCR;
    uint8_t temp_buffer[USER_CTR_SIZE];
    int8_t i;
    
    // Read CTR stored in flash
    readProfileCtr(&nodeMgmtHandle, temp_buffer, USER_CTR_SIZE);
    
    // If it is the same value, increment it by X and store it in flash
    if (aesCtrCompare(temp_buffer, nextCtrVal, USER_CTR_SIZE) == 0)
    {
        for (i = USER_CTR_SIZE-1; i > 0; i--)
        {
             carry = (uint16_t)temp_buffer[i] + carry;
             temp_buffer[i] = (uint8_t)(carry);
             carry = (carry >> 8) & 0xFF;
        }
        setProfileCtr(&nodeMgmtHandle, (void*)temp_buffer, USER_CTR_SIZE);
    }
}

/*! \fn     ctrPostEncryptionTasks(void)
*   \brief  CTR post encryption tasks
*/
void ctrPostEncryptionTasks(void)
{    
    aesIncrementCtr(nextCtrVal, USER_CTR_SIZE);
    aesIncrementCtr(nextCtrVal, USER_CTR_SIZE);    
}

/*! \fn     decryptTempCNodePasswordAndClearCTVFlag(void)
*   \brief  Decrypt the password currently stored in temp_cnode.password, clear credential_timer_valid
*/
void decryptTempCNodePasswordAndClearCTVFlag(void)
{ 
    uint8_t temp_buffer[AES256_CTR_LENGTH];
    
    // Preventing side channel attacks: only send the password after a given amount of time
    activateTimer(TIMER_CREDENTIALS, AES_ENCR_DECR_TIMER_VAL);
    
    // AES decryption: xor our nonce with the ctr value, set the result, then decrypt
    memcpy((void*)temp_buffer, (void*)current_nonce, AES256_CTR_LENGTH);
    aesXorVectors(temp_buffer + (AES256_CTR_LENGTH-USER_CTR_SIZE), temp_cnode.ctr, USER_CTR_SIZE);
    aes256CtrSetIv(&aesctx, temp_buffer, AES256_CTR_LENGTH);
    aes256CtrDecrypt(&aesctx, temp_cnode.password, NODE_CHILD_SIZE_OF_PASSWORD);
    
    // Wait for credential timer to fire (we wanted to clear credential_timer_valid flag anyway)
    while (hasTimerExpired(TIMER_CREDENTIALS, FALSE) == TIMER_RUNNING);    
}

/*! \fn     encryptTempCNodePasswordAndClearCTVFlag(void)
*   \brief  Encrypt the password currently stored in temp_cnode.password, clear credential_timer_valid
*/
static inline void encryptTempCNodePasswordAndClearCTVFlag(void)
{
    uint8_t temp_buffer[AES256_CTR_LENGTH];
    
    // Preventing side channel attacks: only send the return after a given amount of time
    activateTimer(TIMER_CREDENTIALS, AES_ENCR_DECR_TIMER_VAL);

    // AES encryption: xor our nonce with the next available ctr value, set the result as IV, encrypt, increment our next available ctr value
    ctrPreEncryptionTasks();
    memcpy((void*)temp_buffer, (void*)current_nonce, AES256_CTR_LENGTH);
    aesXorVectors(temp_buffer + (AES256_CTR_LENGTH-USER_CTR_SIZE), nextCtrVal, USER_CTR_SIZE);
    aes256CtrSetIv(&aesctx, temp_buffer, AES256_CTR_LENGTH);
    aes256CtrEncrypt(&aesctx, temp_cnode.password, NODE_CHILD_SIZE_OF_PASSWORD);
    memcpy((void*)temp_cnode.ctr, (void*)nextCtrVal, USER_CTR_SIZE);
    ctrPostEncryptionTasks();

    // Wait for credential timer to fire (we wanted to clear credential_timer_valid flag anyway)
   while (hasTimerExpired(TIMER_CREDENTIALS, FALSE) == TIMER_RUNNING);
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
        context_valid_flag = FALSE;
        selected_login_flag = FALSE;
        activateTimer(TIMER_CREDENTIALS, 0);
    }
    
    // Inform GUI of current context
    informGuiOfCurrentContext((char*)name);
    
    // Do we know this context ?
    if ((context_parent_node_addr != NODE_ADDR_NULL) && (smartcard_inserted_unlocked == TRUE))
    {
        context_valid_flag = TRUE;
        return RETURN_OK;
    }
    else
    {
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
    if ((smartcard_inserted_unlocked == FALSE) || (searchForServiceName(name, length) != NODE_ADDR_NULL))
    {
        return RETURN_NOK;
    }
    
    // Ask for user approval
    if(guiAskForDomainAddApproval((char*)name) == RETURN_OK)
    {
        // Display processing screen
        guiDisplayProcessingScreen();
        
        // Copy service name inside the parent node
        memcpy((void*)temp_pnode.service, (void*)name, length);
        
        // Create parent node for service
        if (createParentNode(&nodeMgmtHandle, &temp_pnode) != RETURN_OK)
        {
            return RETURN_NOK;
        }
        
        guiGetBackToCurrentScreen();
        return RETURN_OK;
    }
    else
    {
        guiGetBackToCurrentScreen();
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
        if (hasTimerExpired(TIMER_CREDENTIALS, FALSE) == TIMER_EXPIRED)
        {
            selected_login_child_node_addr = guiAskForLoginSelect(&nodeMgmtHandle, &temp_pnode, &temp_cnode, context_parent_node_addr);
            
            // If a valid child node was selected
            if (selected_login_child_node_addr != NODE_ADDR_NULL)
            {
                selected_login_flag = TRUE;
                activateTimer(TIMER_CREDENTIALS, CREDENTIAL_TIMER_VALIDITY);
            }
        } 
        
        // If the user just approved!
        if ((hasTimerExpired(TIMER_CREDENTIALS, FALSE) == TIMER_RUNNING) && (selected_login_flag == TRUE))
        {
            // Read first child node
            if (readChildNode(&nodeMgmtHandle, &temp_cnode, selected_login_child_node_addr) != RETURN_OK)
            {
                return RETURN_NOK;
            }
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
    if ((context_valid_flag == TRUE) && (hasTimerExpired(TIMER_CREDENTIALS, FALSE) == TIMER_RUNNING) && (selected_login_flag == TRUE))
    {
        // Fetch password from selected login and send it over USB
        if (readChildNode(&nodeMgmtHandle, &temp_cnode, selected_login_child_node_addr) != RETURN_OK)
        {
            return RETURN_NOK;
        }
        
        // Call the password decryption function, which also clears the credential_timer_valid flag
        decryptTempCNodePasswordAndClearCTVFlag();
        strcpy((char*)buffer, (char*)temp_cnode.password);
        
        // Timer fired, return
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
            selected_login_flag = FALSE;
            activateTimer(TIMER_CREDENTIALS, 0);
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
                // Display processing screen
                guiDisplayProcessingScreen();
                
                // Copy login into a temp cnode, and create it in the flash
                memcpy((void*)temp_cnode.login, (void*)name, length);
                if(createChildNode(&nodeMgmtHandle, context_parent_node_addr, &temp_cnode) != RETURN_OK)
                {                    
                    return RETURN_NOK;
                }
                selected_login_child_node_addr = searchForLoginInGivenParent(context_parent_node_addr, name, length);
                selected_login_flag = TRUE;
                
                guiGetBackToCurrentScreen();
                return RETURN_OK;
            } 
            else
            {
                guiGetBackToCurrentScreen();
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
        
        // Copy the password and put random bytes after the final 0
        memcpy((void*)temp_cnode.password, (void*)password, length);
        fillArrayWithRandomBytes(temp_cnode.password + length, NODE_CHILD_SIZE_OF_PASSWORD - length);
        
        // Ask for password changing approval
        if (guiAskForPasswordSet((char*)temp_cnode.login, (char*)password, (char*)temp_pnode.service) == RETURN_OK)
        {
            // Encrypt the password
            encryptTempCNodePasswordAndClearCTVFlag();
            
            // Update child node to store password
            if (updateChildNode(&nodeMgmtHandle, &temp_pnode, &temp_cnode, context_parent_node_addr, selected_login_child_node_addr) != RETURN_OK)
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
    if (hasTimerExpired(TIMER_PASS_CHECK, FALSE) == TIMER_RUNNING)
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
                        
            // Call the password decryption function, which also clears the credential_timer_valid flag
            decryptTempCNodePasswordAndClearCTVFlag();
            
            if (strcmp((char*)temp_cnode.password, (char*)password) == 0)
            {
                return RETURN_PASS_CHECK_OK;
            }
            else
            {
                activateTimer(TIMER_PASS_CHECK, CHECK_PASSWORD_TIMER_VAL);
                return RETURN_PASS_CHECK_NOK;
            }
        }
    }
}

/*! \fn     eraseFlashUsersContents(void)
*   \brief  Erase everything inside the flash
*/
void eraseFlashUsersContents(void)
{
    sectorZeroErase(FLASH_SECTOR_ZERO_A_CODE);
    for (uint8_t i = SECTOR_START; i <= SECTOR_END; i++)
    {
        sectorErase(i);
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

/*! \fn     initEncryptionHandling(uint8_t* aes_key, uint8_t* nonce)
*   \brief  Initialize our encryption/decryption part
*   \param  aes_key     Our AES256 key
*   \param  nonce       The nonce
*/
void initEncryptionHandling(uint8_t* aes_key, uint8_t* nonce)
{
    memcpy((void*)current_nonce, (void*)nonce, AES256_CTR_LENGTH);
    aes256CtrInit(&aesctx, aes_key, current_nonce, AES256_CTR_LENGTH);
    memset((void*)aes_key, 0, AES_KEY_LENGTH/8);
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
    
    // Get new user id
    new_user_id = getNumberOfKnownUsers();

    // Write random bytes in the code protected zone
    fillArrayWithRandomBytes(temp_buffer, SMARTCARD_CPZ_LENGTH);
    writeCodeProtectedZone(temp_buffer);

    // Generate random nonce
    fillArrayWithRandomBytes(temp_nonce, AES256_CTR_LENGTH);
    
    // Create user profile in flash, CTR is set to 0 by library
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
    
    // Generate a new random AES key
    fillArrayWithRandomBytes(temp_buffer, AES_KEY_LENGTH/8);
    writeAES256BitsKey(temp_buffer);
    
    // Initialize encryption handling
    initEncryptionHandling(temp_buffer, temp_nonce);
    
    // Write new pin code
    writeSecurityCode(pin_code);
    
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
        readProfileCtr(&nodeMgmtHandle, nextCtrVal, USER_CTR_SIZE);
        return RETURN_OK;
    }
}

/*! \fn     cloneSmartCard(uint16_t pincode)
*   \brief  Clone a smartcard
*   \param  pincode The current pin code
*   \return success or not
*/
RET_TYPE cloneSmartCard(uint16_t pincode)
{
    // Temp buffers to store AZ1 & AZ2
    uint8_t temp_az1[SMARTCARD_AZ_BIT_LENGTH/8];
    uint8_t temp_az2[SMARTCARD_AZ_BIT_LENGTH/8];
    
    // Check that the current smart card is unlocked
    if (getSmartCardInsertedUnlocked() != TRUE)
    {
        return RETURN_NOK;
    }
    
    // Extract current AZ1 & AZ2
    readSMC((SMARTCARD_AZ1_BIT_START + SMARTCARD_AZ_BIT_LENGTH)/8, (SMARTCARD_AZ1_BIT_START)/8, temp_az1);
    readSMC((SMARTCARD_AZ2_BIT_START + SMARTCARD_AZ_BIT_LENGTH)/8, (SMARTCARD_AZ2_BIT_START)/8, temp_az2);
    
    // Inform the user to remove his smart card
    guiDisplayInformationOnScreen(PSTR("Remove your smartcard"));
    
    // Wait for the user to remove his smart card
    while (isCardPlugged() != RETURN_JRELEASED);
    
    // Inform the user to insert a blank smart card
    guiDisplayInformationOnScreen(PSTR("Insert new smartcard"));
    
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
    writeSmartCardCPZForUserId(temp_az1, current_nonce, nodeMgmtHandle.currentUserId);
    
    // Write new password
    writeSecurityCode(pincode);
    
    // Set the smart card inserted unlocked flag, cleared by interrupt
    setSmartCardInsertedUnlocked();
    
    // Inform the user that it is done
    guiDisplayInformationOnScreen(PSTR("Done"));
    
    return RETURN_OK;
}