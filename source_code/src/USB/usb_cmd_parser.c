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
/*!  \file     usb_cmd_parser.c
*    \brief    USB communication communication parser
*    Created:  09/6/2014
*    Author:   Mathieu Stephan
*/
#include "smart_card_higher_level_functions.h"
#include "touch_higher_level_functions.h"
#include "gui_smartcard_functions.h"
#include "logic_fwflash_storage.h"
#include "gui_screen_functions.h"
#include "gui_basic_functions.h"
#include "logic_aes_and_comms.h"
#include "gui_pin_functions.h"
#include "eeprom_addresses.h"
#include "watchdog_driver.h"
#include "logic_smartcard.h"
#include "usb_cmd_parser.h"
#include "timer_manager.h"
#include "oled_wrapper.h"
#include "logic_eeprom.h"
#include "hid_defines.h"
#include <avr/eeprom.h>
#include "mooltipass.h"
#include "node_mgmt.h"
#include "flash_mem.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "delays.h"
#include "utils.h"
#include "stack.h"
#include "usb.h"
#include "rng.h"

#ifdef FLASH_BLOCK_IMPORT_EXPORT
// Bool to specify if we're writing user flash space
uint8_t flash_import_user_space = FALSE;
// Operation unique identifier to know the current approved flash action
uint8_t currentFlashOpUid;
// One import/export address that may be used
uint16_t flashOpCurAddr1;
// Another import/export address that may be used
uint16_t flashOpCurAddr2;
#endif
#ifdef NODE_BLOCK_IMPORT_EXPORT
// Bool to know if the user approved memory management mode
uint8_t memoryManagementModeApproved = FALSE;
#endif
// Our Mooltipass version that will be returned to our application
const char mooltipass_version[] = FLASH_CHIP_STR "" MOOLTIPASS_VERSION;
// Bool to know if we can import in the media part of flash
uint8_t mediaFlashImportApproved = FALSE;
// Current node we're writing
uint16_t currentNodeWritten = NODE_ADDR_NULL;
// Media flash import temp page
uint16_t mediaFlashImportPage;
// Media flash import temp offset
uint16_t mediaFlashImportOffset;

