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
#include "gui_basic_functions.h"
#include "logic_aes_and_comms.h"
#include "usb_cmd_parser.h"
#include "timer_manager.h"
#include "logic_eeprom.h"
#include "hid_defines.h"
#include "mini_inputs.h"
#include "aes256_ctr.h"
#include "node_mgmt.h"
#include "flash_mem.h"
#include "oledmini.h"
#include "defines.h"
#include "delays.h"
#include "utils.h"
#include "usb.h"
#include "rng.h"

// Know if the smart card is inserted and unlocked
volatile uint8_t smartcard_inserted_unlocked = FALSE;
// Current nonce for our AES256 encryption
uint8_t current_nonce[AES256_CTR_LENGTH];
// Selected login child node address
uint16_t selected_login_child_node_addr;
// Selected login flag (the plugin selected a login)
volatile uint8_t selected_login_flag = FALSE;
// Context valid flag (service / website selected)
volatile uint8_t context_valid_flag = FALSE;
// Login just added flag
volatile uint8_t login_just_added_flag = FALSE;
// Data context valid flag (we know the current data service)
volatile uint8_t data_context_valid_flag = FALSE;
// Currently adding data flag
volatile uint8_t current_adding_data_flag = FALSE;
// Counter for our current data node written bytes
uint8_t currently_adding_data_cntr = 0;
// Counter for our current offset when reading data
uint8_t currently_reading_data_cntr = 0;
// Flag to know if we are writing the first block of data
volatile uint8_t currently_writing_first_block = FALSE;
// Address of the next data node for reading
uint16_t next_data_node_addr = 0;
// Current CTR value used for data node decryption
uint8_t dataNodeCtrVal[USER_CTR_SIZE];
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

#ifdef ENABLE_CREDENTIAL_MANAGEMENT
// management mode state for login selection state machine
uint8_t ondevice_cred_mgmt_action = ONDEVICE_CRED_MGMT_ACTION_NONE;
#endif

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
    login_just_added_flag = FALSE;
    leaveMemoryManagementMode();
    data_context_valid_flag = FALSE;
    current_adding_data_flag = FALSE;
    activateTimer(TIMER_CREDENTIALS, 0);
    smartcard_inserted_unlocked = FALSE;
    currently_writing_first_block = FALSE;
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
        
        if(mode == COMPARE_MODE_COMPARE)
        {
            // We didn't find the service, return first node
            return getStartingParentAddress();            
        }
        else
        {
            return NODE_ADDR_NULL;
        }
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
static inline void ctrPostEncryptionTasks(void)
{
    aesIncrementCtr(nextCtrVal, USER_CTR_SIZE);
    aesIncrementCtr(nextCtrVal, USER_CTR_SIZE);
}

#ifdef MINI_VERSION
/*! \fn     encryptOneAesBlockWithKeyEcb(uint8_t* aes_key, uint8_t* data)
*   \brief  Encrypt a block of data using a given AES key in ECB mode
*   \param  aes_key         AES key
*   \param  data            Data to encrypt, one AES block size long (128bits)
*   \note   aes_key is emptied after use!
*   \note   This function uses the AES context of the current user, so DO NOT call it when a user is logged in!
*/
void encryptOneAesBlockWithKeyEcb(uint8_t* aes_key, uint8_t* data)
{
    // Initialize AES context & encrypt data
    activateTimer(TIMER_CREDENTIALS, AES_ENCR_DECR_TIMER_VAL);
    aes256_init_ecb(&(aesctx.aesCtx), aes_key);
    aes256_encrypt_ecb(&(aesctx.aesCtx), data);
    while (hasTimerExpired(TIMER_CREDENTIALS, FALSE) == TIMER_RUNNING);

    // Delete vars
    memset((void*)&aesctx, 0x00, sizeof(aesctx));
    memset((void*)aes_key, 0x00, AES_KEY_LENGTH/8);    
}

/*! \fn     computeAndDisplayBlockSizeEncryptionResult(uint8_t* aes_key, uint8_t* data, uint8_t stringId)
*   \brief  Encrypt a block of data using a given aes key, display it on the screen and wait for user action
*   \param  aes_key     AES key
*   \param  data        Data to encrypt, one AES block size long (128bits) untouched by the routine
*   \param  stringId    String ID to display on the screen
*   \note   aes_key is emptied after use!
*   \note   This function uses the AES context of the current user, so DO NOT call it when a user is logged in!
*/
void computeAndDisplayBlockSizeEncryptionResult(uint8_t* aes_key, uint8_t* data, uint8_t stringId)
{
    // Buffer to store a copy of the data to encrypt
    uint8_t data_copy[AES_BLOCK_SIZE/8];

    // Get the text to display on the screen
    miniOledClearFrameBuffer();
    miniOledPutCenteredString(THREE_LINE_TEXT_FIRST_POS, readStoredStringToBuffer(stringId));

    // Encrypt data
    memcpy((void*)data_copy, (void*)data, sizeof(data_copy));
    encryptOneAesBlockWithKeyEcb(aes_key, data_copy);

    // Format and display hash
    for (uint8_t i = 0; i < AES256_CTR_LENGTH / 2; i++)
    {
        hexachar_to_string((char)data_copy[i], (char*)&textBuffer1[i*2]);
        hexachar_to_string((char)data_copy[i+(AES_BLOCK_SIZE/8/2)], (char*)&textBuffer2[i*2]);
    }
    miniOledPutCenteredString(THREE_LINE_TEXT_SECOND_POS, (char*)textBuffer1);
    miniOledPutCenteredString(THREE_LINE_TEXT_THIRD_POS, (char*)textBuffer2);
    miniOledFlushEntireBufferToDisplay();

    // Wait for action before next screen
    miniGetWheelAction(TRUE, FALSE);
}
#endif

