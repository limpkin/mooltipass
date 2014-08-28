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
/*!  \file     logic_aes_and_comms.c
*    \brief    Firmware logic - encryption and communications
*    Created:  18/08/2014
*    Author:   Mathieu Stephan
*/
#include <util/atomic.h>
#include <string.h>
#include "gui_credentials_functions.h"
#include "logic_fwflash_storage.h"
#include "gui_screen_functions.h"
#include "logic_aes_and_comms.h"
#include "timer_manager.h"
#include "aes256_ctr.h"
#include "node_mgmt.h"
#include "flash_mem.h"
#include "entropy.h"
#include "defines.h"
#include "delays.h"
#include "usb.h"

// Know if the smart card is inserted and unlocked
uint8_t smartcard_inserted_unlocked = FALSE;
// Current nonce for our AES256 encryption
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
// Our confirmation text variable, sent to gui functions
confirmationText_t conf_text;
// AES256 context variable
aes256CtrCtx_t aesctx;
// Parent node var
pNode temp_pnode;
// Child node var
cNode temp_cnode;


/*! \fn     getSmartCardInsertedUnlocked(void)
*   \brief  know if the smartcard is inserted and unlocked
*   \return The state
*/
uint8_t getSmartCardInsertedUnlocked(void)
{
    return smartcard_inserted_unlocked;
}

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

/*! \fn     initUserFlashContext(uint8_t user_id)
*   \brief  Initialize our flash context
*   \param  user_id The user ID
*/
void initUserFlashContext(uint8_t user_id)
{
    initNodeManagementHandle(user_id);
    readProfileCtr(nextCtrVal);
}