/*! \fn     checkMooltipassPassword(uint8_t* data)
*   \brief  Check that the provided bytes is the mooltipass password
*   \param  data            Password to be checked
*   \param  addr            Address in eeprom where password is stored
*   \param  length          Length of the password
*   \return TRUE or FALSE
*/
uint8_t checkMooltipassPassword(uint8_t* data, void* addr, uint8_t length)
{
    // We use PACKET_EXPORT_SIZE as our passwords are never longer than that
    uint8_t mooltipass_password[PACKET_EXPORT_SIZE];
    
    // Read password in eeprom
    eeprom_read_block((void*)mooltipass_password, (void*)addr, length);
    
    // Preventing side channel attacks: only return after a given amount of time
    activateTimer(TIMER_CREDENTIALS, 1000);
    
    // Do the comparison
    volatile uint8_t password_comparison_result = memcmp((void*)mooltipass_password, (void*)data, length);
    
    // Wait for credential timer to fire (we wanted to clear credential_timer_valid flag anyway)
    while (hasTimerExpired(TIMER_CREDENTIALS, FALSE) == TIMER_RUNNING);
    
    // Clear buffer
    memset((void*)mooltipass_password, 0x00, PACKET_EXPORT_SIZE);
    
    // Check comparison result
    if (password_comparison_result == 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

#ifdef FLASH_BLOCK_IMPORT_EXPORT
/*! \fn     approveImportExportMemoryOperation(uint8_t opUID, uint8_t* pluginAnswer)
*   \brief  Approve a Flash/Eeprom import/export operation
*   \param  opUID           Unique memory operation identifier
*   \param  pluginAnswer    Pointer to the plugin answer byte
*/
void approveImportExportMemoryOperation(uint8_t opUID, uint8_t* pluginAnswer)
{
    // Set all global vars to 0
    flashOpCurAddr1 = 0;
    flashOpCurAddr2 = 0;
    currentFlashOpUid = 0;
    *pluginAnswer = PLUGIN_BYTE_ERROR;
    
    // Ask permission to the user
    if (guiAskForConfirmation(1, (confirmationText_t*)readStoredStringToBuffer(ID_STRING_APPROVEMEMOP)) == RETURN_OK)
    {
        // Ask the user to enter his pin
        if (removeCardAndReAuthUser() == RETURN_OK)
        {
            currentFlashOpUid = opUID;
            *pluginAnswer = PLUGIN_BYTE_OK;            
        }
        else
        {
            guiSetCurrentScreen(SCREEN_DEFAULT_INSERTED_LCK);
            guiGetBackToCurrentScreen();
        }
    }
}
#endif

#ifdef NODE_BLOCK_IMPORT_EXPORT
/*! \fn     leaveMemoryManagementMode(void)
*   \brief  Leave memory management mode
*/
void leaveMemoryManagementMode(void)
{
    memoryManagementModeApproved = FALSE;
}
#endif

/*! \fn     lowerCaseString(char* data)
*   \brief  lower case a string
*   \param  data            String to be lowercased
*/
void lowerCaseString(uint8_t* data)
{
    while(*data)
    {
        *data = tolower(*data);
        data++;
    }
}

/*! \fn     checkTextField(uint8_t* data, uint8_t len)
*   \brief  Check that the sent text is correct
*   \param  data    Pointer to the data
*   \param  len     Length of the text
*   \param  max_len Max length allowed
*   \return If the sent text is ok
*/
RET_TYPE checkTextField(uint8_t* data, uint8_t len, uint8_t max_len)
{    
    // Check that the advertised length is correct, that it is not null and isn't bigger than a data packet
    if ((len > max_len) || (len == 0) || (len != strlen((char*)data)+1) || (len > (RAWHID_RX_SIZE-HID_DATA_START)))
    {
        return RETURN_NOK;
    }
    else
    {
        // lower case string in case of service
        if (max_len == NODE_PARENT_SIZE_OF_SERVICE)
        {
            lowerCaseString(data);
        }
        return RETURN_OK;
    }
}

/*! \fn     usbProcessIncoming(uint8_t caller_id)
*   \brief  Process a possible incoming USB packet
*   \param  caller_id   UID of the calling function
*/
void usbProcessIncoming(uint8_t caller_id)
{
    // Our USB data buffer
    uint8_t incomingData[RAWHID_TX_SIZE];
    
    // Try to read data from USB, return if we didn't receive anything
    if(usbRawHidRecv(incomingData) != RETURN_COM_TRANSF_OK)
    {
        return;
    }
    
    // Temp plugin return value, error by default
    uint8_t plugin_return_value = PLUGIN_BYTE_ERROR;

    // Use message structure
    usbMsg_t* msg = (usbMsg_t*)incomingData;

    // Get data len
    uint8_t datalen = msg->len;

    // Get data cmd
    uint8_t datacmd = msg->cmd;

#ifdef USB_FEATURE_PLUGIN_COMMS
    // Temp ret_type
    RET_TYPE temp_rettype;
#endif
    
#ifdef DEV_PLUGIN_COMMS
    char stack_str[10];
#endif

    // Debug comms
    // USBDEBUGPRINTF_P(PSTR("usb: rx cmd 0x%02x len %u\n"), datacmd, datalen);
    
    // Check if we're currently asking the user to enter his PIN or want to query the MP status
    if ((caller_id == USB_CALLER_PIN) || (datacmd == CMD_MOOLTIPASS_STATUS))
    {
        uint8_t mp_status = 0x00;
        // Last bit: is card inserted
        if (isSmartCardAbsent() == RETURN_NOK)
        {
            mp_status |= 0x01;
        } 
        // Unlocking screen
        if (caller_id == USB_CALLER_PIN)
        {
            mp_status |= 0x02;
        }
        // Smartcard unlocked
        if (getSmartCardInsertedUnlocked() == TRUE)
        {
            mp_status |= 0x04;
        }
        // Unknown card
        if (getCurrentScreen() == SCREEN_DEFAULT_INSERTED_UNKNOWN)
        {
            mp_status |= 0x08;
        }
        // Inform the plugin to inform the user to unlock his card
        usbSendMessage(CMD_MOOLTIPASS_STATUS, 1, &mp_status);
        return;
    }
    
    // Check the text fields when needed
    uint8_t text_field_check_needed = TRUE;
    uint8_t max_text_size = 0;
    if ((datacmd == CMD_CONTEXT) || (datacmd == CMD_ADD_CONTEXT) || (datacmd == CMD_SET_DATA_SERVICE) || (datacmd == CMD_ADD_DATA_SERVICE))
    {
        max_text_size = NODE_PARENT_SIZE_OF_SERVICE;
    } 
    else if (datacmd == CMD_SET_LOGIN)
    {
        max_text_size = NODE_CHILD_SIZE_OF_LOGIN;
    }
    else if ((datacmd == CMD_SET_PASSWORD) || (datacmd == CMD_CHECK_PASSWORD))
    {
        max_text_size = NODE_CHILD_SIZE_OF_PASSWORD;
    }
    else if (datacmd == CMD_SET_CARD_LOGIN)
    {
        max_text_size = SMARTCARD_MTP_LOGIN_LENGTH/8;
    }
    else if (datacmd == CMD_SET_CARD_PASS)
    {
        max_text_size = SMARTCARD_MTP_PASS_LENGTH/8;
    }
    else
    {
        text_field_check_needed = FALSE;
    }
    
    // Perform the text field check
    if ((text_field_check_needed == TRUE) && (checkTextField(msg->body.data, datalen, max_text_size) == RETURN_NOK))
    {
        // Return an error that was defined before (ERROR)
        usbSendMessage(datacmd, 1, &plugin_return_value);
        return;
    }
    
    // Check that we are in node mangement mode when needed
    if ((datacmd >= FIRST_CMD_FOR_DATAMGMT) && (datacmd <= LAST_CMD_FOR_DATA8MGMT) && (memoryManagementModeApproved == FALSE))
    {
        // Return an error that was defined before (ERROR)
        usbSendMessage(datacmd, 1, &plugin_return_value);
        return;        
    }

    // Otherwise, process command
    switch(datacmd)
    {
        // ping command
        case CMD_PING :
        {
            usbHidSend(0, msg, 6);
            return;
        }

        // version command
        case CMD_VERSION :
        {
            usbSendMessage(CMD_VERSION, sizeof(mooltipass_version), mooltipass_version);
            return;
        }

#ifdef USB_FEATURE_PLUGIN_COMMS
        // credential context command
        case CMD_CONTEXT :
        {
            // So in case we're in memory management mode and want to set context, the LUT could be outdated
            if (memoryManagementModeApproved == TRUE)
            {                
                // Update our LUT
                populateServicesLut();
            }
            if (getSmartCardInsertedUnlocked() != TRUE)
            {
                plugin_return_value = PLUGIN_BYTE_NOCARD;
                USBPARSERDEBUGPRINTF_P(PSTR("set context: no card\n"));                
            }
            else if (setCurrentContext(msg->body.data, SERVICE_CRED_TYPE) == RETURN_OK)
            {
                plugin_return_value = PLUGIN_BYTE_OK;
                USBPARSERDEBUGPRINTF_P(PSTR("set context: \"%s\" ok\n"), msg->body.data);
            }
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
                USBPARSERDEBUGPRINTF_P(PSTR("set context: \"%s\" failed\n"), msg->body.data);
            }
            break;
        }
        
        // data context command
        case CMD_SET_DATA_SERVICE :
        {
            if (getSmartCardInsertedUnlocked() != TRUE)
            {
                plugin_return_value = PLUGIN_BYTE_NOCARD;
                USBPARSERDEBUGPRINTF_P(PSTR("set context: no card\n"));                
            }
            else if (setCurrentContext(msg->body.data, SERVICE_DATA_TYPE) == RETURN_OK)
            {
                plugin_return_value = PLUGIN_BYTE_OK;
                USBPARSERDEBUGPRINTF_P(PSTR("set context: \"%s\" ok\n"), msg->body.data);
            }
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
                USBPARSERDEBUGPRINTF_P(PSTR("set context: \"%s\" failed\n"), msg->body.data);
            }
            break;
        }

        // get login
        case CMD_GET_LOGIN :
        {
            if (getLoginForContext((char*)incomingData) == RETURN_OK)
            {
                // Use the buffer to store the login...
                usbSendMessage(CMD_GET_LOGIN, strlen((char*)incomingData)+1, incomingData);
                USBPARSERDEBUGPRINTF_P(PSTR("get login: \"%s\"\n"),(char *)incomingData);
                return;
            }
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
                USBPARSERDEBUGPRINTF_P(PSTR("get login: failed\n"));
            }
            break;
        }

        // get password
        case CMD_GET_PASSWORD :
        {
            if (getPasswordForContext((char*)incomingData) == RETURN_OK)
            {
                usbSendMessage(CMD_GET_PASSWORD, strlen((char*)incomingData)+1, incomingData);
                USBPARSERDEBUGPRINTF_P(PSTR("get pass: \"%s\"\n"),(char *)incomingData);
                return;
            }
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
                USBPARSERDEBUGPRINTF_P(PSTR("get pass: failed\n"));
            }
            break;
        }

        // set login
        case CMD_SET_LOGIN :
        {
            if (setLoginForContext(msg->body.data, datalen) == RETURN_OK)
            {
                plugin_return_value = PLUGIN_BYTE_OK;
                USBPARSERDEBUGPRINTF_P(PSTR("set login: \"%s\" ok\n"),msg->body.data);
            }
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
                USBPARSERDEBUGPRINTF_P(PSTR("set login: \"%s\" failed\n"),msg->body.data);
            }
            break;
        }

        // set password
        case CMD_SET_PASSWORD :
        {
            if (setPasswordForContext(msg->body.data, datalen) == RETURN_OK)
            {
                plugin_return_value = PLUGIN_BYTE_OK;
                USBPARSERDEBUGPRINTF_P(PSTR("set pass: \"%s\" ok\n"),msg->body.data);
            }
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
                USBPARSERDEBUGPRINTF_P(PSTR("set pass: failed\n"));
            }
            break;
        }

        // check password
        case CMD_CHECK_PASSWORD :
        {
            temp_rettype = checkPasswordForContext(msg->body.data);
            if (temp_rettype == RETURN_PASS_CHECK_NOK)
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
            }
            else if(temp_rettype == RETURN_PASS_CHECK_OK)
            {
                plugin_return_value = PLUGIN_BYTE_OK;
            }
            else
            {
                plugin_return_value = PLUGIN_BYTE_NA;
            }
            break;
        }

        // Add credential context
        case CMD_ADD_CONTEXT :
        {
            if (addNewContext(msg->body.data, datalen, SERVICE_CRED_TYPE) == RETURN_OK)
            {
                // We managed to add a new context
                plugin_return_value = PLUGIN_BYTE_OK;
                USBPARSERDEBUGPRINTF_P(PSTR("add context: \"%s\" ok\n"),msg->body.data);
            }
            else
            {
                // Couldn't add a new context
                plugin_return_value = PLUGIN_BYTE_ERROR;
                USBPARSERDEBUGPRINTF_P(PSTR("add context: \"%s\" failed\n"),msg->body.data);
            }
            break;
        }
        
        // Add data context
        case CMD_ADD_DATA_SERVICE :
        {
            if (addNewContext(msg->body.data, datalen, SERVICE_DATA_TYPE) == RETURN_OK)
            {
                // We managed to add a new context
                plugin_return_value = PLUGIN_BYTE_OK;
                USBPARSERDEBUGPRINTF_P(PSTR("add context: \"%s\" ok\n"),msg->body.data);
            }
            else
            {
                // Couldn't add a new context
                plugin_return_value = PLUGIN_BYTE_ERROR;
                USBPARSERDEBUGPRINTF_P(PSTR("add context: \"%s\" failed\n"),msg->body.data);
            }
            break;
    }
    
    // Append data
    case CMD_WRITE_32B_IN_DN :
    {
        if ((addDataForDataContext(&msg->body.data[1], msg->body.data[0]) == RETURN_OK) && (datalen == 1+DATA_NODE_BLOCK_SIZ))
        {
            plugin_return_value = PLUGIN_BYTE_OK;
            USBPARSERDEBUGPRINTF_P(PSTR("set pass: \"%s\" ok\n"),msg->body.data);
        }
        else
        {
            plugin_return_value = PLUGIN_BYTE_ERROR;
            USBPARSERDEBUGPRINTF_P(PSTR("set pass: failed\n"));
        }
        break;
    }    
    
    // get login
    case CMD_READ_32B_IN_DN :
    {
        if (get32BytesDataForCurrentService(incomingData) == RETURN_OK)
        {
            // Use the buffer to store the login...
            usbSendMessage(CMD_READ_32B_IN_DN, DATA_NODE_BLOCK_SIZ, incomingData);
            USBPARSERDEBUGPRINTF_P(PSTR("get login: \"%s\"\n"),(char *)incomingData);
            return;
        }
        else
        {
            plugin_return_value = PLUGIN_BYTE_ERROR;
            USBPARSERDEBUGPRINTF_P(PSTR("get login: failed\n"));
        }
        break;
    }