/*! \fn     decrypt32bBlockOfDataAndClearCTVFlag(uint8_t* data, uint8_t* ctr)
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

/*! \fn     setCurrentContext(uint8_t* name, uint8_t type)
*   \brief  Set our current context
*   \param  name    Name of the desired service / website
*   \param  type    Type of context (data or credential)
*   \return If we found the context
*/
RET_TYPE setCurrentContext(uint8_t* name, uint8_t type)
{
    // Look for name inside our flash
    context_parent_node_addr = searchForServiceName(name, COMPARE_MODE_MATCH, type);
    
    // Clear all flags: no need for an ATOMIC_BLOCK as the clearSmartCardInsertedUnlocked() interrupt sets the following variables to the same values
    context_valid_flag = FALSE;
    selected_login_flag = FALSE;
    login_just_added_flag = FALSE;
    data_context_valid_flag = FALSE;
    current_adding_data_flag = FALSE;
    activateTimer(TIMER_CREDENTIALS, 0);
    currently_writing_first_block = FALSE;
    
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
    
    #ifdef ENABLE_CREDENTIAL_MANAGEMENT
    /* disable menu animation when managing credentials as we have more questions */
    if(ondevice_cred_mgmt_action == ONDEVICE_CRED_MGMT_ACTION_NONE)
    {
        guiGetBackToCurrentScreen();
    }
    #else
    guiGetBackToCurrentScreen();
    #endif
    return ret_val;
}

/*! \fn     getLoginForContext(uint8_t* buffer)
*   \brief  Get login for current context
*   \param  buffer  Buffer to store the login
*   \note   Firmware V1.1 allows the possibility to specify the login we want, buffer therefore contains the USB message as is!
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
	    // Clear current flags: no need for an ATOMIC_BLOCK as the clearSmartCardInsertedUnlocked() interrupt sets the following variables to the same values
		selected_login_flag = FALSE;
		login_just_added_flag = FALSE;
		
        // Credential timer off, ask for user to choose: implemented to avoid trapping the user with 2 successive prompts for a get login
        if (hasTimerExpired(TIMER_CREDENTIALS, FALSE) == TIMER_EXPIRED)
        {
            // Read context parent node
            readParentNode(&temp_pnode, context_parent_node_addr);
            
            // Check it actually has a child!
            if (temp_pnode.nextChildAddress == NODE_ADDR_NULL)
            {
                return RETURN_NOK;
            }
            
            // See if a username has already been specified
            if (buffer[HID_LEN_FIELD] != 0)
            {
                // Check that the payload length is correct
                if (checkTextField((uint8_t*)(buffer + HID_DATA_START), buffer[HID_LEN_FIELD], NODE_CHILD_SIZE_OF_LOGIN) == RETURN_NOK)
                {
                    return RETURN_NOK;
                }
                else
                {
                    // Payload length correct, check that the specified login actually exists...
                    selected_login_child_node_addr = searchForLoginInGivenParent(context_parent_node_addr, (uint8_t*)(buffer + HID_DATA_START));
                    
                    // Requested login exists, ask for acknowledgment from user...
                    if (selected_login_child_node_addr != NODE_ADDR_NULL)
                    {
                        // Prepare confirmation screen
                        #if defined(HARDWARE_OLIVIER_V1)
                        conf_text.lines[0] = readStoredStringToBuffer(ID_STRING_SEND_PASS_FOR);
                        conf_text.lines[1] = (char*)(buffer + HID_DATA_START);
                        conf_text.lines[2] = readStoredStringToBuffer(ID_STRING_ON);
                        conf_text.lines[3] = (char*)temp_pnode.service;
                        #elif defined(MINI_VERSION)
                        conf_text.lines[0] = (char*)temp_pnode.service;
                        conf_text.lines[1] = readStoredStringToBuffer(ID_STRING_SEND_PASS_FOR);
                        conf_text.lines[2] = (char*)(buffer + HID_DATA_START);
                        #endif
                        
                        // If doesn't exist, ask user for confirmation to add to flash
                        #if defined(HARDWARE_OLIVIER_V1)
                        if (guiAskForConfirmation(4, &conf_text) == RETURN_OK)
                        #elif defined(MINI_VERSION)
                        if (guiAskForConfirmation(3, &conf_text) == RETURN_OK)
                        #endif
                        {
                            selected_login_flag = TRUE;
                            guiGetBackToCurrentScreen();
                            activateTimer(TIMER_CREDENTIALS, CREDENTIAL_TIMER_VALIDITY);
                        }
                        else
                        {
                            guiGetBackToCurrentScreen();
                        }
                    }
                }
            } 
            else
            {
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
        }
        
        // If the user just approved!
        if ((hasTimerExpired(TIMER_CREDENTIALS, FALSE) == TIMER_RUNNING) && (selected_login_flag == TRUE))
        {
            // Read selected child node, guaranteed to be null terminated by readchildnode function
            // Note: the buffer here is 64 bytes, temp_cnode.login is null terminated at the 63th byte worst case
            readChildNode(&temp_cnode, selected_login_child_node_addr);
            strcpy((char*)buffer, (char*)temp_cnode.login);
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
        temp_cnode.password[NODE_CHILD_SIZE_OF_PASSWORD-1] = 0;
        strcpy((char*)buffer, (char*)temp_cnode.password);
        memset((void*)temp_cnode.password, 0x00, NODE_CHILD_SIZE_OF_PASSWORD);
        
        // Timer fired, return
        return RETURN_OK;
    }
    else
    {
        return RETURN_NOK;
    }
}

/*! \fn     getDescriptionForContext(void)
*   \brief  Get description for current context
*   \return If description was entered
*/
RET_TYPE getDescriptionForContext(char* buffer)
{
    if ((context_valid_flag == TRUE) && (hasTimerExpired(TIMER_CREDENTIALS, FALSE) == TIMER_RUNNING) && (selected_login_flag == TRUE))
    {
        // Fetch description from selected login and send it over USB
        readChildNode(&temp_cnode, selected_login_child_node_addr);
        
        // Store the description, guaranteed to be null terminated by readchildnode function
        strcpy((char*)buffer, (char*)temp_cnode.description);
        
        // Return
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
        // Clear current flags: no need for an ATOMIC_BLOCK as the clearSmartCardInsertedUnlocked() interrupt sets the following variables to the same values
        selected_login_flag = FALSE;
        login_just_added_flag = FALSE;
        activateTimer(TIMER_CREDENTIALS, 0);
        
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
            #if defined(HARDWARE_OLIVIER_V1)
                conf_text.lines[0] = readStoredStringToBuffer(ID_STRING_ADDUSERNAME);
                conf_text.lines[1] = (char*)name;
                conf_text.lines[2] = readStoredStringToBuffer(ID_STRING_ON);
                conf_text.lines[3] = (char*)temp_pnode.service;
            #elif defined(MINI_VERSION)
                conf_text.lines[0] = (char*)temp_pnode.service;
                conf_text.lines[1] = readStoredStringToBuffer(ID_STRING_ADDUSERNAME);
                conf_text.lines[2] = (char*)name;
            #endif
            
            // If doesn't exist, ask user for confirmation to add to flash
            #if defined(HARDWARE_OLIVIER_V1)
            if (guiAskForConfirmation(4, &conf_text) == RETURN_OK)
            #elif defined(MINI_VERSION)
            if (guiAskForConfirmation(3, &conf_text) == RETURN_OK)
            #endif
            {
                // Display processing screen
                guiDisplayProcessingScreen();
                
                // Set temp cnode to zeroes: we're not setting a random password as a plain text attack would suggest the attacker having control on the device
                // So instead of not setting a password, he'd just put a 31 chars known plaintext...
                memset((void*)&temp_cnode, 0x00, NODE_SIZE);
                encrypt32bBlockOfDataAndClearCTVFlag(temp_cnode.password, temp_cnode.ctr);
                memcpy((void*)temp_cnode.login, (void*)name, length);
                
                // Add "created by plugin" message in the description field
                strcpy((char*)temp_cnode.description, readStoredStringToBuffer(ID_STRING_CREATEDBYPLUG));
                
                // Create child node
                if(createChildNode(context_parent_node_addr, &temp_cnode) == RETURN_OK)
                {
                    selected_login_child_node_addr = searchForLoginInGivenParent(context_parent_node_addr, name);
                    login_just_added_flag = TRUE;
                    selected_login_flag = TRUE;
                    ret_val = RETURN_OK;
                }
            }
        }
    }
    
    #ifdef ENABLE_CREDENTIAL_MANAGEMENT
    /* disable menu animation when managing credentials as we have more questions */
    if(ondevice_cred_mgmt_action == ONDEVICE_CRED_MGMT_ACTION_NONE)
    {
        guiGetBackToCurrentScreen();
    }
    #else
    guiGetBackToCurrentScreen();
    #endif
    return ret_val;
}

