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
#include "usb_cmd_parser.h"
#include "timer_manager.h"
#include "hid_defines.h"
#include "aes256_ctr.h"
#include "node_mgmt.h"
#include "flash_mem.h"
#include "defines.h"
#include "delays.h"
#include "usb.h"
#include "rng.h"

// Know if the smart card is inserted and unlocked
uint8_t smartcard_inserted_unlocked = FALSE;
// Current nonce for our AES256 encryption
uint8_t current_nonce[AES256_CTR_LENGTH];
// Selected login child node address
uint16_t selected_login_child_node_addr;
// Selected login flag (the plugin selected a login)
uint8_t selected_login_flag = FALSE;
// Data context valid flag (we know the current data service)
uint8_t data_context_valid_flag = FALSE;
// Context valid flag (eg we know the current service / website)
uint8_t context_valid_flag = FALSE;
// Currently adding data flag
uint8_t current_adding_data_flag = FALSE;
// Counter for our current data node written bytes
uint8_t currently_adding_data_cntr = 0;
// Counter for our current offset when reading data
uint8_t currently_reading_data_cntr = 0;
// Flag to know if we are writing the first block of data
uint8_t currently_writing_first_block = FALSE;
// Address of the next data node for reading
uint16_t next_data_node_addr = 0;
// Current CTR value used for data node decryption
uint8_t dataNodeCtrVal[3];
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
// Data node ptr;
dNode* temp_dnode_ptr = (dNode*)&temp_cnode;


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
    leaveMemoryManagementMode();
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

