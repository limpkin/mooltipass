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
#include "eeprom_addresses.h"
#include "usb_cmd_parser.h"
#include "userhandling.h"
#include <avr/eeprom.h>
#include "flash_mem.h"
#include "flash.h"
#include "node_mgmt.h"
#include <string.h>
#include <stdint.h>
#include "usb.h"
#include "store.h"
#include "oledmp.h"

// Current address in flash we need to export
uint32_t current_flash_export_addr = 0;
// Current address in eeprom we need to export
uint16_t current_eeprom_export_addr = 0;
// Bool to specify if we're writing user flash space
uint8_t flash_import_user_space = FALSE;
// Current page in flash where we're importing
uint16_t current_flash_import_page = 0;
// Temporary counter to align our data to flash pages
uint16_t current_flash_import_page_pos = 0;
// Bool to specify if user approved flash import
uint8_t flash_import_approved = FALSE;


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

/*! \fn     sendPluginOneByteAnswer(uint8_t command, uint8_t answer, uint8_t* data)
*   \brief  Send a one byte message to the plugin
*   \param  command The command we're answering
*   \param  answer  The answer
*   \param  data    Pointer to the buffer
*/
void sendPluginOneByteAnswer(uint8_t command, uint8_t answer, uint8_t* data)
{
    data[0] = answer;
    pluginSendMessage(command, 1, (char*)data);
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
    
    // Temp ret_type
    RET_TYPE temp_rettype;

    // Debug comms
    //USBDEBUGPRINTF_P(PSTR("usb: rx cmd 0x%02x len %u\n"), datacmd, datalen);

    switch(datacmd)
    {
        // ping command
        case CMD_PING :
            memcpy((void*)incomingData, (void*)msg->body.data, 2);
            memcpy((void*)incomingData+2, (void*)msg->body.data+2, 2);
            pluginSendMessage(CMD_PING, 4, (char*)incomingData);
            break;

        // version command
        case CMD_VERSION :
            incomingData[0] = 0x01;
            incomingData[1] = 0x01;
            pluginSendMessage(CMD_VERSION, 2, (char*)incomingData);
            break;
            
        // context command
        case CMD_CONTEXT :
            if (checkTextField(msg->body.data, datalen, NODE_PARENT_SIZE_OF_SERVICE) == RETURN_NOK)
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
                USBDEBUGPRINTF_P(PSTR("setCtx: len %d too big\n"), datalen);
            } 
            else if (setCurrentContext(msg->body.data, datalen) == RETURN_OK)
            {
                plugin_return_value = PLUGIN_BYTE_OK;
                USBDEBUGPRINTF_P(PSTR("set context: \"%s\" ok\n"), msg->body.data);
            }
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
                USBDEBUGPRINTF_P(PSTR("set context: \"%s\" failed\n"), msg->body.data);
            }
            sendPluginOneByteAnswer(CMD_CONTEXT, plugin_return_value, incomingData);
            break;
            
        // get login
        case CMD_GET_LOGIN :
            if (getLoginForContext((char*)incomingData) == RETURN_OK)
            {
                // Use the buffer to store the login...
                pluginSendMessage(CMD_GET_LOGIN, strlen((char*)incomingData), (char*)incomingData);
                USBDEBUGPRINTF_P(PSTR("get login: \"%s\"\n"),(char *)incomingData);
            } 
            else
            {
                sendPluginOneByteAnswer(CMD_GET_LOGIN, PLUGIN_BYTE_ERROR, incomingData);
                USBDEBUGPRINTF_P(PSTR("get login: failed\n"));
            }
            break;
            
        // get password
        case CMD_GET_PASSWORD :
            if (getPasswordForContext((char*)incomingData) == RETURN_OK)
            {
                pluginSendMessage(CMD_GET_PASSWORD, strlen((char*)incomingData), (char*)incomingData);
                USBDEBUGPRINTF_P(PSTR("get pass: \"%s\"\n"),(char *)incomingData);
            } 
            else
            {
                sendPluginOneByteAnswer(CMD_GET_PASSWORD, PLUGIN_BYTE_ERROR, incomingData);
                USBDEBUGPRINTF_P(PSTR("get pass: failed\n"));
            }
            break;
            
        // set login
        case CMD_SET_LOGIN :
            if (checkTextField(msg->body.data, datalen, NODE_CHILD_SIZE_OF_LOGIN) == RETURN_NOK)
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
                USBDEBUGPRINTF_P(PSTR("set login: \"%s\" checkTextField failed\n"),msg->body.data);
            } 
            else if (setLoginForContext(msg->body.data, datalen) == RETURN_OK)
            {
                plugin_return_value = PLUGIN_BYTE_OK;
                USBDEBUGPRINTF_P(PSTR("set login: \"%s\" ok\n"),msg->body.data);
            } 
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
                USBDEBUGPRINTF_P(PSTR("set login: \"%s\" failed\n"),msg->body.data);
            }
            sendPluginOneByteAnswer(CMD_SET_LOGIN, plugin_return_value, incomingData);
            break;
        
        // set password
        case CMD_SET_PASSWORD :
            if (checkTextField(msg->body.data, datalen, NODE_CHILD_SIZE_OF_PASSWORD) == RETURN_NOK)
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
                USBDEBUGPRINTF_P(PSTR("set pass: len %d invalid\n"), datalen);
            } 
            else if (setPasswordForContext(msg->body.data, datalen) == RETURN_OK)
            {
                plugin_return_value = PLUGIN_BYTE_OK;
                USBDEBUGPRINTF_P(PSTR("set pass: \"%s\" ok\n"),msg->body.data);
            } 
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
                USBDEBUGPRINTF_P(PSTR("set pass: failed\n"));
            }
            sendPluginOneByteAnswer(CMD_SET_PASSWORD, plugin_return_value, incomingData);
            break;
        
        // check password
        case CMD_CHECK_PASSWORD :
            if (checkTextField(msg->body.data, datalen, NODE_CHILD_SIZE_OF_PASSWORD) == RETURN_NOK)
            {
                sendPluginOneByteAnswer(CMD_CHECK_PASSWORD, PLUGIN_BYTE_ERROR, incomingData);
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
            sendPluginOneByteAnswer(CMD_CHECK_PASSWORD, plugin_return_value, incomingData); 
            break;
        
        // set password
        case CMD_ADD_CONTEXT :
            if (checkTextField(msg->body.data, datalen, NODE_PARENT_SIZE_OF_SERVICE) == RETURN_NOK)
            {
                // Check field
                plugin_return_value = PLUGIN_BYTE_ERROR;
                USBDEBUGPRINTF_P(PSTR("set context: len %d invalid\n"), datalen);
            } 
            else if (addNewContext(msg->body.data, datalen) == RETURN_OK)
            {
                // We managed to add a new context
                plugin_return_value = PLUGIN_BYTE_OK;             
                USBDEBUGPRINTF_P(PSTR("add context: \"%s\" ok\n"),msg->body.data);
            } 
            else
            {
                // Couldn't add a new context
                plugin_return_value = PLUGIN_BYTE_ERROR;
                USBDEBUGPRINTF_P(PSTR("add context: \"%s\" failed\n"),msg->body.data);
            }
            sendPluginOneByteAnswer(CMD_ADD_CONTEXT, plugin_return_value, incomingData);    
            break;
            
        // export flash contents
        case CMD_EXPORT_FLASH :
        {
            uint8_t size = PACKET_EXPORT_SIZE;
            
            // Check datalen for arg
            if (datalen != 1)
            {
                USBDEBUGPRINTF_P(PSTR("export: no param\n"));
                break;
            }
            
            // Check if the plugin wants a fresh export
            if (msg->body.data[0] == 0)
            {
                // Export start
                current_flash_export_addr = 0x0000;
            }
            
            // Check if the export address is correct
            if (current_flash_export_addr >= FLASH_SIZE)
            {
                pluginSendMessage(CMD_EXPORT_FLASH_END, 0, (char*)incomingData);
                USBDEBUGPRINTF_P(PSTR("export: end\n"));
                break;
            }
            
            // Check how much data we need
            if ((FLASH_SIZE - current_flash_export_addr) < (uint32_t)PACKET_EXPORT_SIZE)
            {
                size = (uint8_t)(FLASH_SIZE - current_flash_export_addr);
            }
            
            // Get a block of data and send it, increment counter
            flashRawRead(incomingData, current_flash_export_addr, size);
            pluginSendMessageWithRetries(CMD_EXPORT_FLASH, size, (char*)incomingData, 255);
            current_flash_export_addr += size;
            break;
        }            
            
        // export eeprom contents
        case CMD_EXPORT_EEPROM :
        {
            uint8_t size = PACKET_EXPORT_SIZE;
            
            // Check datalen for arg
            if (datalen != 1)
            {
                USBDEBUGPRINTF_P(PSTR("export: no param\n"));
                break;
            }
            
            // Check if the plugin wants a fresh export
            if (msg->body.data[0] == 0)
            {
                // Export start
                current_eeprom_export_addr = 0x0000;
            }
            
            // Check if the export address is correct
            if (current_eeprom_export_addr >= EEPROM_SIZE)
            {
                pluginSendMessage(CMD_EXPORT_EEPROM_END, 0, (char*)incomingData);
                USBDEBUGPRINTF_P(PSTR("export: end\n"));
                break;
            }
            
            // Check how much data we need
            if ((EEPROM_SIZE - current_eeprom_export_addr) < PACKET_EXPORT_SIZE)
            {
                size = (uint8_t)(EEPROM_SIZE - current_eeprom_export_addr);
            }
            
            // Get a block of data and send it, increment counter
            eeprom_read_block(incomingData, (void*)current_eeprom_export_addr, size);
            pluginSendMessageWithRetries(CMD_EXPORT_EEPROM, size, (char*)incomingData, 255);
            current_eeprom_export_addr += size;
            break;
        }
            
        // import flash contents
        case CMD_IMPORT_FLASH_BEGIN :
        {
            // Check datalen for arg
            if (datalen != 1)
            {
                USBDEBUGPRINTF_P(PSTR("import: no param\n"));
                break;
            }
            
            // Check what we want to write
            if (msg->body.data[0] == 0x00)
            {
                flash_import_user_space = TRUE;
                current_flash_import_page = 0x0000;
            } 
            else
            {
                flash_import_user_space = FALSE;
                current_flash_import_page = GRAPHIC_ZONE_PAGE_START;
            }
            
            // Ask for user confirmation
            if (TRUE)
            {
                flash_import_approved = TRUE;
                current_flash_import_page_pos = 0;
                plugin_return_value = PLUGIN_BYTE_OK;
            } 
            else
            {
                flash_import_approved = FALSE;
                plugin_return_value = PLUGIN_BYTE_ERROR;
            }
            sendPluginOneByteAnswer(CMD_IMPORT_FLASH_BEGIN, plugin_return_value, incomingData);
            break;
        }
            
        // import flash contents
        case CMD_IMPORT_FLASH :
        {
            // Check if we actually approved the import, haven't gone over the flash boundaries, if we're correctly aligned page size wise
            if ((flash_import_approved == FALSE) || (current_flash_import_page >= PAGE_COUNT) || (current_flash_import_page_pos + datalen > BYTES_PER_PAGE) || ((flash_import_user_space == FALSE) && (current_flash_import_page >= PAGE_PER_SECTOR)))
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
            }
            else
            {
                flashWriteBuffer(msg->body.data, current_flash_import_page_pos, datalen);
                current_flash_import_page_pos+= datalen;
                
                // If we just filled a page, flush it to the page
                if (current_flash_import_page_pos == BYTES_PER_PAGE)
                {
                    flashWriteBufferToPage(current_flash_import_page);
                    current_flash_import_page_pos = 0;
                    current_flash_import_page++;
                    
                    // If we are importing user contents, skip the graphics zone
                    if ((flash_import_user_space == TRUE) && (current_flash_import_page == GRAPHIC_ZONE_PAGE_START))
                    {
                        //current_flash_import_page = PAGE_PER_SECTOR;
                    }
                }
                plugin_return_value = PLUGIN_BYTE_OK;
            }
            sendPluginOneByteAnswer(CMD_IMPORT_FLASH, plugin_return_value, incomingData);
            break;
        }
            
        // end flash import
        case CMD_EXPORT_FLASH_END :
        {
            flash_import_approved = FALSE;
            sendPluginOneByteAnswer(CMD_EXPORT_FLASH_END, PLUGIN_BYTE_OK, incomingData);
            break;
        }            

        // Development commands