/*! \fn     setDescriptionForContext(uint8_t* description)
*   \brief  Set description for current context
*   \param  description String containing the description
*   \param  length      String length
*   \return Operation success or not
*/
RET_TYPE setDescriptionForContext(uint8_t* description)
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

        // If we haven't just added the login, ask for permission
        if (login_just_added_flag == FALSE)
        {
            // Prepare password changing approval text
            #if defined(HARDWARE_OLIVIER_V1)
                conf_text.lines[0] = readStoredStringToBuffer(ID_STRING_CHANGE_DESC_FOR);
                conf_text.lines[1] = (char*)temp_cnode.login;
                conf_text.lines[2] = readStoredStringToBuffer(ID_STRING_ON);
                conf_text.lines[3] = (char*)temp_pnode.service;
            #elif defined(MINI_VERSION)
                conf_text.lines[0] = (char*)temp_pnode.service;
                conf_text.lines[1] = readStoredStringToBuffer(ID_STRING_CHANGE_DESC_FOR);
                conf_text.lines[2] = (char*)temp_cnode.login;
            #endif

            // Ask for password changing approval
            #if defined(HARDWARE_OLIVIER_V1)
            if (guiAskForConfirmation(4, &conf_text) != RETURN_OK)
            #elif defined(MINI_VERSION)
            if (guiAskForConfirmation(3, &conf_text) != RETURN_OK)
            #endif
            {
                guiGetBackToCurrentScreen();
                return RETURN_NOK;
            }

            guiGetBackToCurrentScreen();
        }

        // Update the description field
        updateChildNodeDescription(&temp_cnode, selected_login_child_node_addr, description);

        return RETURN_OK;
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
    // Temp CTR value
    uint8_t temp_ctr[3];

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
        
        // Put random bytes after the final 0
        fillArrayWithRandomBytes(password + length, NODE_CHILD_SIZE_OF_PASSWORD - length);
        
        // Prepare password changing approval text
        #if defined(HARDWARE_OLIVIER_V1)
            conf_text.lines[0] = readStoredStringToBuffer(ID_STRING_CHANGEPASSFOR);
            conf_text.lines[1] = (char*)temp_cnode.login;
            conf_text.lines[2] = readStoredStringToBuffer(ID_STRING_ON);
            conf_text.lines[3] = (char*)temp_pnode.service;
        #elif defined(MINI_VERSION)
            conf_text.lines[0] = (char*)temp_pnode.service;
            conf_text.lines[1] = readStoredStringToBuffer(ID_STRING_CHANGEPASSFOR);
            conf_text.lines[2] = (char*)temp_cnode.login;
        #endif        
        
        // Ask for password changing approval
        #if defined(HARDWARE_OLIVIER_V1)
        if ((login_just_added_flag == TRUE) || (guiAskForConfirmation(4, &conf_text) == RETURN_OK))
        #elif defined(MINI_VERSION)
        if ((login_just_added_flag == TRUE) || (guiAskForConfirmation(3, &conf_text) == RETURN_OK))
        #endif
        {
            // Get back to current screen
            #ifdef ENABLE_CREDENTIAL_MANAGEMENT
            /* disable menu animation when managing credentials as we have more questions */
            if(ondevice_cred_mgmt_action == ONDEVICE_CRED_MGMT_ACTION_NONE)
            {
                guiGetBackToCurrentScreen();
            }
            #else
            if (login_just_added_flag == FALSE)
            {
                guiGetBackToCurrentScreen();
            }
            #endif
            
            // Remove login just added flag
            login_just_added_flag = FALSE;

            // Encrypt the password
            encrypt32bBlockOfDataAndClearCTVFlag(password, temp_ctr);
            
            // Update child node to store password
            if(updateChildNodePassword(&temp_cnode, selected_login_child_node_addr, password, temp_ctr) != RETURN_OK)
            {
                return RETURN_NOK;
            }

            // Inform that the db has changed
            userDBChangedActions(FALSE);
            
            return RETURN_OK;
        }
        else
        {
            // Get back to current screen
            #ifdef ENABLE_CREDENTIAL_MANAGEMENT
            /* disable menu animation when managing credentials as we have more questions */
            if(ondevice_cred_mgmt_action == ONDEVICE_CRED_MGMT_ACTION_NONE)
            {
                guiGetBackToCurrentScreen();
            }
            #else
            guiGetBackToCurrentScreen();
            #endif

            return RETURN_NOK;
        }
    }
}

