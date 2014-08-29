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
#include "gui_smartcard_functions.h"
#include "logic_fwflash_storage.h"
#include "gui_screen_functions.h"
#include "logic_aes_and_comms.h"
#include "eeprom_addresses.h"
#include "watchdog_driver.h"
#include "logic_smartcard.h"
#include "usb_cmd_parser.h"
#include "logic_eeprom.h"
#include <avr/eeprom.h>
#include "mooltipass.h"
#include "node_mgmt.h"
#include "flash_mem.h"
#include "version.h"
#include <string.h>
#include "delays.h"
#include "oledmp.h"
#include "utils.h"
#include "stack.h"
#include "usb.h"

// Bool to specify if we're writing user flash space
uint8_t flash_import_user_space = FALSE;
// Operation unique identifier to know the current approved flash action
uint8_t currentFlashOpUid;
// One import/export address that may be used
uint16_t flashOpCurAddr1;
// Another import/export address that may be used
uint16_t flashOpCurAddr2;


/*! \fn     checkTextField(uint8_t* data, uint8_t len)
*   \brief  Check that the sent text is correct
*   \param  data    Pointer to the data
*   \param  len     Length of the text
*   \param  max_len Max length allowed
*   \return If the sent text is ok
*/
RET_TYPE checkTextField(uint8_t* data, uint8_t len, uint8_t max_len)
{
    if ((len > max_len) || (len == 0) || (len != strlen((char*)data)+1) || (len > (RAWHID_RX_SIZE-HID_DATA_START)))
    {
        return RETURN_NOK;
    }
    else
    {
        return RETURN_OK;
    }
}

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
        currentFlashOpUid = opUID;
        *pluginAnswer = PLUGIN_BYTE_OK;
    }
}

/*! \fn     usbProcessIncoming(uint8_t* incomingData)
*   \brief  Process the incoming USB packet
*   \param  incomingData    Pointer to the packet (can be overwritten!)
*/
void usbProcessIncoming(uint8_t* incomingData)
{
    // Temp plugin return value
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

    switch(datacmd)
    {
        // ping command
        case CMD_PING :
        {
            usbSendMessage(0, 6, msg);
            return;
        }

        // version command
        case CMD_VERSION :
        {
            msg->len = 3; // len + cmd + FLASH_CHIP
            msg->cmd = CMD_VERSION;
            msg->body.data[0] = FLASH_CHIP;
            msg->len += getVersion((char*)&msg->body.data[1], sizeof(msg->body.data) - 1);
            usbSendMessage(0, msg->len, msg);
            return;
        }

#ifdef USB_FEATURE_PLUGIN_COMMS
        // context command
        case CMD_CONTEXT :
        {
            if (checkTextField(msg->body.data, datalen, NODE_PARENT_SIZE_OF_SERVICE) == RETURN_NOK)
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
                USBPARSERDEBUGPRINTF_P(PSTR("setCtx: len %d too big\n"), datalen);
            }
            else if (getSmartCardInsertedUnlocked() != TRUE)
            {
                plugin_return_value = PLUGIN_BYTE_NOCARD;
                USBPARSERDEBUGPRINTF_P(PSTR("set context: no card\n"));                
            }
            else if (setCurrentContext(msg->body.data, datalen) == RETURN_OK)
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
            if (checkTextField(msg->body.data, datalen, NODE_CHILD_SIZE_OF_LOGIN) == RETURN_NOK)
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
                USBPARSERDEBUGPRINTF_P(PSTR("set login: \"%s\" checkTextField failed\n"),msg->body.data);
            }
            else if (setLoginForContext(msg->body.data, datalen) == RETURN_OK)
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
            if (checkTextField(msg->body.data, datalen, NODE_CHILD_SIZE_OF_PASSWORD) == RETURN_NOK)
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
                USBPARSERDEBUGPRINTF_P(PSTR("set pass: len %d invalid\n"), datalen);
            }
            else if (setPasswordForContext(msg->body.data, datalen) == RETURN_OK)
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
            if (checkTextField(msg->body.data, datalen, NODE_CHILD_SIZE_OF_PASSWORD) == RETURN_NOK)
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
                break;
            }
            temp_rettype = checkPasswordForContext(msg->body.data, datalen);
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

        // set password
        case CMD_ADD_CONTEXT :
        {
            if (checkTextField(msg->body.data, datalen, NODE_PARENT_SIZE_OF_SERVICE) == RETURN_NOK)
            {
                // Check field
                plugin_return_value = PLUGIN_BYTE_ERROR;
                USBPARSERDEBUGPRINTF_P(PSTR("set context: len %d invalid\n"), datalen);
            }
            else if (addNewContext(msg->body.data, datalen) == RETURN_OK)
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
        
        // set password bootkey
        case CMD_SET_BOOTLOADER_PWD :
        {
            if ((eeprom_read_byte((uint8_t*)EEP_BOOT_PWD_SET) != BOOTLOADER_PWDOK_KEY) && (datalen == PACKET_EXPORT_SIZE))
            {
                eeprom_write_block((void*)msg->body.data, (void*)EEP_BOOT_PWD, PACKET_EXPORT_SIZE);
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
            #ifndef DEV_PLUGIN_COMMS
                uint8_t temp_buffer[PACKET_EXPORT_SIZE];
            #endif
            
            // Mandatory wait for bruteforce
            userViewDelay();
            #ifdef DEV_PLUGIN_COMMS
                // Write "jump to bootloader" key in eeprom
                eeprom_write_word((uint16_t*)EEP_BOOTKEY_ADDR, BOOTLOADER_BOOTKEY);
                // Use WDT to reset the device
                cli();
                wdt_reset();
                wdt_clear_flag();
                wdt_change_enable();
                wdt_enable_2s();
                sei();
                while(1);
            #else
                if ((eeprom_read_byte((uint8_t*)EEP_BOOT_PWD_SET) == BOOTLOADER_PWDOK_KEY) && (datalen == PACKET_EXPORT_SIZE))
                {
                    eeprom_read_block((void*)temp_buffer, (void*)EEP_BOOT_PWD, PACKET_EXPORT_SIZE);
                    if (memcmp((void*)temp_buffer, (void*)msg->body.data, PACKET_EXPORT_SIZE) == 0)
                    {
                        // Write "jump to bootloader" key in eeprom
                        eeprom_write_word((uint16_t*)EEP_BOOTKEY_ADDR, BOOTLOADER_BOOTKEY);
                        // Use WDT to reset the device
                        cli();
                        wdt_reset();
                        wdt_clear_flag();
                        wdt_change_enable();
                        wdt_enable_2s();
                        sei();
                        while(1);
                    }
                }
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
            if (cloneSmartCardProcess(SMARTCARD_DEFAULT_PIN) == RETURN_OK)
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
                oledFlipBuffers(0,0);
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
#endif

        default :   return;
    }
    usbSendMessage(datacmd, 1, &plugin_return_value);
}