#endif

#ifdef FLASH_BLOCK_IMPORT_EXPORT
        // flash export start
        case CMD_EXPORT_FLASH_START :
        {
            approveImportExportMemoryOperation(CMD_EXPORT_FLASH_START, &plugin_return_value);
            guiGetBackToCurrentScreen();
            break;
        }

        // export flash contents
        case CMD_EXPORT_FLASH :
        {
            uint8_t size = PACKET_EXPORT_SIZE;
            
            // Check that the user approved
            if (currentFlashOpUid != CMD_EXPORT_FLASH_START)
            {
                return;
            }

            //flashOpCurAddr1 is the page
            //flashOpCurAddr2 is the offset
            // Check if the export address is correct
            if (flashOpCurAddr1 >= PAGE_COUNT)
            {
                usbSendMessage(CMD_EXPORT_FLASH_END, 0, NULL);
                USBPARSERDEBUGPRINTF_P(PSTR("export: end\n"));
                currentFlashOpUid = 0;
                return;
            }

            // Check how much data we need in case we're close to the page end
            if ((BYTES_PER_PAGE - flashOpCurAddr2) < PACKET_EXPORT_SIZE)
            {
                size = (uint8_t)(BYTES_PER_PAGE - flashOpCurAddr2);
            }

            // Get a block of data and send it, increment counter
            readDataFromFlash(flashOpCurAddr1, flashOpCurAddr2, size, (void*)incomingData);
            usbSendMessage(CMD_EXPORT_FLASH, size, incomingData);
            //usbSendMessageWithRetries(CMD_EXPORT_FLASH, size, (char*)incomingData, 255);
            flashOpCurAddr2 += size;
            
            if (flashOpCurAddr2 == BYTES_PER_PAGE)
            {
                flashOpCurAddr2 = 0;
                flashOpCurAddr1++;
            }

            // Skip over the graphics address if we're in that case
            if (flashOpCurAddr1 == GRAPHIC_ZONE_PAGE_START)
            {
                flashOpCurAddr1 = GRAPHIC_ZONE_PAGE_END;
            }
            return;
        }
        
        // flash export end
        case CMD_EXPORT_FLASH_END :
        {
            currentFlashOpUid = 0;
            return;
        }

        // flash export start
        case CMD_EXPORT_EEPROM_START :
        {
            approveImportExportMemoryOperation(CMD_EXPORT_EEPROM_START, &plugin_return_value);
            guiGetBackToCurrentScreen();
            break;
        }

        // export eeprom contents
        case CMD_EXPORT_EEPROM :
        {
            uint8_t size = PACKET_EXPORT_SIZE;

            // Check that the user approved
            if (currentFlashOpUid != CMD_EXPORT_EEPROM_START)
            {
                return;
            }

            //flashOpCurAddr1 is the current eeprom address
            // Check if the export address is correct
            if (flashOpCurAddr1 >= EEPROM_SIZE)
            {
                usbSendMessage(CMD_EXPORT_EEPROM_END, 0, NULL);
                USBPARSERDEBUGPRINTF_P(PSTR("export: end\n"));
                currentFlashOpUid = 0;
                return;
            }

            // Check how much data we need
            if ((EEPROM_SIZE - flashOpCurAddr1) < PACKET_EXPORT_SIZE)
            {
                size = (uint8_t)(EEPROM_SIZE - flashOpCurAddr1);
            }

            // Get a block of data and send it, increment counter
            eeprom_read_block(incomingData, (void*)flashOpCurAddr1, size);
            usbSendMessage(CMD_EXPORT_EEPROM, size, (char*)incomingData);
            //usbSendMessageWithRetries(CMD_EXPORT_EEPROM, size, (char*)incomingData, 255);
            flashOpCurAddr1 += size;
            return;
        }
        
        // end eeprom export
        case CMD_EXPORT_EEPROM_END :
        {
            currentFlashOpUid = 0;
            return;
        }

        // import flash contents
        case CMD_IMPORT_FLASH_BEGIN :
        {
            // Check datalen for arg
            if (datalen != 1)
            {
                USBPARSERDEBUGPRINTF_P(PSTR("import: no param\n"));
                return;
            }
            
            // Ask user approval            
            approveImportExportMemoryOperation(CMD_IMPORT_FLASH_BEGIN, &plugin_return_value);

            //flashOpCurAddr1 is the page
            //flashOpCurAddr2 is the offset
            // Check what we want to write
            if (msg->body.data[0] == 0x00)
            {
                flashOpCurAddr1 = 0x0000;
                flash_import_user_space = TRUE;
            }
            else
            {
                flash_import_user_space = FALSE;
                flashOpCurAddr1 = GRAPHIC_ZONE_PAGE_START;
            }
            
            // Get back to normal screen
            guiGetBackToCurrentScreen();
            break;
        }

        // import flash contents
        case CMD_IMPORT_FLASH :
        {
            // Check if we actually approved the import, haven't gone over the flash boundaries, if we're correctly aligned page size wise
            if ((currentFlashOpUid != CMD_IMPORT_FLASH_BEGIN) || (flashOpCurAddr1 >= PAGE_COUNT) || (flashOpCurAddr2 + datalen > BYTES_PER_PAGE) || ((flash_import_user_space == FALSE) && (flashOpCurAddr1 >= GRAPHIC_ZONE_PAGE_END)))
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
                currentFlashOpUid = 0;
            }
            else
            {
                flashWriteBuffer(msg->body.data, flashOpCurAddr2, datalen);
                flashOpCurAddr2+= datalen;

                // If we just filled a page, flush it to the page
                if (flashOpCurAddr2 == BYTES_PER_PAGE)
                {
                    flashWriteBufferToPage(flashOpCurAddr1);
                    flashOpCurAddr2 = 0;
                    flashOpCurAddr1++;

                    // If we are importing user contents, skip the graphics zone
                    if ((flash_import_user_space == TRUE) && (flashOpCurAddr1 == GRAPHIC_ZONE_PAGE_START))
                    {
                        flashOpCurAddr1 = GRAPHIC_ZONE_PAGE_END;
                    }
                }
                plugin_return_value = PLUGIN_BYTE_OK;
            }
            break;
        }

        // end flash import
        case CMD_IMPORT_FLASH_END :
        {
            if ((currentFlashOpUid == CMD_IMPORT_FLASH_BEGIN) && (flashOpCurAddr2 != 0))
            {
                flashWriteBufferToPage(flashOpCurAddr1);
            }
            plugin_return_value = PLUGIN_BYTE_OK;
            currentFlashOpUid = 0;
            break;
        }

        // import flash contents
        case CMD_IMPORT_EEPROM_BEGIN :
        {
            // Ask for user confirmation
            approveImportExportMemoryOperation(CMD_IMPORT_EEPROM_BEGIN, &plugin_return_value);
            guiGetBackToCurrentScreen();
            break;
        }

        // import flash contents
        case CMD_IMPORT_EEPROM :
        {
            // flashOpCurAddr1 is the current eeprom address
            if ((currentFlashOpUid != CMD_IMPORT_EEPROM_BEGIN) || ((flashOpCurAddr1 + datalen) >= EEPROM_SIZE))
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
                currentFlashOpUid = 0;
            }
            else
            {
                eeprom_write_block((void*)msg->body.data, (void*)flashOpCurAddr1, datalen);
                flashOpCurAddr1+= datalen;
                plugin_return_value = PLUGIN_BYTE_OK;
            }
            break;
        }

        // end eeprom import
        case CMD_IMPORT_EEPROM_END :
        {
            plugin_return_value = PLUGIN_BYTE_OK;
            currentFlashOpUid = 0;
            break;
        }