/*! \fn     addDataForDataContext(uint8_t* data, uint8_t last_packet_flag)
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
        // context not set
        return RETURN_NOK;
    }
    else
    {
        // Check if we haven't already setup a child data node, parent node is already in our memory when flag is set
        if (current_adding_data_flag == FALSE)
        {
            // First packet... ask for permission
            // Prepare data adding approval text
            if (temp_pnode.nextChildAddress != NODE_ADDR_NULL)
            {
                conf_text.lines[0] = readStoredStringToBuffer(ID_STRING_UPDATE_DATA_FOR);
            }
            else
            {
                conf_text.lines[0] = readStoredStringToBuffer(ID_STRING_ADD_DATA_FOR);
            }
            conf_text.lines[1] = (char*)temp_pnode.service;
            
            // Ask for data adding approval
            if (guiAskForConfirmation(2, &conf_text) == RETURN_OK)
            {
                //In case this context already contains data child nodes,
                // delete all of them before adding the first new block of data
                // calling deleteDataNodeChain is safe even if the address is NULL
                deleteDataNodeChain(temp_pnode.nextChildAddress, temp_dnode_ptr);

                memset((void*)temp_dnode_ptr, 0, NODE_SIZE);
                currently_writing_first_block = TRUE;
                currently_reading_data_cntr = 0;
                current_adding_data_flag = TRUE;
                currently_adding_data_cntr = 0;
                userDBChangedActions(TRUE);
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
                    currently_reading_data_cntr = 0;
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
*   \param  RETURN_OK or RETURN_BACK
*/
RET_TYPE askUserForLoginAndPasswordKeybOutput(uint16_t child_address, char* service_name)
{    
    confirmationText_t temp_conf_text;
    
    // If the user picked a credential set
    if (child_address != NODE_ADDR_NULL)
    {
        // Read child node
        readChildNode(&temp_cnode, child_address);
        temp_conf_text.lines[0] = service_name;
        
        #ifdef MINI_VERSION
        while(TRUE)
        {
            // If login isn't empty, ask the user if he wants to output the login
            if (temp_cnode.login[0] != 0)
            {
                // Check if we're connected through USB
                if (isUsbConfigured())
                {
                    temp_conf_text.lines[1] = readStoredStringToBuffer(ID_STRING_ENTERLOGINQ);
                    RET_TYPE confirmation_result = guiAskForConfirmation(2, &temp_conf_text);
                    if (confirmation_result == RETURN_OK)
                    {
                        usbKeybPutStr((char*)temp_cnode.login);
                        if (getMooltipassParameterInEeprom(KEY_AFTER_LOGIN_SEND_BOOL_PARAM) != FALSE)
                        {
                            usbKeyboardPress(getMooltipassParameterInEeprom(KEY_AFTER_LOGIN_SEND_PARAM), 0);
                        }
                    }
                    else if (confirmation_result == RETURN_BACK)
                    {
                        return RETURN_BACK;
                    }
                } 
                else
                {
                    temp_conf_text.lines[1] = readStoredStringToBuffer(ID_STRING_SHOW_LOGINQ);
                    RET_TYPE confirmation_result = guiAskForConfirmation(2, &temp_conf_text);
                    if (confirmation_result == RETURN_OK)
                    {
                        guiDisplayLoginOrPasswordOnScreen((char*)temp_cnode.login);
                    }
                    else if (confirmation_result == RETURN_BACK)
                    {
                        return RETURN_BACK;
                    }
                }
            }
        
            decrypt32bBlockOfDataAndClearCTVFlag(temp_cnode.password, temp_cnode.ctr);
            temp_cnode.password[sizeof(temp_cnode.password)-1] = 0;
            // Ask the user if he wants to output the password
            if (isUsbConfigured())
            {
                temp_conf_text.lines[1] = readStoredStringToBuffer(ID_STRING_ENTERPASSQ);
                RET_TYPE confirmation_result = guiAskForConfirmation(2, &temp_conf_text);
                if (confirmation_result == RETURN_OK)
                {
                    usbKeybPutStr((char*)temp_cnode.password);
                    memset((void*)temp_cnode.password, 0x00, NODE_CHILD_SIZE_OF_PASSWORD);
                    if (getMooltipassParameterInEeprom(KEY_AFTER_PASS_SEND_BOOL_PARAM) != FALSE)
                    {
                        usbKeyboardPress(getMooltipassParameterInEeprom(KEY_AFTER_PASS_SEND_PARAM), 0);
                    }
                    return RETURN_OK;
                }
                else if (confirmation_result == RETURN_BACK)
                {
                    if (temp_cnode.login[0] != 0)
                    {
                        continue;
                    }
                    else
                    {
                        return RETURN_BACK;
                    }
                }
                else
                {
                    return RETURN_NOK;
                }
            }
            else
            {
                temp_conf_text.lines[1] = readStoredStringToBuffer(ID_STRING_SHOW_PASSQ);
                RET_TYPE confirmation_result = guiAskForConfirmation(2, &temp_conf_text);
                if (confirmation_result == RETURN_OK)
                {
                    guiDisplayLoginOrPasswordOnScreen((char*)temp_cnode.password);
                    memset((void*)temp_cnode.password, 0x00, NODE_CHILD_SIZE_OF_PASSWORD);
                    return RETURN_OK;
                }
                else if (confirmation_result == RETURN_BACK)
                {
                    if (temp_cnode.login[0] != 0)
                    {
                        continue;
                    }
                    else
                    {
                        return RETURN_BACK;
                    }
                }
                else
                {
                    return RETURN_NOK;
                }
            }
        }
        #else
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
                    if (getMooltipassParameterInEeprom(KEY_AFTER_LOGIN_SEND_BOOL_PARAM) != FALSE)
                    {
                        usbKeyboardPress(getMooltipassParameterInEeprom(KEY_AFTER_LOGIN_SEND_PARAM), 0);
                    }
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
                memset((void*)temp_cnode.password, 0x00, NODE_CHILD_SIZE_OF_PASSWORD);
                if (getMooltipassParameterInEeprom(KEY_AFTER_PASS_SEND_BOOL_PARAM) != FALSE)
                {
                    usbKeyboardPress(getMooltipassParameterInEeprom(KEY_AFTER_PASS_SEND_PARAM), 0);
                }
            }
        }
        else
        {
            temp_conf_text.lines[1] = readStoredStringToBuffer(ID_STRING_SHOW_PASSQ);
            if (guiAskForConfirmation(2, &temp_conf_text) == RETURN_OK)
            {
                guiDisplayLoginOrPasswordOnScreen((char*)temp_cnode.password);
                memset((void*)temp_cnode.password, 0x00, NODE_CHILD_SIZE_OF_PASSWORD);
            }
        }
        #endif
    }

    return RETURN_OK;
}

