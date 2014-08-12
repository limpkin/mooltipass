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
#include "watchdog_driver.h"
#include "usb_cmd_parser.h"
#include "userhandling.h"
#include <avr/eeprom.h>
#include <util/delay.h>
#include "mooltipass.h"
#include "node_mgmt.h"
#include "flash_mem.h"
#include <string.h>
#include "oledmp.h"
#include "usb.h"
#include "gui.h"

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
// Current byte in eeprom we're importing
uint16_t current_eeprom_import_pos = 0;
// Bool to specify if user approved flash import
uint8_t flash_import_approved = FALSE;
// Bool to specify if user approved eeprom import
uint8_t eeprom_import_approved = FALSE;
// Bool to specify if user approved flash export
uint8_t flash_export_approved = FALSE;
// Bool to specify if user approved eeprom export
uint8_t eeprom_export_approved = FALSE;


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
            msg->len = 3;
            msg->cmd = CMD_VERSION;
            msg->body.version.major = MOOLT_VERSION_MAJOR;
            msg->body.version.minor = MOOLT_VERSION_MINOR;
            msg->body.version.flash_chip = FLASH_CHIP;
            msg->body.version.build = BUILD_NUMBER;
            usbSendMessage(0, 2+sizeof(msg->body.version), msg);
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
            else if ((getSmartCardInsertedUnlocked() != TRUE) && (guiDisplayInsertSmartCardScreenAndWait() == RETURN_NOK))
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

        // flash export start
        case CMD_EXPORT_FLASH_START :
        {
            if (guiAskForConfirmation(PSTR("Approve flash export?")) == RETURN_OK)
            {
                flash_export_approved = TRUE;
                plugin_return_value = PLUGIN_BYTE_OK;
            } 
            else
            {
                flash_export_approved = FALSE;
                plugin_return_value = PLUGIN_BYTE_ERROR;
            }
            break;
        }

        // export flash contents
        case CMD_EXPORT_FLASH :
        {
            uint8_t size = PACKET_EXPORT_SIZE;

            // Check datalen for arg, check approval status
            if ((datalen != 1) || (flash_export_approved == FALSE))
            {
                return;
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
                usbSendMessage(CMD_EXPORT_FLASH_END, 0, NULL);
                USBPARSERDEBUGPRINTF_P(PSTR("export: end\n"));
                flash_export_approved = FALSE;
                return;
            }

#if 0
            // Check how much data we need in case we're close to the graphics section
            if ((current_flash_export_addr < GRAPHIC_ZONE_START) && ((GRAPHIC_ZONE_START - current_flash_export_addr) < (uint32_t)PACKET_EXPORT_SIZE))
            {
                size = (uint8_t)(GRAPHIC_ZONE_START - current_flash_export_addr);
            }

            // Check how much data we need in case we're close to the flash end
            if ((FLASH_SIZE - current_flash_export_addr) < (uint32_t)PACKET_EXPORT_SIZE)
            {
                size = (uint8_t)(FLASH_SIZE - current_flash_export_addr);
            }
#endif

            // Get a block of data and send it, increment counter
            flashRawRead(incomingData, current_flash_export_addr, size);
            usbSendMessageWithRetries(CMD_EXPORT_FLASH, size, (char*)incomingData, 255);
            current_flash_export_addr += size;

#if 0
            // Skip over the graphics address if we're in that case
            if (current_flash_export_addr == GRAPHIC_ZONE_START)
            {
                current_flash_export_addr = GRAPHIC_ZONE_END;
            }
#endif
            return;
        }
        
        // flash export end
        case CMD_EXPORT_FLASH_END :
        {
            flash_export_approved = FALSE;
            return;
        }

        // flash export start
        case CMD_EXPORT_EEPROM_START :
        {
            if (guiAskForConfirmation(PSTR("Approve eeprom export?")) == RETURN_OK)
            {
                eeprom_export_approved = TRUE;
                plugin_return_value = PLUGIN_BYTE_OK;
            } 
            else
            {
                eeprom_export_approved = FALSE;
                plugin_return_value = PLUGIN_BYTE_ERROR;
            }
            break;
        }

        // export eeprom contents
        case CMD_EXPORT_EEPROM :
        {
            uint8_t size = PACKET_EXPORT_SIZE;

            // Check datalen for arg, check that export has been approved
            if ((datalen != 1) || (eeprom_export_approved == FALSE))
            {
                return;
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
                usbSendMessage(CMD_EXPORT_EEPROM_END, 0, NULL);
                USBPARSERDEBUGPRINTF_P(PSTR("export: end\n"));
                eeprom_export_approved = FALSE;
                return;
            }

            // Check how much data we need
            if ((EEPROM_SIZE - current_eeprom_export_addr) < PACKET_EXPORT_SIZE)
            {
                size = (uint8_t)(EEPROM_SIZE - current_eeprom_export_addr);
            }

            // Get a block of data and send it, increment counter
            eeprom_read_block(incomingData, (void*)current_eeprom_export_addr, size);
            usbSendMessageWithRetries(CMD_EXPORT_EEPROM, size, (char*)incomingData, 255);
            current_eeprom_export_addr += size;
            return;
        }
        
        // end eeprom export
        case CMD_EXPORT_EEPROM_END :
        {
            eeprom_export_approved = FALSE;
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
            if (guiAskForConfirmation(PSTR("Approve flash import?")) == RETURN_OK)
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
            break;
        }

        // import flash contents
        case CMD_IMPORT_FLASH :
        {
            // Check if we actually approved the import, haven't gone over the flash boundaries, if we're correctly aligned page size wise
            if ((flash_import_approved == FALSE) || (current_flash_import_page >= PAGE_COUNT) || (current_flash_import_page_pos + datalen > BYTES_PER_PAGE) || ((flash_import_user_space == FALSE) && (current_flash_import_page >= GRAPHIC_ZONE_PAGE_END)))
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
                flash_import_approved = FALSE;
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
                        current_flash_import_page = GRAPHIC_ZONE_PAGE_END;
                    }
                }
                plugin_return_value = PLUGIN_BYTE_OK;
            }
            break;
        }

        // end flash import
        case CMD_IMPORT_FLASH_END :
        {
            flash_import_approved = FALSE;
            plugin_return_value = PLUGIN_BYTE_OK;
            break;
        }

        // import flash contents
        case CMD_IMPORT_EEPROM_BEGIN :
        {
            // Ask for user confirmation
            if (guiAskForConfirmation(PSTR("Approve eeprom import?")) == RETURN_OK)
            {
                current_eeprom_import_pos = 0;
                eeprom_import_approved = TRUE;
                plugin_return_value = PLUGIN_BYTE_OK;
            }
            else
            {
                eeprom_import_approved = FALSE;
                plugin_return_value = PLUGIN_BYTE_ERROR;
            }
            break;
        }

        // import flash contents
        case CMD_IMPORT_EEPROM :
        {
            if ((eeprom_import_approved == FALSE) || ((current_eeprom_import_pos + datalen) >= EEPROM_SIZE))
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
                eeprom_import_approved = FALSE;
            }
            else
            {
                eeprom_write_block((void*)msg->body.data, (void*)current_eeprom_import_pos, datalen);
                current_eeprom_import_pos+= datalen;
                plugin_return_value = PLUGIN_BYTE_OK;
            }
            break;
        }

        // end eeprom import
        case CMD_IMPORT_EEPROM_END :
        {
            eeprom_import_approved = FALSE;
            plugin_return_value = PLUGIN_BYTE_OK;
            break;
        }
        
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
            _delay_ms(3000);
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
                    eeprom_read_block((void*)temp_buffer, (void*)EEP_NB_KNOWN_CARDS_ADDR, PACKET_EXPORT_SIZE);
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
            if (cloneSmartCard(SMARTCARD_DEFAULT_PIN) == RETURN_OK)
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

#if 0
            oledFlipBuffers(0,0);
            oledWriteActiveBuffer();
            oledClear();
            uint32_t start = millis();
            oledPutstr_P(PSTR("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"));
            oledPutstr_P(PSTR("abcdefghijklmnopqrstuvwxyz:~#$"));
            oledPutstr_P(PSTR("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"));
            oledPutstr_P(PSTR("abcdefghijklmnopqrstuvwxyz:~#$"));
            uint32_t end = millis();
            usbPrintf_P(PSTR("Time to print: %lu msecs\n"),end-start);
#endif
            return;
        }
#endif

        default :   return;
    }
    usbSendMessage(datacmd, 1, &plugin_return_value);
}

