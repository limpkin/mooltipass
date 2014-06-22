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
#include "eeprom_addresses.h"
#include "usb_cmd_parser.h"
#include "userhandling.h"
#include <avr/eeprom.h>
#include "flash_mem.h"
#include "node_mgmt.h"
#include <string.h>
#include <stdint.h>
#include "usb.h"

#define FLASH_SIZE (PAGE_COUNT * (uint32_t)BYTES_PER_PAGE)
#define EXPORT_PACKET_SIZE 62   // number of data bytes per export message

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
    
    // temp loops
    uint16_t i,j;

    // Debug comms
    USBDEBUGPRINTF_P(PSTR("usb: rx cmd 0x%02x len %u\n"), datacmd, datalen);

    switch(datacmd)
    {
        // ping command
        case CMD_PING :
            pluginSendMessage(CMD_PING, 0, (char*)incomingData);
            break;

        // version command
        case CMD_VERSION :
            incomingData[0] = 0x01;
            incomingData[1] = 0x01;
            pluginSendMessage(CMD_VERSION, 2, (char*)incomingData);
            break;
            
        // context command
        case CMD_CONTEXT :
            if (checkTextField(msg->body, datalen, NODE_PARENT_SIZE_OF_SERVICE) == RETURN_NOK)
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
                USBDEBUGPRINTF_P(PSTR("setCtx: len %d too big\n"), datalen);
            } 
            else if (setCurrentContext(msg->body, datalen) == RETURN_OK)
            {
                plugin_return_value = PLUGIN_BYTE_OK;
            }
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
            }
            sendPluginOneByteAnswer(CMD_CONTEXT, PLUGIN_BYTE_ERROR, incomingData);
            break;
            
        // get login
        case CMD_GET_LOGIN :
            if (getLoginForContext((char*)incomingData) == RETURN_OK)
            {
                // Use the buffer to store the login...
                pluginSendMessage(CMD_GET_LOGIN, strlen((char*)incomingData), (char*)incomingData);
            } 
            else
            {
                sendPluginOneByteAnswer(CMD_GET_LOGIN, PLUGIN_BYTE_ERROR, incomingData);
            }
            break;
            
        // get password
        case CMD_GET_PASSWORD :
            if (getPasswordForContext((char*)incomingData) == RETURN_OK)
            {
                pluginSendMessage(CMD_GET_PASSWORD, strlen((char*)incomingData), (char*)incomingData);
            } 
            else
            {
                 sendPluginOneByteAnswer(CMD_GET_PASSWORD, PLUGIN_BYTE_ERROR, incomingData);
            }
            break;
            
        // set login
        case CMD_SET_LOGIN :
            if (checkTextField(msg->body, datalen, NODE_CHILD_SIZE_OF_LOGIN) == RETURN_NOK)
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
            } 
            else if (setLoginForContext(msg->body, datalen) == RETURN_OK)
            {
                plugin_return_value = PLUGIN_BYTE_OK;
            } 
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
            }
            sendPluginOneByteAnswer(CMD_SET_LOGIN, plugin_return_value, incomingData);
            break;
        
        // set password
        case CMD_SET_PASSWORD :
            if (checkTextField(msg->body, datalen, NODE_CHILD_SIZE_OF_PASSWORD) == RETURN_NOK)
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
                USBDEBUGPRINTF_P(PSTR("set pass: len %d invalid\n"), datalen);
            } 
            else if (setPasswordForContext(msg->body, datalen) == RETURN_OK)
            {
                plugin_return_value = PLUGIN_BYTE_OK;
            } 
            else
            {
                plugin_return_value = PLUGIN_BYTE_ERROR;
                USBDEBUGPRINTF_P(PSTR("set pass: failed\n"));
            }
            sendPluginOneByteAnswer(CMD_CHECK_PASSWORD, plugin_return_value, incomingData);
            break;
        
        // check password
        case CMD_CHECK_PASSWORD :
            if (checkTextField(msg->body, datalen, NODE_CHILD_SIZE_OF_PASSWORD) == RETURN_NOK)
            {
                sendPluginOneByteAnswer(CMD_CHECK_PASSWORD, PLUGIN_BYTE_ERROR, incomingData);
                break;
            } 
            temp_rettype = checkPasswordForContext(msg->body, datalen);
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
            if (checkTextField(msg->body, datalen, NODE_PARENT_SIZE_OF_SERVICE) == RETURN_NOK)
            {
                // Check field
                plugin_return_value = PLUGIN_BYTE_ERROR;
                USBDEBUGPRINTF_P(PSTR("set context: len %d invalid\n"), datalen);
            } 
            else if (addNewContext(msg->body, datalen) == RETURN_OK)
            {
                // We managed to add a new context
                plugin_return_value = PLUGIN_BYTE_OK;             
            } 
            else
            {
                // Couldn't add a new context
                plugin_return_value = PLUGIN_BYTE_ERROR;
                USBDEBUGPRINTF_P(PSTR("add context: failed\n"));
            }
            sendPluginOneByteAnswer(CMD_ADD_CONTEXT, plugin_return_value, incomingData);    
            break;
            
        // export flash contents
        case CMD_EXPORT_FLASH: 
        {
            uint8_t size=EXPORT_PACKET_SIZE;
            for (uint32_t addr=0; addr<FLASH_SIZE; addr+=EXPORT_PACKET_SIZE) 
            {
                if ((FLASH_SIZE-addr) < EXPORT_PACKET_SIZE) 
                {
                    size = (uint8_t)(FLASH_SIZE - addr);
                }
                flashRawRead(incomingData, addr, size);
                pluginSendMessage(CMD_EXPORT_FLASH, size, (char*)incomingData);
            }
            pluginSendMessage(CMD_EXPORT_FLASH_END, 0, (char*)incomingData);
            break;
        }
            
        // export eeprom contents
        case CMD_EXPORT_EEPROM :
            #if EEPROM_SIZE == 1024
                for (i = 0; i < 17; i++)
                {
                    for (j = 0; j < 60; j++)
                    {
                        incomingData[j] = eeprom_read_byte((uint8_t*)j);
                    }
                    pluginSendMessage(CMD_EXPORT_EEPROM, 60, (char*)incomingData);
                }
                for (i = 0; i < 4; i++)
                {
                    incomingData[i] = eeprom_read_byte((uint8_t*)i+(17*60));
                }
                pluginSendMessage(CMD_EXPORT_EEPROM, 4, (char*)incomingData);
                pluginSendMessage(CMD_EXPORT_EEPROM_END, 0, (char*)incomingData);
            #else
                #error "eeprom not supported"
            #endif
            break;

        default : break;
    }
}