/*! \fn     favoritePickingLogic(void)
*   \brief  Logic for picking a favorite's credentials
*/
void favoritePickingLogic(void)
{
    #ifdef MINI_VERSION
        // Special ifdef to allow going back action in the mooltipass mini
        uint16_t chosen_login_addr;

        while (TRUE)
        {
            // favoriteSelectionScreen loads the chosen parent node in memory before exciting
            chosen_login_addr = favoriteSelectionScreen(&temp_pnode, &temp_cnode);

            // No login was chosen
            if (chosen_login_addr == NODE_ADDR_NULL)
            {
                return;
            }

            // Ask the user permission to enter login / password, check for back action
            if (askUserForLoginAndPasswordKeybOutput(chosen_login_addr, (char*)temp_pnode.service) == RETURN_BACK)
            {
                continue;
            }
            else
            {
                return;
            }
        }
    #else
        // favoriteSelectionScreen loads the chosen parent node in memory before exciting
        askUserForLoginAndPasswordKeybOutput(favoriteSelectionScreen(&temp_pnode, &temp_cnode), (char*)temp_pnode.service);
    #endif
}

/*! \fn     loginSelectLogic(void)
*   \brief  Logic for finding a given login
*/
void loginSelectLogic(void)
{
    #ifdef MINI_VERSION
        // Special ifdef to allow going back action in the mooltipass mini
        uint16_t chosen_service_addr;
        uint16_t chosen_login_addr;
        uint8_t state_machine = 0;

        while (TRUE)
        {
            if (state_machine == 0)
            {
                // Ask user to select a service
                chosen_service_addr = loginSelectionScreen();

                // No service was chosen
                if (chosen_service_addr == NODE_ADDR_NULL)
                {
                    return;
                }

                state_machine++;
            }
            else if (state_machine == 1)
            {
                // If there are different logins for this service, ask the user to pick one
                chosen_login_addr = guiAskForLoginSelect(&temp_pnode, &temp_cnode, chosen_service_addr, TRUE);

                // In case the user went back
                if ((chosen_login_addr == NODE_ADDR_NULL) && (miniGetLastReturnedAction() == WHEEL_ACTION_LONG_CLICK))
                {
                    state_machine = 0;
                }
                else
                {
                    state_machine++;
                }
            }
            else if (state_machine == 2)
            {
                // Ask the user permission to enter login / password, check for back action
                if (askUserForLoginAndPasswordKeybOutput(chosen_login_addr, (char*)temp_pnode.service) == RETURN_BACK)
                {
                    // Check if the chosen login node is an only child. Guaranteed to work as askUserForLoginAndPasswordKeybOutput(NODE_ADDR_NULL) returns RETURN_OK
                    readChildNode(&temp_cnode, chosen_login_addr);

                    // If only child, go back to service selection, otherwise go back to login selection
                    if ((temp_cnode.prevChildAddress == NODE_ADDR_NULL) && (temp_cnode.nextChildAddress == NODE_ADDR_NULL))
                    {
                        state_machine = 0;
                    } 
                    else
                    {
                        state_machine = 1;
                    }
                }
                else
                {
                    return;
                }
            }
        }
    #else
        askUserForLoginAndPasswordKeybOutput(guiAskForLoginSelect(&temp_pnode, &temp_cnode, loginSelectionScreen(), TRUE), (char*)temp_pnode.service);
    #endif
}

#ifdef ENABLE_CREDENTIAL_MANAGEMENT
/*! \fn     managementActionPickingLogic(void)
*   \brief  Logic for picking a credential management action
*/
void managementActionPickingLogic(void)
{
    while (TRUE)
    {
        /* display selection screen */
        ondevice_cred_mgmt_action = managementActionSelectionScreen();

        if(ondevice_cred_mgmt_action == ONDEVICE_CRED_MGMT_ACTION_NONE)
        {
            return;
        }

        loginManagementSelectLogic();
    }
}

/*! \fn     askUserToSaveToFlash(pNode *p, cNode *c, uint16_t pAddr, uint16_t cAddr)
*   \brief  Asks the user for applicable charsets
*   \param  p       parent node
*   \param  c       child node
*   \param  pAddr   parent node address
*   \param  cAddr   child node address
*   \return RETURN_OK if update was successful, RETURN_NOK otherwise
*/
RET_TYPE askUserToSaveToFlash(pNode *p, cNode *c, uint16_t pAddr, uint16_t cAddr)
{
    /* prevent saving out of credential management actions, then ask for confirmation */
    if((ondevice_cred_mgmt_action != ONDEVICE_CRED_MGMT_ACTION_NONE) && (guiAskForConfirmation(1, (confirmationText_t*)readStoredStringToBuffer(ID_STRING_MGMT_SAVETOFLASHQ)) == RETURN_OK))
    {
        /* commit changes to external flash */
        if(updateChildNode(p, c, pAddr, cAddr) == RETURN_OK)
        {
            guiDisplayInformationOnScreenAndWait(ID_STRING_MGMT_OPSUCCESS);
            return RETURN_OK;
        }
        else
        {
            /* revert to previous child node state */
            readChildNode(&temp_cnode, cAddr);
            guiDisplayInformationOnScreenAndWait(ID_STRING_MGMT_OPFAILURE);
            return RETURN_NOK;
        }
    }
    else
    {
        return RETURN_NOK;
    }
}