/*! \fn     searchForServiceName(uint8_t* name, uint8_t mode)
*   \brief  Find a given service name
*   \param  name    Name of the service / website
*   \param  mode    Mode of compare (see service_compare_mode_t)
*   \param  type    Type of context (data or credential)
*   \return Address of the found node, NODE_ADDR_NULL otherwise
*/
uint16_t searchForServiceName(uint8_t* name, uint8_t mode, uint8_t type)
{
    uint16_t next_node_addr;
    int8_t compare_result;
    
    // If it is of credential type, use the LUT to accelerate things
    if (type == SERVICE_CRED_TYPE)
    {
        next_node_addr = getParentNodeForLetter(name[0]);
    }
    else
    {
        next_node_addr = getStartingDataParentAddress();
    }
    
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
            if (mode == COMPARE_MODE_MATCH)
            {
                compare_result = strncmp((char*)name, (char*)temp_pnode.service, NODE_CHILD_SIZE_OF_LOGIN);
                
                if (compare_result == 0)
                {
                    // Result found
                    return next_node_addr;
                } 
                else if (compare_result < 0)
                {
                    // Nodes are alphabetically sorted, escape if we went over
                    return NODE_ADDR_NULL;
                }
            }
            else if ((mode == COMPARE_MODE_COMPARE) && (strncmp((char*)name, (char*)temp_pnode.service, NODE_CHILD_SIZE_OF_LOGIN) < 0))
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
*   \return Address of the found node, NODE_ADDR_NULL otherwise
*/
uint16_t searchForLoginInGivenParent(uint16_t parent_addr, uint8_t* name)
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
        if (strncmp((char*)temp_cnode.login, (char*)name, NODE_CHILD_SIZE_OF_LOGIN) == 0)
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
    if (memcmp(temp_buffer, nextCtrVal, USER_CTR_SIZE) == 0)
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

/*! \fn     decryptBlockOfDataAndClearCTVFlag(uint8_t* data, uint8_t* ctr)
*   \brief  Decrypt a block of data, clear credential_timer_valid
*   \param  data    Data to be decrypted
*   \param  ctr     Ctr value for the data
*/
void decrypt32bBlockOfDataAndClearCTVFlag(uint8_t* data, uint8_t* ctr)
{
    uint8_t temp_buffer[AES256_CTR_LENGTH];
    
    // Preventing side channel attacks: only send the password after a given amount of time
    activateTimer(TIMER_CREDENTIALS, AES_ENCR_DECR_TIMER_VAL);
    
    // AES decryption: xor our nonce with the ctr value, set the result, then decrypt
    memcpy((void*)temp_buffer, (void*)current_nonce, AES256_CTR_LENGTH);
    aesXorVectors(temp_buffer + (AES256_CTR_LENGTH-USER_CTR_SIZE), ctr, USER_CTR_SIZE);
    aes256CtrSetIv(&aesctx, temp_buffer, AES256_CTR_LENGTH);
    aes256CtrDecrypt(&aesctx, data, AES_ROUTINE_ENC_SIZE);
    
    // Wait for credential timer to fire (we wanted to clear credential_timer_valid flag anyway)
    while (hasTimerExpired(TIMER_CREDENTIALS, FALSE) == TIMER_RUNNING);
}

/*! \fn     encrypt32bBlockOfDataAndClearCTVFlag(uint8_t* data, uint8_t* ctr)
*   \brief  Encrypt a block of data, clear credential_timer_valid
*   \param  data    Data to be decrypted
*   \param  ctr     Pointer to where to store the ctr
*/
void encrypt32bBlockOfDataAndClearCTVFlag(uint8_t* data, uint8_t* ctr)
{
    uint8_t temp_buffer[AES256_CTR_LENGTH];
    
    // Preventing side channel attacks: only send the return after a given amount of time
    activateTimer(TIMER_CREDENTIALS, AES_ENCR_DECR_TIMER_VAL);

    // AES encryption: xor our nonce with the next available ctr value, set the result as IV, encrypt, increment our next available ctr value
    ctrPreEncryptionTasks();
    memcpy((void*)temp_buffer, (void*)current_nonce, AES256_CTR_LENGTH);
    aesXorVectors(temp_buffer + (AES256_CTR_LENGTH-USER_CTR_SIZE), nextCtrVal, USER_CTR_SIZE);
    aes256CtrSetIv(&aesctx, temp_buffer, AES256_CTR_LENGTH);
    aes256CtrEncrypt(&aesctx, data, AES_ROUTINE_ENC_SIZE);
    memcpy((void*)ctr, (void*)nextCtrVal, USER_CTR_SIZE);
    ctrPostEncryptionTasks();

    // Wait for credential timer to fire (we wanted to clear credential_timer_valid flag anyway)
    while (hasTimerExpired(TIMER_CREDENTIALS, FALSE) == TIMER_RUNNING);
}

/*! \fn     setCurrentContext(uint8_t* name, uint8_t length)
*   \brief  Set our current context
*   \param  name    Name of the desired service / website
*   \param  type    Type of context (data or credential)
*   \return If we found the context
*/
RET_TYPE setCurrentContext(uint8_t* name, uint8_t type)
{
    // Look for name inside our flash
    context_parent_node_addr = searchForServiceName(name, COMPARE_MODE_MATCH, type);
    
    // Clear all flags
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        context_valid_flag = FALSE;
        selected_login_flag = FALSE;
        data_context_valid_flag = FALSE;
        current_adding_data_flag = FALSE;
        activateTimer(TIMER_CREDENTIALS, 0);
        currently_writing_first_block = FALSE;
    }
    
    // Do we know this context ?
    if ((context_parent_node_addr != NODE_ADDR_NULL) && (smartcard_inserted_unlocked == TRUE))
    {
        if (type == SERVICE_CRED_TYPE)
        {
            context_valid_flag = TRUE;
        } 
        else
        {
            data_context_valid_flag = TRUE;
            // Load the parent node in memory
            readParentNode(&temp_pnode, context_parent_node_addr);
        }
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
*   \param  type    Type of context (data or credential)
*   \return If we added the context
*/
RET_TYPE addNewContext(uint8_t* name, uint8_t length, uint8_t type)
{
    RET_TYPE ret_val = RETURN_NOK;
    
    // Check if the context doesn't already exist
    if ((smartcard_inserted_unlocked == FALSE) || (searchForServiceName(name, COMPARE_MODE_MATCH, type) != NODE_ADDR_NULL))
    {
        return RETURN_NOK;
    }
    
    // Prepare domain approval screen
    if (type == SERVICE_CRED_TYPE)
    {
        conf_text.lines[0] = readStoredStringToBuffer(ID_STRING_CONF_NEWCREDS);
    }
    else
    {
        conf_text.lines[0] = readStoredStringToBuffer(ID_STRING_CONF_NEWDATA);
    }
    conf_text.lines[1] = (char*)name;
    
    // Ask for user approval, flash screen
    if(guiAskForConfirmation(0xF0 | 2, &conf_text) == RETURN_OK)
    {
        // Display processing screen
        guiDisplayProcessingScreen();
        
        // Copy service name inside the parent node
        memcpy((void*)temp_pnode.service, (void*)name, length);
        
        // Create parent node for service
        if (createParentNode(&temp_pnode, type) == RETURN_OK)
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
            // Read context parent node
            readParentNode(&temp_pnode, context_parent_node_addr);
            
            // Check it actually has a child!
            if (temp_pnode.nextChildAddress == NODE_ADDR_NULL)
            {
                return RETURN_NOK;
            }
            
            // Ask the user to a pick a child
            selected_login_child_node_addr = guiAskForLoginSelect(&temp_pnode, &temp_cnode, context_parent_node_addr, FALSE);
            guiGetBackToCurrentScreen();
            
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
        decrypt32bBlockOfDataAndClearCTVFlag(temp_cnode.password, temp_cnode.ctr);
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
        selected_login_child_node_addr = searchForLoginInGivenParent(context_parent_node_addr, name);
        
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
                
                // Set temp cnode to zeroes, generate random password, store the node in flash
                memset((void*)&temp_cnode, 0x00, NODE_SIZE);
                fillArrayWithRandomBytes(temp_cnode.password, NODE_CHILD_SIZE_OF_PASSWORD - 1);
                temp_cnode.password[NODE_CHILD_SIZE_OF_PASSWORD-1] = 0;
                encrypt32bBlockOfDataAndClearCTVFlag(temp_cnode.password, temp_cnode.ctr);
                memcpy((void*)temp_cnode.login, (void*)name, length);
                
                // Add "created by plugin" message in the description field
                strcpy((char*)temp_cnode.description, readStoredStringToBuffer(ID_STRING_CREATEDBYPLUG));
                
                // Create child node
                if(createChildNode(context_parent_node_addr, &temp_cnode) == RETURN_OK)
                {
                    selected_login_child_node_addr = searchForLoginInGivenParent(context_parent_node_addr, name);
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
            encrypt32bBlockOfDataAndClearCTVFlag(temp_cnode.password, temp_cnode.ctr);
            
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

/*! \fn     addDataForDataContext(uint8_t* data, uint8_t length)
*   \brief  Add 32 bytes of data to our current data parent
*   \param  data                Block of data to add
*   \param  last_packet_flag    Flag to know if it is our last packet
*   \return Operation success or not
*/
RET_TYPE addDataForDataContext(uint8_t* data, uint8_t last_packet_flag)
{
    uint8_t temp_ctr[3];
    
    if (data_context_valid_flag == FALSE)
    {
        // Login not set
        return RETURN_NOK;
    }
    else
    {
        // Check if we haven't already setup a child data node, parent node is already in our memory when flag is set
        if ((temp_pnode.nextChildAddress == NODE_ADDR_NULL) && (current_adding_data_flag == FALSE))
        {
            // No child... ask for permission
            // Prepare data adding approval text
            conf_text.lines[0] = readStoredStringToBuffer(ID_STRING_ADD_DATA_FOR);
            conf_text.lines[1] = (char*)temp_pnode.service;
            
            // Ask for data adding approval
            if (guiAskForConfirmation(2, &conf_text) == RETURN_OK)
            {
                memset((void*)temp_dnode_ptr, 0, NODE_SIZE);
                currently_writing_first_block = TRUE;
                currently_reading_data_cntr = 0;
                current_adding_data_flag = TRUE;
                currently_adding_data_cntr = 0;
            }            
            guiGetBackToCurrentScreen(); 
        }
        
        // Check that we approved data adding, that we're not adding too much data in the node and that the encrypt length is a multiple of 16 if we are not writing the last block
        if (current_adding_data_flag == FALSE)
        {
            // If the service has a data child and that we're currently not adding data, refuse it (we can't append data to an already existing data set)
            return RETURN_NOK;
        }
        else
        {
            // Copy data in our data node at the right spot
            memcpy(&temp_dnode_ptr->data[currently_adding_data_cntr], data, 32);
            // Encrypt the data
            encrypt32bBlockOfDataAndClearCTVFlag(&temp_dnode_ptr->data[currently_adding_data_cntr], temp_ctr);
            // If we write the first block of data, update ctr value in parent node
            if (currently_adding_data_cntr == 0)
            {
                memcpy((void*)temp_pnode.startDataCtr, temp_ctr, 3);
            }
            
            // Check if we need to write the node in flash
            currently_adding_data_cntr += 32;
            if ((currently_adding_data_cntr == DATA_NODE_DATA_LENGTH) || (last_packet_flag != FALSE))
            {
                // Last 8 bits of the flags is the number of bytes stored
                temp_dnode_ptr->flags = currently_adding_data_cntr;
                // Write the node, reset the counter, erase data node
                if (writeNewDataNode(context_parent_node_addr, &temp_pnode, temp_dnode_ptr, currently_writing_first_block, last_packet_flag) != RETURN_OK)
                {
                    return RETURN_NOK;
                }
                memset((void*)temp_dnode_ptr, 0, NODE_SIZE);
                currently_writing_first_block = FALSE;
                currently_adding_data_cntr = 0;
            }
            
            // If we are writing the last block, set the flags
            if (last_packet_flag != FALSE)
            {
                // Because of the if above
                temp_pnode.nextChildAddress = NODE_ADDR_NULL+1;
                current_adding_data_flag = FALSE;
            }
            
            return RETURN_OK;
        }
    }
}


/*! \fn     get32BytesDataForCurrentService(uint8_t* buffer, uint8_t* bytes_written)
*   \brief  Get a 32bytes block of data
*   \param  buffer          Buffer where to store the data
*   \return Success status
*/
RET_TYPE get32BytesDataForCurrentService(uint8_t* buffer)
{
        if (data_context_valid_flag == FALSE)
        {
            // Context invalid
            return RETURN_NOK;
        } 
        else
        {            
            // Credential timer off, ask for user to approve
            if (hasTimerExpired(TIMER_CREDENTIALS, FALSE) == TIMER_EXPIRED)
            {
                // Read current parent node, extract child addr and ctr value
                readParentNode(&temp_pnode, context_parent_node_addr);
                memcpy(dataNodeCtrVal, temp_pnode.startDataCtr, 3);
                next_data_node_addr = temp_pnode.nextChildAddress;
                
                // No child... ask for permission
                // Prepare data adding approval text
                conf_text.lines[0] = readStoredStringToBuffer(ID_STRING_GET_DATA_FOR);
                conf_text.lines[1] = (char*)temp_pnode.service;
                
                // Ask for data adding approval
                if (guiAskForConfirmation(2, &conf_text) == RETURN_OK)
                {
                    activateTimer(TIMER_CREDENTIALS, CREDENTIAL_TIMER_VALIDITY);
                }
                guiGetBackToCurrentScreen(); 
            }
            if (hasTimerExpired(TIMER_CREDENTIALS, FALSE) == TIMER_RUNNING)
            {
                // If currently_reading_data_cntr is at 0 it means we need to read new node
                if (currently_reading_data_cntr == 0)
                {
                    // If we don't have a next value...
                    if (next_data_node_addr == NODE_ADDR_NULL)
                    {
                        activateTimer(TIMER_CREDENTIALS, 0);
                        return RETURN_NOK;
                    }
                    else
                    {
                        readNode((gNode*)temp_dnode_ptr, next_data_node_addr);
                        next_data_node_addr = temp_dnode_ptr->nextDataAddress; 
                        
                        // Check that we are actually reading something valid...
                        if(validBitFromFlags(temp_dnode_ptr->flags) == NODE_VBIT_INVALID)
                        {
                            return RETURN_NOK;
                        }                  
                    }
                }
                
                // Call the password decryption function, which also clears the credential_timer_valid flag
                decrypt32bBlockOfDataAndClearCTVFlag(&temp_dnode_ptr->data[currently_reading_data_cntr], dataNodeCtrVal);
                activateTimer(TIMER_CREDENTIALS, CREDENTIAL_TIMER_VALIDITY);
                // Increment ctr value
                aesIncrementCtr(dataNodeCtrVal, USER_CTR_SIZE);
                aesIncrementCtr(dataNodeCtrVal, USER_CTR_SIZE);                
                // Copy in our buffer the data
                memcpy(buffer, (void*)&temp_dnode_ptr->data[currently_reading_data_cntr], 32);
                                
                // Increment our counter
                currently_reading_data_cntr += 32;
                if (currently_reading_data_cntr >= (temp_dnode_ptr->flags & 0x00FF))
                {
                    currently_reading_data_cntr = 0;
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
*   \return Operation success or not (see pass_check_return_t)
*/
RET_TYPE checkPasswordForContext(uint8_t* password)
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
            decrypt32bBlockOfDataAndClearCTVFlag(temp_cnode.password, temp_cnode.ctr);
            
            if (strncmp((char*)temp_cnode.password, (char*)password, NODE_CHILD_SIZE_OF_PASSWORD) == 0)
            {
                memset((void*)temp_cnode.password, 0x00, NODE_CHILD_SIZE_OF_PASSWORD);
                return RETURN_PASS_CHECK_OK;
            }
            else
            {
                memset((void*)temp_cnode.password, 0x00, NODE_CHILD_SIZE_OF_PASSWORD);
                activateTimer(TIMER_PASS_CHECK, CHECK_PASSWORD_TIMER_VAL);
                return RETURN_PASS_CHECK_NOK;
            }
        }
    }
}

/*! \fn     askUserForLoginAndPasswordKeybOutput(uint16_t child_address, char* service_name)
*   \brief  Ask the user to enter the login password of a given child
*   \param  child_address   Address of the child
*   \param  service_name    Service name
*/
void askUserForLoginAndPasswordKeybOutput(uint16_t child_address, char* service_name)
{    
    confirmationText_t temp_conf_text;
    
    // If the user picked a credential set
    if (child_address != NODE_ADDR_NULL)
    {
        // Read child node
        readChildNode(&temp_cnode, child_address);
        temp_conf_text.lines[0] = service_name;
        
        // If login isn't empty, ask the user if he wants to output the login
        if (temp_cnode.login[0] != 0)
        {
            // Check if we're connected through USB
            if (isUsbConfigured())
            {
                temp_conf_text.lines[1] = readStoredStringToBuffer(ID_STRING_ENTERLOGINQ);
                if (guiAskForConfirmation(2, &temp_conf_text) == RETURN_OK)
                {
                    usbKeybPutStr((char*)temp_cnode.login);
                    usbKeyboardPress(KEY_TAB, 0);
                }
            } 
            else
            {
                temp_conf_text.lines[1] = readStoredStringToBuffer(ID_STRING_SHOW_LOGINQ);
                if (guiAskForConfirmation(2, &temp_conf_text) == RETURN_OK)
                {
                    guiDisplayLoginOrPasswordOnScreen((char*)temp_cnode.login);
                }
            }
        }
        
        decrypt32bBlockOfDataAndClearCTVFlag(temp_cnode.password, temp_cnode.ctr);
        // Ask the user if he wants to output the password
        if (isUsbConfigured())
        {
            temp_conf_text.lines[1] = readStoredStringToBuffer(ID_STRING_ENTERPASSQ);
            if (guiAskForConfirmation(2, &temp_conf_text) == RETURN_OK)
            {
                usbKeybPutStr((char*)temp_cnode.password);
            }
        }
        else
        {
            temp_conf_text.lines[1] = readStoredStringToBuffer(ID_STRING_SHOW_PASSQ);
            if (guiAskForConfirmation(2, &temp_conf_text) == RETURN_OK)
            {
                guiDisplayLoginOrPasswordOnScreen((char*)temp_cnode.password);
            }
        }
    }    
}

/*! \fn     favoritePickingLogic(void)
*   \brief  Logic for picking a favorite's credentials
*/
void favoritePickingLogic(void)
{
    // favoriteSelectionScreen loads the chosen parent node in memory before exciting
    askUserForLoginAndPasswordKeybOutput(favoriteSelectionScreen(&temp_pnode, &temp_cnode), (char*)temp_pnode.service);
}

/*! \fn     loginSelectLogic(void)
*   \brief  Logic for finding a given login
*/
void loginSelectLogic(void)
{
    askUserForLoginAndPasswordKeybOutput(guiAskForLoginSelect(&temp_pnode, &temp_cnode, loginSelectionScreen(), TRUE), (char*)temp_pnode.service);
}