#ifdef  DEV_PLUGIN_COMMS            
        // erase eeprom
        case CMD_ERASE_EEPROM :
        {
            firstTimeUserHandlingInit();
            sendPluginOneByteAnswer(CMD_ERASE_EEPROM, PLUGIN_BYTE_OK, incomingData); 
            break;
        }   
        // erase flash
        case CMD_ERASE_FLASH :
        {
            formatFlash();
            sendPluginOneByteAnswer(CMD_ERASE_FLASH, PLUGIN_BYTE_OK, incomingData); 
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
            sendPluginOneByteAnswer(CMD_ERASE_SMC, plugin_return_value, incomingData); 
            break;
        }   

        case CMD_ALLOCATE_SLOT :
        {
            uint8_t slotId = storeAllocateSlot(msg->body.storeAllocate.size);
            pluginSendMessage(CMD_ALLOCATE_SLOT, 1, (char *)&slotId);
            break;
        }

        case CMD_WRITE_SLOT :
        {
            uint8_t res = storeWriteSlot(msg->body.storeWrite.slotId, datalen-1, msg->body.storeWrite.data);
            incomingData[1] = msg->body.storeWrite.slotId;;
            incomingData[0] = res;
            pluginSendMessage(CMD_WRITE_SLOT, 2, (char *)incomingData);
            break;
        }

        case CMD_ERASE_SLOTS :  // Erase all slots
        {
            uint8_t res = 1;
            storeInit();
            pluginSendMessage(CMD_ERASE_SLOTS, 1, (char *)&res);
            break;
        }

        case CMD_DRAW_SLOT :
            usbPrintf_P(PSTR("slot %d display\n"), incomingData[2]);
            oledBitmapDrawSlot(0, 0, msg->body.data[0], OLED_SCROLL_UP);
            break;

        case CMD_FLASH_READ :
        {
            uint8_t size = msg->body.flashRead.size;
            uint32_t addr = msg->body.flashRead.addr;
            usbPrintf_P(PSTR("read 0x%lx %u bytes\n"), addr, size);
            if (size > (RAWHID_TX_SIZE - 7))
            {
                incomingData[0] = 0;
                pluginSendMessage(CMD_FLASH_READ, 1, (char *)incomingData);
                usbPrintf_P(PSTR("read error size %u > %u\n"), size, RAWHID_TX_SIZE-7);
            }
            if (flashRead(&incomingData[5], addr, size) >= 0)
            {
                incomingData[0] = 1;    // success
                *(uint32_t *)&incomingData[1] = addr;
                pluginSendMessage(CMD_FLASH_READ, size+5, (char *)incomingData);
            }
            else
            {
                incomingData[0] = 0;
                pluginSendMessage(CMD_FLASH_READ, 1, (char *)incomingData);
                usbPrintf_P(PSTR("read error\n"));
            }
            break;
        }


#endif

        default : break;
    }
}