/*! \fn     selectCharset(void)
*   \brief  Asks the user for applicable charsets
*
*   Asks the user to select allowed charsets.
*   \param  original_flags   original flag values (childNode.flags) to modify and return.
*   \return Charset bitfield to be stored in RESERVED bitfield of credential child node.
*/
uint16_t askUserToSelectCharset(uint16_t original_flags)
{
    uint16_t flags = original_flags & ~NODE_F_CHILD_USERFLAGS_MASK; /* reset allowed charset */

    if(guiAskForConfirmation(1, (confirmationText_t*)readStoredStringToBuffer(ID_STRING_MGMT_EDIT_CHARSETQ)) == RETURN_OK)
    {
        do
        {
            /* Enable charset? A-Z */
            if(guiAskForConfirmation(1, (confirmationText_t*)readStoredStringToBuffer(ID_STRING_MGMT_ENABLE_ALPHA_UPPERQ)) == RETURN_OK)
            {
                flags |= CHARSET_BIT_ALPHA_UPPER;
            }

            /* Enable charset? a-z */
            if(guiAskForConfirmation(1, (confirmationText_t*)readStoredStringToBuffer(ID_STRING_MGMT_ENABLE_ALPHA_LOWERQ)) == RETURN_OK)
            {
                flags |= CHARSET_BIT_ALPHA_LOWER;
            }

            /* Enable charset? 0-9 */
            if(guiAskForConfirmation(1, (confirmationText_t*)readStoredStringToBuffer(ID_STRING_MGMT_ENABLE_NUMQ)) == RETURN_OK)
            {
                flags |= CHARSET_BIT_NUM;
            }

            /* Enable charset? specials: ?!,.:;*+-=/ */
            conf_text.lines[0] = readStoredStringToBuffer(ID_STRING_MGMT_ENABLE_SPECIALSQ);
            conf_text.lines[1] = readStoredStringToBuffer(ID_STRING_MGMT_CHARSET_SPECIALS1);
            if(guiAskForConfirmation(2, &conf_text) == RETURN_OK)
            {
                flags |= CHARSET_BIT_SPECIALS1;
            }

            /* Enable charset? specials: ()[]{}<> */
            conf_text.lines[0] = readStoredStringToBuffer(ID_STRING_MGMT_ENABLE_SPECIALSQ);
            conf_text.lines[1] = readStoredStringToBuffer(ID_STRING_MGMT_CHARSET_SPECIALS2);
            if(guiAskForConfirmation(2, &conf_text) == RETURN_OK)
            {
                flags |= CHARSET_BIT_SPECIALS2;
            }

            /* Enable charset? specials: \"'`^|~ */
            conf_text.lines[0] = readStoredStringToBuffer(ID_STRING_MGMT_ENABLE_SPECIALSQ);
            conf_text.lines[1] = readStoredStringToBuffer(ID_STRING_MGMT_CHARSET_SPECIALS3);
            if(guiAskForConfirmation(2, &conf_text) == RETURN_OK)
            {
                flags |= CHARSET_BIT_SPECIALS3;
            }

            /* Enable charset? specials: _#$%&@ */
            conf_text.lines[0] = readStoredStringToBuffer(ID_STRING_MGMT_ENABLE_SPECIALSQ);
            conf_text.lines[1] = readStoredStringToBuffer(ID_STRING_MGMT_CHARSET_SPECIALS4);
            if(guiAskForConfirmation(2, &conf_text) == RETURN_OK)
            {
                flags |= CHARSET_BIT_SPECIALS4;
            }

            /* Enable charset? space */
            if(guiAskForConfirmation(1, (confirmationText_t*)readStoredStringToBuffer(ID_STRING_MGMT_ENABLE_SPACEQ)) == RETURN_OK)
            {
                flags |= CHARSET_BIT_SPACE;
            }
        }
        while ((flags & NODE_F_CHILD_USERFLAGS_MASK) == 0x00); /* ask again until the user picks at least one charset */

        return flags;
    } 
    else
    {
        return original_flags;
    }
}

