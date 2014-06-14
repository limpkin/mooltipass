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
#include <string.h>
#include <stdint.h>
#include "usb.h"
#include "oledmp.h"


/*! \fn     usbProcessIncoming(uint8_t* incomingData)
*   \brief  Process the incoming USB packet
*   \param  incomingData    Pointer to the packet (can be overwritten!)
*/
void usbProcessIncoming(uint8_t* incomingData)
{   
    // Get data len
    uint8_t datalen = incomingData[HID_LEN_FIELD];

    // Get data cmd
    uint8_t datacmd = incomingData[HID_TYPE_FIELD];

    #ifdef DEBUG_USB
        printf_P(PSTR("usb: rx cmd 0x%02x len %u\n"), datacmd, datalen);
    #endif
    #ifdef DEBUG_USB_MORE
        for (uint8_t ind=0; ind<8 && ind<2+datalen; ind++) 
        {
            printf_P(PSTR("0x%02x "), incomingData[ind]);
        }
        printf_P(PSTR("\n"));
    #endif

//    usbPrintf_P(PSTR("Data Received cmd: %i"), datacmd);
    
//     if (incomingData[0] == 'a')
//     {
//         usbPutstr("lapin");
//         //usbKeybPutStr("lapin");
//     }

//    usbKeybPutStr("   ");
//

    switch(datacmd)
    {
        // ping command
        case CMD_PING :
            pluginSendMessage(CMD_PING, 0, (char*)incomingData);
            #ifdef DEBUG_USB
                printf_P(PSTR("usb: tx 0x%02x len %d\n"), incomingData[1], incomingData[0]);
            #endif
            break;

        // version command
        case CMD_VERSION :
            incomingData[0] = 0x01;
            incomingData[1] = 0x01;
            #ifdef DEBUG_USB
            printf_P(PSTR("usb: tx 0x%02x len %d\n"), incomingData[1], incomingData[0]);
            #endif
            pluginSendMessage(CMD_VERSION, 2, (char*)incomingData);
            break;
            
        // context command
        case CMD_CONTEXT :
            if ((datalen > RAWHID_RX_SIZE - HID_DATA_START) || (datalen == 0))
            {
                // Wrong data length
                #ifdef DEBUG_USB
                    printf_P(PSTR("setCtx: len %d too big\n"), datalen);
                #endif
                incomingData[0] = 0x00;
                pluginSendMessage(CMD_CONTEXT, 1, (char*)incomingData);
            } 
            else
            {
                if (setCurrentContext(incomingData+HID_DATA_START, datalen) == RETURN_OK)
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
            if (getLoginForContext((char *)incomingData) == RETURN_OK)
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
            if (getPasswordForContext((char *)incomingData) == RETURN_OK)
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
            if ((datalen > RAWHID_RX_SIZE - HID_DATA_START) || (datalen == 0))
            {
                // Wrong data length
                incomingData[0] = 0x00;
                pluginSendMessage(CMD_SET_LOGIN, 1, (char*)incomingData);
            } 
            if (setLoginForContext(incomingData, datalen) == RETURN_OK)
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
            if ((datalen > RAWHID_RX_SIZE - HID_DATA_START) || (datalen == 0))
            {
                // Wrong data length
                incomingData[0] = 0x00;
                pluginSendMessage(CMD_SET_PASSWORD, 1, (char*)incomingData);
            } 
            if (setPasswordForContext(incomingData, datalen) == RETURN_OK)
            {
                incomingData[0] = 0x01;                
            } 
            else
            {
                incomingData[0] = 0x00;
            }
            pluginSendMessage(CMD_SET_PASSWORD, 1, (char*)incomingData);
            break;

        default : break;
    }
}