#endif
#ifdef NODE_BLOCK_IMPORT_EXPORT
        // Read user profile in flash
        case CMD_START_MEMORYMGMT :
        {            
            // Check that the smartcard is unlocked
            if (getSmartCardInsertedUnlocked() == TRUE)
            {
                // If so, ask the user to approve memory management mode
                if (guiAskForConfirmation(0xF0 | 1, (confirmationText_t*)readStoredStringToBuffer(ID_STRING_MEMORYMGMTQ)) == RETURN_OK)
                {
                    // Ask the user to enter his pin
                    if (removeCardAndReAuthUser() == RETURN_OK)
                    {
                        guiSetCurrentScreen(SCREEN_MEMORY_MGMT);
                        plugin_return_value = PLUGIN_BYTE_OK;
                        memoryManagementModeApproved = TRUE;
                    }
                    else
                    {
                        guiSetCurrentScreen(SCREEN_DEFAULT_INSERTED_LCK);
                    }
                }                
                // Change screen
                guiGetBackToCurrentScreen();
            }            
            break;
        }
        
        // Read starting parent
        case CMD_GET_STARTING_PARENT :
        {
            // Memory management mode check implemented before the switch
            // Read starting parent
            uint16_t temp_address = getStartingParentAddress();
                
            // Send address
            usbSendMessage(CMD_GET_STARTING_PARENT, 2, (uint8_t*)&temp_address);
                
            // Return
            return;         
        }
        
        // Read data starting parent
        case CMD_GET_DN_START_PARENT :
        {
            // Memory management mode check implemented before the switch
            // Read starting parent
            uint16_t temp_address = getStartingDataParentAddress();
                
            // Send address
            usbSendMessage(CMD_GET_DN_START_PARENT, 2, (uint8_t*)&temp_address);
                
            // Return
            return;          
        }
        
        // Get free node addresses
        case CMD_GET_FREE_SLOTS_ADDR :
        {
            // Check that an address has been provided
            if (datalen == 2)
            {
                // Memory management mode check implemented before the switch
                uint16_t* temp_addr_ptr = (uint16_t*)msg->body.data;
                uint16_t nodeAddresses[31];
                uint8_t nodesFound;
                
                // Call the dedicated function
                nodesFound = findFreeNodes(31, nodeAddresses, pageNumberFromAddress(*temp_addr_ptr), nodeNumberFromAddress(*temp_addr_ptr));
                
                // Send addresses
                usbSendMessage(CMD_GET_FREE_SLOTS_ADDR, nodesFound*2, (uint8_t*)nodeAddresses);
                return;
            }
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
                break;                
            }
        }
        
        // End memory management mode
        case CMD_END_MEMORYMGMT :
        {
            // Memory management mode check implemented before the switch
            // memoryManagementModeApproved is cleared when user removes his card
            guiSetCurrentScreen(SCREEN_DEFAULT_INSERTED_NLCK);
            plugin_return_value = PLUGIN_BYTE_OK;
            currentNodeWritten = NODE_ADDR_NULL;
            leaveMemoryManagementMode();
            guiGetBackToCurrentScreen();
            activityDetectedRoutine();
            populateServicesLut();
            scanNodeUsage();
            break;
        }
        
        // Read node from Flash
        case CMD_READ_FLASH_NODE :
        {
            // Memory management mode check implemented before the switch
            // Check that the mode is approved & that args are supplied
            if (datalen == 2)
            {
                // First two bytes are the node address
                uint16_t* temp_node_addr_ptr = (uint16_t*)msg->body.data;
                // Temp buffer to store the node
                uint8_t temp_buffer[NODE_SIZE];
                
                //  Check user permissions
                if(checkUserPermission(*temp_node_addr_ptr) == RETURN_OK)
                {
                    // Read node in flash & send it, ownership check is done in the function
                    readNode((gNode*)temp_buffer, *temp_node_addr_ptr);
                    usbSendMessage(CMD_READ_FLASH_NODE, NODE_SIZE, temp_buffer);
                    return;
                }
                else
                {
                    plugin_return_value = PLUGIN_BYTE_ERROR;
                }                
            }
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
            }
            break;
        }
        
        // Set favorite
        case CMD_SET_FAVORITE :
        {
            // Memory management mode check implemented before the switch
            // Check that the mode is approved & that args are supplied
            if (datalen == 5)
            {
                uint16_t* temp_par_addr = (uint16_t*)&msg->body.data[1];
                uint16_t* temp_child_addr = (uint16_t*)&msg->body.data[3];
                
                setFav(msg->body.data[0], *temp_par_addr, *temp_child_addr);
                plugin_return_value = PLUGIN_BYTE_OK;
            }
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
            }
            break;            
        }
        
        // Get favorite
        case CMD_GET_FAVORITE :
        {
            // Memory management mode check implemented before the switch
            // Check that the mode is approved & that args are supplied
            if (datalen == 1)
            {
                uint16_t data[2];
                readFav(msg->body.data[0], &data[0], &data[1]);
                usbSendMessage(CMD_GET_FAVORITE, 4, (void*)data);
                return;
            }
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
            }
            break;
        }
        
        // Set starting parent
        case CMD_SET_STARTING_PARENT :
        {
            // Memory management mode check implemented before the switch
            // Check that the mode is approved & that args are supplied
            if (datalen == 2)
            {
                uint16_t* temp_par_addr = (uint16_t*)&msg->body.data[0];
                setStartingParent(*temp_par_addr);
                plugin_return_value = PLUGIN_BYTE_OK;
            }
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
            }
            break;            
        }
        
        // Set data starting parent
        case CMD_SET_DN_START_PARENT :
        {
            // Memory management mode check implemented before the switch
            // Check that the mode is approved & that args are supplied
            if (datalen == 2)
            {
                uint16_t* temp_par_addr = (uint16_t*)&msg->body.data[0];
                setDataStartingParent(*temp_par_addr);
                plugin_return_value = PLUGIN_BYTE_OK;
            }
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
            }
            break;            
        }
        
        // Set new CTR value
        case CMD_SET_CTRVALUE :
        {
            // Memory management mode check implemented before the switch
            // Check that the mode is approved & that args are supplied
            if (datalen == USER_CTR_SIZE)
            {
                setProfileCtr(msg->body.data);
                plugin_return_value = PLUGIN_BYTE_OK;
            }
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
            }
            break;            
        }
        
        // Get CTR value
        case CMD_GET_CTRVALUE :
        {
            // Memory management mode check implemented before the switch
            // Temp buffer to store CTR
            uint8_t tempCtrVal[USER_CTR_SIZE];
                
            // Read CTR value
            readProfileCtr(tempCtrVal);
                
            // Send it
            usbSendMessage(CMD_GET_CTRVALUE, USER_CTR_SIZE, tempCtrVal);
            return;
        }
        
        // Add a known card to the MP, 8 first bytes is the CPZ, next 16 is the CTR nonce
        case CMD_ADD_CARD_CPZ_CTR :
        {
            // Memory management mode check implemented before the switch
            // Check that the mode is approved & that args are supplied
            if (datalen == SMARTCARD_CPZ_LENGTH + AES256_CTR_LENGTH)
            {
                if (writeSmartCardCPZForUserId(msg->body.data, &msg->body.data[SMARTCARD_CPZ_LENGTH], getCurrentUserID()) == RETURN_OK)
                {
                    plugin_return_value = PLUGIN_BYTE_OK;
                }
                else
                {
                    plugin_return_value = PLUGIN_BYTE_ERROR;                    
                }
            }
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
            }
            break;
        }
        
        // Get all the cpz ctr values for current user
        case CMD_GET_CARD_CPZ_CTR :
        {
            // Memory management mode check implemented before the switch
            outputLUTEntriesForGivenUser(getCurrentUserID());
            plugin_return_value = PLUGIN_BYTE_OK;            
            break;            
        }
        
        // Write node in Flash
        case CMD_WRITE_FLASH_NODE : 
        {
            // First two bytes are the node address
            uint16_t* temp_node_addr_ptr = (uint16_t*)msg->body.data;
            
            // Check that the plugin provided the address and packet #
            if (datalen < 3)
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
            } 
            else
            {                
                // If it is the first packet, store the address and load the page in the internal buffer
                if (msg->body.data[2] == 0)
                {
                    //  Check user permissions
                    if(checkUserPermission(*temp_node_addr_ptr) == RETURN_OK)
                    {
                        currentNodeWritten = *temp_node_addr_ptr;
                        loadPageToInternalBuffer(pageNumberFromAddress(currentNodeWritten));                        
                    }
                }
                
                // Check that the address the plugin wants to write is the one stored and that we're not writing more than we're supposed to
                if ((currentNodeWritten == *temp_node_addr_ptr) && (currentNodeWritten != NODE_ADDR_NULL) && (msg->body.data[2] * (PACKET_EXPORT_SIZE-3) + (datalen - 3) <= NODE_SIZE))
                {
                    // If it's the first packet, set correct user ID
                    if (msg->body.data[2] == 0)
                    {
                        userIdToFlags((uint16_t*)&(msg->body.data[3]), getCurrentUserID());
                    }
                    
                    // Fill the data at the right place
                    flashWriteBuffer(msg->body.data + 3, (NODE_SIZE * nodeNumberFromAddress(currentNodeWritten)) + (msg->body.data[2] * (PACKET_EXPORT_SIZE-3)), datalen - 3);
                    
                    // If we finished writing, flush buffer
                    if (msg->body.data[2] == (NODE_SIZE/(PACKET_EXPORT_SIZE-3)))
                    {
                        flashWriteBufferToPage(pageNumberFromAddress(currentNodeWritten));
                    }
                    
                    plugin_return_value = PLUGIN_BYTE_OK;
                }
                else
                {
                    plugin_return_value = PLUGIN_BYTE_ERROR;
                }
            }
            break;
        }