/*! \fn     generateRandomPassword(uint8_t *password, uint8_t length, uint16_t flags)
*   \brief  generates a new random password according to allowed charset
*
*           Generates a new random password according to a provided allowed charset bitfield.
*           If no allowed charset are defined, defaults to all charsets but the 'space' character,
*           to prevent confusing passwords where the space could not be easily detected (i.e. at
*           the beginning or end of a password).
*   \param  password    password buffer to fill
*   \param  length      requested password length
*   \param  flags       child node flags (allowed charset)
*   \return RETURN_OK if generation was successful, RETURN_NOK otherwise
*/
RET_TYPE generateRandomPassword(uint8_t *password, uint8_t length, uint16_t flags)
{
    /* allowed charset flags */
    uint16_t charsetflags = flags & NODE_F_CHILD_USERFLAGS_MASK;
    /* list of charset sizes */
    uint8_t charset_sizes[8] = {CHARSET_SIZE_SPACE, CHARSET_SIZE_SPECIALS4, CHARSET_SIZE_SPECIALS3, CHARSET_SIZE_SPECIALS2,
        CHARSET_SIZE_SPECIALS1, CHARSET_SIZE_NUM, CHARSET_SIZE_ALPHA_LOWER, CHARSET_SIZE_ALPHA_UPPER};
    /* list of charset string IDs. 'space' is an exception */
    uint8_t charset_strings[8] = {0, ID_STRING_MGMT_CHARSET_SPECIALS4, ID_STRING_MGMT_CHARSET_SPECIALS3, ID_STRING_MGMT_CHARSET_SPECIALS2, ID_STRING_MGMT_CHARSET_SPECIALS1,
        ID_STRING_MGMT_CHARSET_NUM, ID_STRING_MGMT_CHARSET_ALPHA_LOWER, ID_STRING_MGMT_CHARSET_ALPHA_UPPER};
    /* final allowed charset size */
    uint8_t charset_final_size = 0;
    char * current_charset; /* temporary pointer to charset loaded from external flash */
    char cs[96];            /* final charset, null-terminated */

    /* exit if inconsistent length */
    if(length > (NODE_CHILD_SIZE_OF_PASSWORD - 1))
    {
        return RETURN_NOK;
    }

    /* force all charsets but space if none are allowed */
    if(charsetflags == 0x0000)
    {
        charsetflags = (NODE_F_CHILD_USERFLAGS_MASK & ~CHARSET_BIT_SPACE);
    }

    /* calculate allowed charset size from bitfield */
    for (uint8_t i = 0; i < 8; ++i)
    {
        if((charsetflags & (1 << i)) > 0)
        {
            if(i == 0)
            {
                cs[i] = ' ';
            }
            else
            {
                current_charset = readStoredStringToBuffer(charset_strings[i]);
                strcpy(cs+charset_final_size, current_charset);
            }
            charset_final_size += charset_sizes[i];
        }
    }

    /* force null-termination */
    cs[95] = '\0';
    cs[charset_final_size] = '\0';

    /* pre-fill password with random numbers */
    fillArrayWithRandomBytes(password, NODE_CHILD_SIZE_OF_PASSWORD - 1);
    password[NODE_CHILD_SIZE_OF_PASSWORD-1] = 0;

    /* use each random number as a character index to pick from the allowed charset */
    for (uint8_t i = 0; i < NODE_CHILD_SIZE_OF_PASSWORD; ++i)
    {
        /* truncate after desired length, but keep random data as padding */
        if(i >= length)
        {
            password[i] = '\0';
            break;
        }

        /* FIXME: WARNING, using modulus to generate random values within a range
         * introduces a cryptographic bias, as it breaks range uniformity */
        password[i] = cs[(password[i] % charset_final_size)];
    }
    /* display picked password */
    guiDisplayLoginOrPasswordOnScreen((char *)password);

    return RETURN_OK;
}

/*! \fn     sendOrDisplayString(char * str, uint8_t is_password)
*   \brief  sends a string as keystrokes over USB, or display it on screen otherwise
*   \param  str         string to send over USB keypresses or display
*   \param  is_password send key after password if != 0, key after login otherwise
*/
void sendOrDisplayString(char * str, uint8_t is_password)
{
    if (isUsbConfigured())
    {
        usbKeybPutStr(str);

        if(is_password)
        {
            if (getMooltipassParameterInEeprom(KEY_AFTER_PASS_SEND_BOOL_PARAM) != FALSE)
            {
                usbKeyboardPress(getMooltipassParameterInEeprom(KEY_AFTER_PASS_SEND_PARAM), 0);
            }
        }
        else
        {
            if (getMooltipassParameterInEeprom(KEY_AFTER_LOGIN_SEND_BOOL_PARAM) != FALSE)
            {
                usbKeyboardPress(getMooltipassParameterInEeprom(KEY_AFTER_LOGIN_SEND_PARAM), 0);
            }
        }
    }
    else
    {
        guiDisplayLoginOrPasswordOnScreen(str);
    }
}