/*! \fn     searchForServiceName(uint8_t* name, uint8_t length)
*   \brief  Find a given service name
*   \param  name    Name of the service / website
*   \param  length  Length of the string
*   \return Address of the found node, NODE_ADDR_NULL otherwise
*/
uint16_t searchForServiceName(uint8_t* name, uint8_t length)
{
    uint16_t next_node_addr = getStartingParentAddress();
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
            // Read parent node
            readParentNode(&temp_pnode, next_node_addr);
            
            // Compare its service name with the name that was provided
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
    
    // Read parent node and get first child address
    readParentNode(&temp_pnode, parent_addr);    
    next_node_addr = temp_pnode.nextChildAddress;
    
    // Check that there's actually a child node
    if (next_node_addr == NODE_ADDR_NULL)
    {
        return NODE_ADDR_NULL;
    }
    
    // Start going through the nodes
    do
    {
        // Read child node
        readChildNode(&temp_cnode, next_node_addr);
        
        // Compare login with the provided name
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
    readProfileCtr(temp_buffer);
    
    // If it is the same value, increment it by CTR_FLASH_MIN_INCR and store it in flash
    if (aesCtrCompare(temp_buffer, nextCtrVal, USER_CTR_SIZE) == 0)
    {
        for (i = USER_CTR_SIZE-1; i > 0; i--)
        {
            carry = (uint16_t)temp_buffer[i] + carry;
            temp_buffer[i] = (uint8_t)(carry);
            carry = (carry >> 8) & 0xFF;
        }
        setProfileCtr(temp_buffer);
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
    RET_TYPE ret_val = RETURN_NOK;
    
    // Check if the context doesn't already exist
    if ((smartcard_inserted_unlocked == FALSE) || (searchForServiceName(name, length) != NODE_ADDR_NULL))
    {
        return RETURN_NOK;
    }
    
    // Prepare domain approval screen
    conf_text.lines[0] = readStoredStringToBuffer(ID_STRING_CONF_NEWCREDS);
    conf_text.lines[1] = (char*)name;
    
    // Ask for user approval
    if(guiAskForConfirmation(2, &conf_text) == RETURN_OK)
    {
        // Display processing screen
        guiDisplayProcessingScreen();
        
        // Copy service name inside the parent node
        memcpy((void*)temp_pnode.service, (void*)name, length);
        
        // Create parent node for service
        if (createParentNode(&temp_pnode) == RETURN_OK)
        {
            ret_val = RETURN_OK;
        }
    }
    
    guiGetBackToCurrentScreen();
    return ret_val;
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
            selected_login_child_node_addr = guiAskForLoginSelect(&temp_pnode, &temp_cnode, context_parent_node_addr);
            
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
            // Read selected child node
            readChildNode(&temp_cnode, selected_login_child_node_addr);
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
        readChildNode(&temp_cnode, selected_login_child_node_addr);
        
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
    RET_TYPE ret_val = RETURN_NOK;
    
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
            // Prepare confirmation screen
            conf_text.lines[0] = readStoredStringToBuffer(ID_STRING_ADDUSERNAME);
            conf_text.lines[1] = (char*)name;
            conf_text.lines[2] = readStoredStringToBuffer(ID_STRING_ON);
            conf_text.lines[3] = (char*)temp_pnode.service;
            
            // If doesn't exist, ask user for confirmation to add to flash
            if (guiAskForConfirmation(4, &conf_text) == RETURN_OK)
            {
                // Display processing screen
                guiDisplayProcessingScreen();
                
                // Copy login into a temp cnode, and create it in the flash
                memcpy((void*)temp_cnode.login, (void*)name, length);
                
                // Create child node
                if(createChildNode(context_parent_node_addr, &temp_cnode) == RETURN_OK)
                {
                    selected_login_child_node_addr = searchForLoginInGivenParent(context_parent_node_addr, name, length);
                    selected_login_flag = TRUE;
                    ret_val = RETURN_OK;
                }
            }
        }
    }
    
    guiGetBackToCurrentScreen();
    return ret_val;
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
        readParentNode(&temp_pnode, context_parent_node_addr);
        
        // Read child node
        readChildNode(&temp_cnode, selected_login_child_node_addr);
        
        // Copy the password and put random bytes after the final 0
        memcpy((void*)temp_cnode.password, (void*)password, length);
        fillArrayWithRandomBytes(temp_cnode.password + length, NODE_CHILD_SIZE_OF_PASSWORD - length);
        
        // Prepare password changing approval text
        conf_text.lines[0] = readStoredStringToBuffer(ID_STRING_CHANGEPASSFOR);
        conf_text.lines[1] = (char*)temp_cnode.login;
        conf_text.lines[2] = readStoredStringToBuffer(ID_STRING_ON);
        conf_text.lines[3] = (char*)temp_pnode.service;
        
        // Ask for password changing approval
        if (guiAskForConfirmation(4, &conf_text) == RETURN_OK)
        {
            // Get back to current screen
            guiGetBackToCurrentScreen();
            
            // Encrypt the password
            encryptTempCNodePasswordAndClearCTVFlag();
            
            // Update child node to store password
            if (updateChildNode(&temp_pnode, &temp_cnode, context_parent_node_addr, selected_login_child_node_addr) != RETURN_OK)
            {
                return RETURN_NOK;
            }
            
            return RETURN_OK;
        }
        else
        {
            // Get back to current screen
            guiGetBackToCurrentScreen();
            
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
            readChildNode(&temp_cnode, selected_login_child_node_addr);
            
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

/*! \fn     favoritePickingLogic(void)
*   \brief  Logic for picking a favorite's credentials
*/
void favoritePickingLogic(void)
{
    uint16_t pickedChild;
    
    pickedChild = favoriteSelectionScreen(&temp_pnode, &temp_cnode);
    
    // If the user picked a credential set
    if (pickedChild != NODE_ADDR_NULL)
    {
        // Read child node
        readChildNode(&temp_cnode, pickedChild);
        
        // Ask the user if he wants to output the login
        if (guiAskForConfirmation(1, (confirmationText_t*)readStoredStringToBuffer(ID_STRING_ENTERLOGINQ)) == RETURN_OK)
        {
            usbKeybPutStr((char*)temp_cnode.login);
        }
        
        // Ask the user if he wants to output the login
        if (guiAskForConfirmation(1, (confirmationText_t*)readStoredStringToBuffer(ID_STRING_ENTERPASSQ)) == RETURN_OK)
        {
            decryptTempCNodePasswordAndClearCTVFlag();
            usbKeybPutStr((char*)temp_cnode.password);
        }
    }
}