#endif

        // import media flash contents
        case CMD_IMPORT_MEDIA_START :
        {            
            // Set default addresses
            mediaFlashImportPage = GRAPHIC_ZONE_PAGE_START;
            mediaFlashImportOffset = 0;
            
            // No check if dev comms
            #ifdef DEV_PLUGIN_COMMS
                plugin_return_value = PLUGIN_BYTE_OK;
                mediaFlashImportApproved = TRUE;
            #else            
                // Mandatory wait for bruteforce
                userViewDelay();
                
                // Compare with our password if it is set
                if (datalen == PACKET_EXPORT_SIZE)
                {                    
                    // Prepare asking confirmation screen
                    confirmationText_t temp_conf_text;
                    temp_conf_text.lines[0] = readStoredStringToBuffer(ID_STRING_WARNING);
                    temp_conf_text.lines[1] = readStoredStringToBuffer(ID_STRING_ALLOW_UPDATE);
                    
                    // Allow bundle update if password is not set
                    if ((eeprom_read_byte((uint8_t*)EEP_BOOT_PWD_SET) != BOOTLOADER_PWDOK_KEY) || ((guiAskForConfirmation(2, &temp_conf_text) == RETURN_OK) && (checkMooltipassPassword(msg->body.data, (void*)EEP_BOOT_PWD, PACKET_EXPORT_SIZE) == TRUE)))
                    {
                        plugin_return_value = PLUGIN_BYTE_OK;
                        mediaFlashImportApproved = TRUE;
                    }
                    guiGetBackToCurrentScreen();
                }
            #endif            
            break;
        }

        // import media flash contents
        case CMD_IMPORT_MEDIA :
        {
            // Check if we actually approved the import, haven't gone over the flash boundaries, if we're correctly aligned page size wise
            if ((mediaFlashImportApproved == FALSE) || (mediaFlashImportPage >= GRAPHIC_ZONE_PAGE_END) || (mediaFlashImportOffset + datalen > BYTES_PER_PAGE))
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
                mediaFlashImportApproved = FALSE;
            }
            else
            {
                flashWriteBuffer(msg->body.data, mediaFlashImportOffset, datalen);
                mediaFlashImportOffset+= datalen;

                // If we just filled a page, flush it to the page
                if (mediaFlashImportOffset == BYTES_PER_PAGE)
                {
                    flashWriteBufferToPage(mediaFlashImportPage);
                    mediaFlashImportOffset = 0;
                    mediaFlashImportPage++;
                }
                plugin_return_value = PLUGIN_BYTE_OK;
            }
            break;
        }

        // end media flash import
        case CMD_IMPORT_MEDIA_END :
        {
            if ((mediaFlashImportApproved == TRUE) && (mediaFlashImportOffset != 0))
            {
                flashWriteBufferToPage(mediaFlashImportPage);
            }
            plugin_return_value = PLUGIN_BYTE_OK;
            mediaFlashImportApproved = FALSE;
            break;
        }
        
        // Set Mooltipass param
        case CMD_SET_MOOLTIPASS_PARM :
        {
            // Check that args are supplied
            if (datalen == 2)
            {
                // Set correct value in eeprom and refresh parameters that need refreshing
                setMooltipassParameterInEeprom(msg->body.data[0], msg->body.data[1]);
                mp_timeout_enabled = getMooltipassParameterInEeprom(LOCK_TIMEOUT_ENABLE_PARAM);
                plugin_return_value = PLUGIN_BYTE_OK;
                //initTouchSensing();
                //launchCalibrationCycle();
            }
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
            }
            break;
        }
        
        // Get Mooltipass param
        case CMD_GET_MOOLTIPASS_PARM :
        {
            plugin_return_value = getMooltipassParameterInEeprom(msg->body.data[0]);
            break;
        }
        
        // Get current card CPZ
        case CMD_GET_CUR_CARD_CPZ :
        {
            // Check that an unknown card is inserted
            if ((getCurrentScreen() == SCREEN_DEFAULT_INSERTED_UNKNOWN) || (getSmartCardInsertedUnlocked() == TRUE))
            {
                uint8_t temp_buffer[SMARTCARD_CPZ_LENGTH];
                
                // Read code protected zone
                readCodeProtectedZone(temp_buffer);
                
                // Send it to the app
                usbSendMessage(CMD_GET_CUR_CARD_CPZ, sizeof(temp_buffer), (void*)temp_buffer);
                return;
            }
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
            }
            break;
        }
        
        // Reset smartcard
        case CMD_RESET_CARD :
        {
            // Check we're not authenticated, check that the user could unlock the card
            if (getCurrentScreen() == SCREEN_DEFAULT_INSERTED_UNKNOWN)
            {
                // Ask the user to unlock the card
                activityDetectedRoutine();
                if (guiCardUnlockingProcess() == RETURN_OK)
                {
                    eraseSmartCard();
                    plugin_return_value = PLUGIN_BYTE_OK;
                    // Success, ask the user to remove the card
                    guiSetCurrentScreen(SCREEN_DEFAULT_INSERTED_INVALID);
                }
                else
                {
                    plugin_return_value = PLUGIN_BYTE_ERROR;
                }
                guiGetBackToCurrentScreen();
            }
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
            }
            break;
        }
        
        // Add current unknown smartcard
        case CMD_ADD_UNKNOWN_CARD :
        {
            // Check the args, check we're not authenticated, check that the user could unlock the card
            if ((datalen == SMARTCARD_CPZ_LENGTH + AES256_CTR_LENGTH) && (getCurrentScreen() == SCREEN_DEFAULT_INSERTED_UNKNOWN))
            {
                uint8_t temp_buffer[AES_KEY_LENGTH/8];
                uint8_t new_user_id;
                
                // Read code protected zone
                readCodeProtectedZone(temp_buffer);
                
                // Check that the provided CPZ is the current one, ask the user to unlock the card and check that we can add the user
                activityDetectedRoutine();
                if ((memcmp(temp_buffer, msg->body.data, SMARTCARD_CPZ_LENGTH) == 0) && (guiCardUnlockingProcess() == RETURN_OK) && (addNewUserForExistingCard(&msg->body.data[SMARTCARD_CPZ_LENGTH], &new_user_id) == RETURN_OK))
                {
                    // Success, jump to the main menu
                    readAES256BitsKey(temp_buffer);
                    initUserFlashContext(new_user_id);
                    initEncryptionHandling(temp_buffer, &msg->body.data[SMARTCARD_CPZ_LENGTH]);
                    setSmartCardInsertedUnlocked();
                    plugin_return_value = PLUGIN_BYTE_OK;
                    guiSetCurrentScreen(SCREEN_DEFAULT_INSERTED_NLCK);
                }
                else
                {
                    plugin_return_value = PLUGIN_BYTE_ERROR;                    
                }
                guiGetBackToCurrentScreen();
            }
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
            }
            break;
        }
        
        // Read card login
        case CMD_READ_CARD_LOGIN :
        {
            if (getSmartCardInsertedUnlocked() == TRUE)
            {
                uint8_t temp_data[SMARTCARD_MTP_LOGIN_LENGTH/8];
                readMooltipassWebsiteLogin(temp_data);
                usbSendMessage(CMD_READ_CARD_LOGIN, sizeof(temp_data), (void*)temp_data);
                return;
            } 
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
            }
            break;
        }
        
        // Read card stored password
        case CMD_READ_CARD_PASS :
        {
            if (getSmartCardInsertedUnlocked() == TRUE)
            {
                if (guiAskForConfirmation(1, (confirmationText_t*)readStoredStringToBuffer(ID_STRING_SEND_SMC_PASS)) == RETURN_OK)
                {
                    uint8_t temp_data[SMARTCARD_MTP_PASS_LENGTH/8];
                    readMooltipassWebsitePassword(temp_data);
                    usbSendMessage(CMD_READ_CARD_PASS, sizeof(temp_data), (void*)temp_data);
                    guiGetBackToCurrentScreen();
                    return;
                } 
                else
                {
                    guiGetBackToCurrentScreen();
                    plugin_return_value = PLUGIN_BYTE_ERROR;
                }
            } 
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
            }
            break;
        }
        
        // Set card login
        case CMD_SET_CARD_LOGIN :
        {
            if (getSmartCardInsertedUnlocked() == TRUE)
            {
                if (guiAskForConfirmation(1, (confirmationText_t*)readStoredStringToBuffer(ID_STRING_SET_SMC_LOGIN)) == RETURN_OK)
                {
                    // Temp buffer for application zone 2
                    uint8_t temp_az2[SMARTCARD_AZ_BIT_LENGTH/8];
                    
                    // Read Application Zone 2
                    readApplicationZone2(temp_az2);
                    // Erase Application Zone 2
                    eraseApplicationZone1NZone2SMC(FALSE);
                    // Write our data in the buffer at the right spot
                    memcpy(temp_az2 + (SMARTCARD_MTP_LOGIN_OFFSET/8), msg->body.data, datalen);
                    // Write the new data in the card
                    writeApplicationZone2(temp_az2);
                    
                    // Return OK
                    plugin_return_value = PLUGIN_BYTE_OK;
                } 
                else
                {
                    plugin_return_value = PLUGIN_BYTE_ERROR;
                }
                guiGetBackToCurrentScreen();
            }
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
            }
            break;
        }
        
        // Set card stored password
        case CMD_SET_CARD_PASS :
        {
            if (getSmartCardInsertedUnlocked() == TRUE)
            {
                if (guiAskForConfirmation(1, (confirmationText_t*)readStoredStringToBuffer(ID_STRING_SET_SMC_PASS)) == RETURN_OK)
                {
                    // Temp buffer for application zone 1
                    uint8_t temp_az1[SMARTCARD_AZ_BIT_LENGTH/8];
                    
                    // Read Application Zone 1
                    readApplicationZone1(temp_az1);
                    // Erase Application Zone 1
                    eraseApplicationZone1NZone2SMC(TRUE);
                    // Write our data in buffer
                    memcpy(temp_az1 + (SMARTCARD_MTP_PASS_OFFSET/8), msg->body.data, datalen);
                    // Write the new data in the card
                    writeApplicationZone1(temp_az1);
                    
                    // Return OK
                    plugin_return_value = PLUGIN_BYTE_OK;
                } 
                else
                {
                    plugin_return_value = PLUGIN_BYTE_ERROR;
                }
                guiGetBackToCurrentScreen();
            }
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
            }
            break;
        }
        
        // Get 32 random bytes
        case CMD_GET_RANDOM_NUMBER :
        {
            uint8_t randomBytes[32];
            fillArrayWithRandomBytes(randomBytes, 32);
            usbSendMessage(CMD_GET_RANDOM_NUMBER, 32, randomBytes);
            return;
        }  
        
        // Set current date
        case CMD_SET_DATE :
        {
            uint16_t* temp_uint_ptr = (uint16_t*)msg->body.data;
            plugin_return_value = PLUGIN_BYTE_OK;
            setCurrentDate(*temp_uint_ptr);
            break;
        }      
        
        // Set Mooltipass UID
        case CMD_SET_UID :
        {            
            // The packet should contain the UID request key and the UID
            if ((datalen == (UID_REQUEST_KEY_SIZE + UID_SIZE)) && (eeprom_read_byte((uint8_t*)EEP_UID_REQUEST_KEY_SET_ADDR) != UID_REQUEST_KEY_OK_KEY))
            {
                // The request key and uid are adjacent in eeprom memory
                eeprom_write_block((void*)msg->body.data, (void*)EEP_UID_REQUEST_KEY_ADDR, (UID_REQUEST_KEY_SIZE + UID_SIZE));
                eeprom_write_byte((uint8_t*)EEP_UID_REQUEST_KEY_SET_ADDR, UID_REQUEST_KEY_OK_KEY);
                plugin_return_value = PLUGIN_BYTE_OK;
            }
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
            }
            break;
        }   
        
        // Get Mooltipass UID
        case CMD_GET_UID :
        {
            // The packet should contain the UID request key and the UID
            if ((datalen == UID_REQUEST_KEY_SIZE) && (eeprom_read_byte((uint8_t*)EEP_UID_REQUEST_KEY_SET_ADDR) == UID_REQUEST_KEY_OK_KEY))
            {
                // Check uid request key
                if (checkMooltipassPassword(msg->body.data, (void*)EEP_UID_REQUEST_KEY_ADDR, UID_REQUEST_KEY_SIZE) == TRUE)
                {
                    uint8_t mooltipass_uid[UID_SIZE];
                    eeprom_read_block((void*)mooltipass_uid, (void*)EEP_UID_ADDR, sizeof(mooltipass_uid));
                    usbSendMessage(CMD_GET_UID, sizeof(mooltipass_uid), mooltipass_uid);
                    return;
                } 
                else
                {
                    plugin_return_value = PLUGIN_BYTE_ERROR;
                }
            }
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
            }
            break;
        }       
        
        // set password bootkey
        case CMD_SET_BOOTLOADER_PWD :
        {
            if ((eeprom_read_byte((uint8_t*)EEP_BOOT_PWD_SET) != BOOTLOADER_PWDOK_KEY) && (datalen == PACKET_EXPORT_SIZE))
            {
                eeprom_write_block((void*)msg->body.data, (void*)EEP_BOOT_PWD, PACKET_EXPORT_SIZE);
                eeprom_write_word((uint16_t*)EEP_BACKUP_BOOTKEY_ADDR, CORRECT_BOOTKEY);
                eeprom_write_byte((uint8_t*)EEP_BOOT_PWD_SET, BOOTLOADER_PWDOK_KEY);
                plugin_return_value = PLUGIN_BYTE_OK;
            }
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
            }
            break;
        }
        
        // Jump to bootloader
        case CMD_JUMP_TO_BOOTLOADER :
        {            
            // Mandatory wait for bruteforce
            userViewDelay();
            #ifdef DEV_PLUGIN_COMMS
                // Write "jump to bootloader" key in eeprom
                eeprom_write_word((uint16_t*)EEP_BOOTKEY_ADDR, BOOTLOADER_BOOTKEY);
                // Set bootloader password bool to FALSE
                eeprom_write_byte((uint8_t*)EEP_BOOT_PWD_SET, FALSE);
                // Use WDT to reset the device
                cli();
                wdt_reset();
                wdt_clear_flag();
                wdt_change_enable();
                wdt_enable_2s();
                sei();
                while(1);
            #else
               // Prepare asking confirmation screen
                confirmationText_t temp_conf_text;
                temp_conf_text.lines[0] = readStoredStringToBuffer(ID_STRING_WARNING);
                temp_conf_text.lines[1] = readStoredStringToBuffer(ID_STRING_ALLOW_UPDATE);
                
                if ((eeprom_read_byte((uint8_t*)EEP_BOOT_PWD_SET) == BOOTLOADER_PWDOK_KEY) && (datalen == PACKET_EXPORT_SIZE) && (guiAskForConfirmation(2, &temp_conf_text) == RETURN_OK) && (checkMooltipassPassword(msg->body.data, (void*)EEP_BOOT_PWD, PACKET_EXPORT_SIZE) == TRUE))
                {
                    // Write "jump to bootloader" key in eeprom
                    eeprom_write_word((uint16_t*)EEP_BOOTKEY_ADDR, BOOTLOADER_BOOTKEY);
                    eeprom_write_word((uint16_t*)EEP_BACKUP_BOOTKEY_ADDR, BOOTLOADER_BOOTKEY);
                    // Set bootloader password bool to FALSE
                    eeprom_write_byte((uint8_t*)EEP_BOOT_PWD_SET, FALSE);
                    // Use WDT to reset the device
                    cli();
                    wdt_reset();
                    wdt_clear_flag();
                    wdt_change_enable();
                    wdt_enable_2s();
                    sei();
                    while(1);
                }
                
                // Return to last screen
                guiGetBackToCurrentScreen();
            #endif
        }

        // Development commands
