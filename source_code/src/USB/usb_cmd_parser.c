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
#include "usb_cmd_parser.h"
#include "userhandling.h"
#include "node_mgmt.h"
#include <string.h>
#include <stdint.h>
#include "oledmp.h"
#include "usb.h"


/*! \fn     checkTextField(uint8_t* data, uint8_t len)
*   \brief  Check that the sent text is correct
*   \param  data    Pointer to the data
*   \param  len     Length of the text
*   \param  max_len Max length allowed
*   \return If the sent text is ok
*/
RET_TYPE checkTextField(uint8_t* data, uint8_t len, uint8_t max_len)
{
    if ((len > max_len) || (len == 0) || (len != strlen((char*)data)+1))
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
    // Use message structure
    usbMsg_t* msg = (usbMsg_t*)incomingData;;
    
    // Get data len
    uint8_t datalen = msg->len;

    // Get data cmd
    uint8_t datacmd = msg->cmd;
    
    // Temp ret_type
    RET_TYPE temp_rettype;

    USBOLEDDPRINTF_P(PSTR("usb: rx cmd 0x%02x len %u\n"), datacmd, datalen);
    #ifdef USB_DEBUG_OUTPUT_OLED_MORE
        for (uint8_t ind=0; ind<8 && ind<2+datalen; ind++) 
        {
            USBOLEDDPRINTF_P(PSTR("0x%02x "), incomingData[ind]);
        }
        USBOLEDDPRINTF_P(PSTR("\n"));
    #endif

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
                // Wrong data length
                incomingData[0] = 0x00;
                pluginSendMessage(CMD_CONTEXT, 1, (char*)incomingData);
                USBOLEDDPRINTF_P(PSTR("setCtx: len %d too big\n"), datalen);
            } 
            else
            {
                if (setCurrentContext(msg->body, datalen) == RETURN_OK)
                {
                    // Found context
                    incomingData[0] = 0x01;
                    pluginSendMessage(CMD_CONTEXT, 1, (char*)incomingData);
                } 
                else
                {
                    // Didn't find context
                    incomingData[0] = 0x00;
                    pluginSendMessage(CMD_CONTEXT, 1, (char*)incomingData);
                }
            }
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
                incomingData[0] = 0x00;
                pluginSendMessage(CMD_GET_LOGIN, 1, (char*)incomingData);
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
                incomingData[0] = 0x00;
                pluginSendMessage(CMD_GET_PASSWORD, 1, (char*)incomingData);
            }
            break;
            
        // set login
        case CMD_SET_LOGIN :
            if (checkTextField(msg->body, datalen, RAWHID_RX_SIZE - HID_DATA_START) == RETURN_NOK)
            {
                // Wrong data length
                incomingData[0] = 0x00;
                pluginSendMessage(CMD_SET_LOGIN, 1, (char*)incomingData);
                break;
            } 
            if (setLoginForContext(msg->body, datalen) == RETURN_OK)
            {
                incomingData[0] = 0x01;                
            } 
            else
            {
                incomingData[0] = 0x00;
            }
            pluginSendMessage(CMD_SET_LOGIN, 1, (char*)incomingData);
            break;
        
        // set password
        case CMD_SET_PASSWORD :
            if (checkTextField(msg->body, datalen, NODE_CHILD_SIZE_OF_PASSWORD) == RETURN_NOK)
            {
                // Wrong data length
                incomingData[0] = 0x00;
                pluginSendMessage(CMD_SET_PASSWORD, 1, (char*)incomingData);
                USBOLEDDPRINTF_P(PSTR("set pass: len %d invalid\n"), datalen);
                break;
            } 
            if (setPasswordForContext(msg->body, datalen) == RETURN_OK)
            {
                incomingData[0] = 0x01;                
            } 
            else
            {
                USBOLEDDPRINTF_P(PSTR("set pass: failed\n"));
                incomingData[0] = 0x00;
            }
            pluginSendMessage(CMD_SET_PASSWORD, 1, (char*)incomingData);
            break;
        
        // check password
        case CMD_CHECK_PASSWORD :
            if (checkTextField(msg->body, datalen, NODE_CHILD_SIZE_OF_PASSWORD) == RETURN_NOK)
            {
                // Wrong data length
                incomingData[0] = 0x00;
                pluginSendMessage(CMD_CHECK_PASSWORD, 1, (char*)incomingData);
                break;
            } 
            temp_rettype = checkPasswordForContext(msg->body, datalen);
            if (temp_rettype == RETURN_PASS_CHECK_NOK)
            {
                incomingData[0] = 0x00;                
            } 
            else if(temp_rettype == RETURN_PASS_CHECK_OK)
            {
                incomingData[0] = 0x01;
            }
            else
            {
                incomingData[0] = 0x02;                
            }
            pluginSendMessage(CMD_CHECK_PASSWORD, 1, (char*)incomingData);
            break;
        
        // set password
        case CMD_ADD_CONTEXT :
            if (checkTextField(msg->body, datalen, NODE_PARENT_SIZE_OF_SERVICE) == RETURN_NOK)
            {
                // Wrong data length
                incomingData[0] = 0x00;
                pluginSendMessage(CMD_ADD_CONTEXT, 1, (char*)incomingData);
                USBOLEDDPRINTF_P(PSTR("set context: len %d invalid\n"), datalen);
                break;
            } 
            if (addNewContext(msg->body, datalen) == RETURN_OK)
            {
                incomingData[0] = 0x01;                
            } 
            else
            {
                USBOLEDDPRINTF_P(PSTR("add context: failed\n"));
                incomingData[0] = 0x00;
            }
            pluginSendMessage(CMD_ADD_CONTEXT, 1, (char*)incomingData);
            break;

        default : break;
    }
}