/*! \fn     loginManagementSelectLogic(void)
*   \brief  Logic for finding a given login in credential management mode.
*           Performs credential edition/renewal/deletion.
*/
void loginManagementSelectLogic(void)
{
    uint16_t chosen_service_addr;
    uint16_t chosen_login_addr;
    uint8_t state_machine = 0;
    uint8_t strbuffer[NODE_CHILD_SIZE_OF_LOGIN];    /* buffer for text input */
    uint8_t newpasslen;                             /* requested password length */

    /* if the user wants to add credentials, we might have to create a new service */
    if(ondevice_cred_mgmt_action == ONDEVICE_CRED_MGMT_ACTION_CREATE)
    {
        /* First ask if the user wants to add a new service. Force adding a new service if none exists. */
        if((getStartingParentAddress() == NODE_ADDR_NULL) || (guiAskForConfirmation(1, (confirmationText_t*)readStoredStringToBuffer(ID_STRING_MGMT_CREATENEWSERVICEQ)) == RETURN_OK))
        {
            if(miniTextEntry((char *)strbuffer, NODE_CHILD_SIZE_OF_LOGIN, 0, 0, 0, readStoredStringToBuffer(ID_STRING_MGMT_TYPE_SVCNAME)) == RETURN_OK)
            {
                if (addNewContext(strbuffer, NODE_CHILD_SIZE_OF_LOGIN, SERVICE_CRED_TYPE) == RETURN_OK)
                {
                    guiDisplayInformationOnScreenAndWait(ID_STRING_MGMT_OPSUCCESS);
                }
                else
                {
                    guiDisplayInformationOnScreenAndWait(ID_STRING_MGMT_OPFAILURE);
                }
            }
        }
    }

    if (getStartingParentAddress() == NODE_ADDR_NULL)
    {
        guiDisplayInformationOnScreenAndWait(ID_STRING_MGMT_NOSERVICEAVAILABLE);
        return;
    }

    while (TRUE)
    {
        if (state_machine == 0)
        {
            // Ask user to select a service
            chosen_service_addr = loginSelectionScreen();

            // No service was chosen
            if (chosen_service_addr == NODE_ADDR_NULL)
            {
                return;
            }

            /* manually set context */
            context_parent_node_addr = chosen_service_addr;
            context_valid_flag = TRUE;

            /* load selected service parent node */
            readParentNode(&temp_pnode, chosen_service_addr);

            state_machine++;
        }
        else if (state_machine == 1)
        {
            /* the user asked to create new credentials */
            if(ondevice_cred_mgmt_action == ONDEVICE_CRED_MGMT_ACTION_CREATE)
            {
                /* Ask for new login */
                if(miniTextEntry((char *)strbuffer, NODE_CHILD_SIZE_OF_LOGIN, 0, 0, 0, readStoredStringToBuffer(ID_STRING_MGMT_TYPE_LOGIN)) == RETURN_OK)
                {
                    /* create login entry, with temporary random password */
                    if(setLoginForContext(strbuffer, strlen((char *)strbuffer)) == RETURN_OK)
                    {
                        /* retrieve newly created child node */
                        chosen_login_addr = selected_login_child_node_addr;
                        readChildNode(&temp_cnode, chosen_login_addr);

                        /* set allowed charset for password */
                        temp_cnode.flags = askUserToSelectCharset(temp_cnode.flags);

                        /* ask for desired password length and generate it */
                        if(miniTextEntry((char *)&newpasslen, 1, 0, 1, NODE_CHILD_SIZE_OF_PASSWORD-1, readStoredStringToBuffer(ID_STRING_MGMT_PASSWORDLENGTHQ)) == RETURN_OK)
                        {
                            generateRandomPassword(temp_cnode.password, newpasslen, temp_cnode.flags);
                            encrypt32bBlockOfDataAndClearCTVFlag(temp_cnode.password, temp_cnode.ctr);
                        }
                        /* save modified child node to external flash */
                        askUserToSaveToFlash(&temp_pnode, &temp_cnode, chosen_service_addr, chosen_login_addr);
                    }

                }
                ondevice_cred_mgmt_action = ONDEVICE_CRED_MGMT_ACTION_NONE; /* exit credential management */
                return;
            }

            // If there are different logins for this service, ask the user to pick one
            chosen_login_addr = guiAskForLoginSelect(&temp_pnode, &temp_cnode, chosen_service_addr, TRUE);

            // In case the user went back
            if ((chosen_login_addr == NODE_ADDR_NULL) && (miniGetLastReturnedAction() == WHEEL_ACTION_LONG_CLICK))
            {
                state_machine = 0;
            }
            else
            {
                state_machine++;
            }
        }
        else if (state_machine == 2)
        {
            /* load selected credentials */
            readChildNode(&temp_cnode, chosen_login_addr);

            /* the user has selected a specific login */
            switch(ondevice_cred_mgmt_action)
            {
                case ONDEVICE_CRED_MGMT_ACTION_EDIT:
                    /* edit login string */
                    if(guiAskForConfirmation(1, (confirmationText_t*)readStoredStringToBuffer(ID_STRING_MGMT_EDIT_LOGINQ)) == RETURN_OK)
                    {
                        if(miniTextEntry((char *)&temp_cnode.login, NODE_CHILD_SIZE_OF_LOGIN, strlen((char *)&temp_cnode.login), 0, 0, readStoredStringToBuffer(ID_STRING_MGMT_TYPE_LOGIN)) != RETURN_OK)
                        {
                            /* revert to previous child node state */
                            readChildNode(&temp_cnode, chosen_login_addr);
                        }
                    }
                    /* edit allowed charset */
                    temp_cnode.flags = askUserToSelectCharset(temp_cnode.flags);

                    /* save modified data */
                    askUserToSaveToFlash(&temp_pnode, &temp_cnode, chosen_service_addr, chosen_login_addr);
                    break;
                case ONDEVICE_CRED_MGMT_ACTION_RENEW:
                    /* Display or send login as keystrokes */
                    if(guiAskForConfirmation(1, (confirmationText_t*)readStoredStringToBuffer(ID_STRING_ENTERLOGINQ)) == RETURN_OK)
                    {
                        sendOrDisplayString((char*)temp_cnode.login, 0);
                    }

                    /* Display or send current password as keystrokes */
                    if(guiAskForConfirmation(1, (confirmationText_t*)readStoredStringToBuffer(ID_STRING_MGMT_ENTER_OLDPASSQ)) == RETURN_OK)
                    {
                        decrypt32bBlockOfDataAndClearCTVFlag(temp_cnode.password, temp_cnode.ctr);
                        sendOrDisplayString((char*)temp_cnode.password, 1);
                    }

                    /* Confirm password renewal */
                    if(guiAskForConfirmation(1, (confirmationText_t*)readStoredStringToBuffer(ID_STRING_MGMT_GENERATE_NEW_PASSQ)) == RETURN_OK)
                    {
                        /* Ask for new password length */
                        if(miniTextEntry((char *)&newpasslen, 1, 0, 1, NODE_CHILD_SIZE_OF_PASSWORD-1, readStoredStringToBuffer(ID_STRING_MGMT_PASSWORDLENGTHQ)) == RETURN_OK)
                        {
                            /* generate new password */
                            generateRandomPassword(temp_cnode.password, newpasslen, temp_cnode.flags);
                            encrypt32bBlockOfDataAndClearCTVFlag(temp_cnode.password, temp_cnode.ctr);

                            /* Ask to save to flash */
                            if(askUserToSaveToFlash(&temp_pnode, &temp_cnode, chosen_service_addr, chosen_login_addr) == RETURN_OK)
                            {
                                /* ask to display or send the new password as keystrokes */
                                decrypt32bBlockOfDataAndClearCTVFlag(temp_cnode.password, temp_cnode.ctr);
                                if(guiAskForConfirmation(1, (confirmationText_t*)readStoredStringToBuffer(ID_STRING_MGMT_ENTER_NEWPASSQ)) == RETURN_OK)
                                {
                                    sendOrDisplayString((char*)temp_cnode.password, 1);

                                    /* eventually repeat the new password */
                                    if(guiAskForConfirmation(1, (confirmationText_t*)readStoredStringToBuffer(ID_STRING_MGMT_ENTER_NEWPASS_AGAINQ)) == RETURN_OK)
                                    {
                                        sendOrDisplayString((char*)temp_cnode.password, 1);
                                    }
                                }
                            }
                        }
                    }
                    break;
                case ONDEVICE_CRED_MGMT_ACTION_DELETE:
                    /* ask twice for confirmation */
                    if((guiAskForConfirmation(1, (confirmationText_t*)readStoredStringToBuffer(ID_STRING_MGMT_DELETE_CREDSQ)) == RETURN_OK) && (guiAskForConfirmation(1, (confirmationText_t*)readStoredStringToBuffer(ID_STRING_MGMT_AREYOUSUREQ)) == RETURN_OK))
                    {
                        cNode buf_cnode;
                        deleteChildNode(chosen_service_addr, chosen_login_addr, &buf_cnode);
                    }
                    break;
                default:
                    break;
            }
            ondevice_cred_mgmt_action = ONDEVICE_CRED_MGMT_ACTION_NONE; /* exit credential management */
            return;
        }
    }
}
#endif