#ifdef  DEV_PLUGIN_COMMS
        // erase eeprom
        case CMD_ERASE_EEPROM :
        {
            eraseFlashUsersContents();
            firstTimeUserHandlingInit();
            plugin_return_value = PLUGIN_BYTE_OK;
            break;
        }

        // erase flash
        case CMD_ERASE_FLASH :
        {
            eraseFlashUsersContents();
            plugin_return_value = PLUGIN_BYTE_OK;
            break;
        }

        // erase eeprom
        case CMD_ERASE_SMC :
        {
            if (getSmartCardInsertedUnlocked() == TRUE)
            {
                eraseSmartCard();
                plugin_return_value = PLUGIN_BYTE_OK;
            }
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
            }
            break;
        }

        case CMD_DRAW_BITMAP :
        {
            usbPrintf_P(PSTR("draw bitmap file %d\n"), msg->body.data[0]);
            if (msg->body.data[3] != 0)     // clear
            {
                oledWriteActiveBuffer();
                oledClear();
                oledBitmapDrawFlash(msg->body.data[1], msg->body.data[2], msg->body.data[0], 0);
            }
            else
            {
                // don't clear, overlay active screen
                oledWriteActiveBuffer();
                oledBitmapDrawFlash(msg->body.data[1], msg->body.data[2], msg->body.data[0], 0);
            }
            return;
        }
        
        case CMD_CLONE_SMARTCARD :
        {
            uint16_t pin_code = SMARTCARD_DEFAULT_PIN;
            if (cloneSmartCardProcess(&pin_code) == RETURN_OK)
            {
                plugin_return_value = PLUGIN_BYTE_OK;
            } 
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
            }
            break;
        }

        case CMD_SET_FONT :
        {
            usbPrintf_P(PSTR("set font file %d\n"), msg->body.data[0]);
            oledSetFont(msg->body.data[0]);

            if (datalen > 1) {
                usbPrintf_P(PSTR("testing string \"%s\"\n"), (char *)&msg->body.data[1]);
                oledDisplayOtherBuffer();
                oledWriteActiveBuffer();
                oledClear();
                oledPutstr((char *)&msg->body.data[1]);
            }

            return;
        }
        case CMD_STACK_FREE:
            
            usbPutstr("Stack Free ");
            int_to_string(stackFree(),stack_str);
            usbPutstr(stack_str);
            usbPutstr(" bytes\n");
        return;

        case CMD_USB_KEYBOARD_PRESS:
            plugin_return_value = PLUGIN_BYTE_OK;
            if(datalen == 2)
            {
                usbKeyboardPress(msg->body.data[0], msg->body.data[1]);
            }
            else
            { 
                plugin_return_value = PLUGIN_BYTE_ERROR;
            }
            break;
#endif

        default :   return;
    }
    
    // Return an answer that was defined before calling break
    usbSendMessage(datacmd, 1, &plugin_return_value);
}

