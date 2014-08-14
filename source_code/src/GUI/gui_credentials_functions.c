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
/*!  \file     gui_credentials_functions.c
*    \brief    General user interface - credentials functions
*    Created:  22/6/2014
*    Author:   Mathieu Stephan
*/
#include "touch_higher_level_functions.h"
#include "gui_screen_functions.h"
#include "gui_basic_functions.h"
#include "node_mgmt.h"
#include "defines.h"
#include "oledmp.h"
#include "anim.h"
#include "gui.h"


/*! \fn     informGuiOfCurrentContext(char* context)
*   \param  context String of the context
*   \brief  Inform the GUI of the current context
*/
void informGuiOfCurrentContext(char* context)
{
}

/*! \fn     guiAskForLoginSelect(mgmtHandle* h, pNode* p, cNode* c, uint16_t parentNodeAddress)
*   \brief  Ask for user login selection / approval
*   \param  h                   Pointer to management handle
*   \param  p                   Pointer to a parent node
*   \param  c                   Pointer to a child node
*   \param  parentNodeAddress   Address of the parent node
*   \return Valid child node address or 0 otherwise
*/
uint16_t guiAskForLoginSelect(mgmtHandle* h, pNode* p, cNode* c, uint16_t parentNodeAddress)
{
    uint16_t temp_child_address;
    uint16_t addresses[4];
    uint8_t led_mask;
    int8_t i = 0;
    int8_t j;
    
    // Read the parent node
    if (readParentNode(h, p, parentNodeAddress) != RETURN_OK)
    {
        return NODE_ADDR_NULL;
    }
    
    // Read child address
    temp_child_address = p->nextChildAddress;
    
    // Check if there are stored credentials
    if (temp_child_address == NODE_ADDR_NULL)
    {
        return NODE_ADDR_NULL;
    }
    
    // Check if there's only one child
    if (readChildNode(h, c, temp_child_address) != RETURN_OK)
    {
        return NODE_ADDR_NULL;
    }
    if (c->nextChildAddress == NODE_ADDR_NULL)
    {
        confirmationText_t temp_conf_text;
        
        // Prepare asking confirmation screen
        temp_conf_text.line1 = PSTR("Confirm login for");
        temp_conf_text.line2 = (char*)p->service;
        temp_conf_text.line3 = PSTR("with these credentials:");
        temp_conf_text.line4 = (char*)c->login;
        
        // Prompt user for confirmation
        if(guiAskForConfirmation(4, &temp_conf_text) == RETURN_OK)
        {
            // Get back to other screen
            guiGetBackToCurrentScreen();
            return temp_child_address;
        }
        else
        {
            // Get back to other screen
            guiGetBackToCurrentScreen();
            return NODE_ADDR_NULL;
        }
    }
    else
    {
        uint8_t action_chosen = FALSE;
        
        while (action_chosen != TRUE)
        {
            // Draw asking bitmap
            oledClear();
            oledBitmapDrawFlash(0, 0, BITMAP_LOGIN, 0);
            
            // Write domain name on screen
            oledPutstrXY(0, 24, OLED_CENTRE, (char*)p->service);
            
            // Clear led_mask
            led_mask = 0;
            
            // List logins on screen
            while ((temp_child_address != NODE_ADDR_NULL) && (i != 4))
            {
                // Read child node to get login
                if (readChildNode(h, c, temp_child_address) != RETURN_OK)
                {
                    return NODE_ADDR_NULL;
                }
                
                // Print login on screen
                if (i == 0)
                {
                    //oledPutstrXY(72, 0, OLED_RIGHT, (char*)c->login);
                    oledPutstrXY(0, 4, OLED_LEFT, (char*)c->login);
                    
                    // Cover left arrow if there's no predecessor
                    if (c->prevChildAddress == NODE_ADDR_NULL)
                    {
                        led_mask |= LED_MASK_LEFT;
                        oledFillXY(60, 24, 22, 18, 0x00);
                    }
                }
                else if (i == 1)
                {
                    //oledPutstrXY(184, 0, OLED_LEFT, (char*)c->login);
                    oledPutstrXY(255, 4, OLED_RIGHT, (char*)c->login);
                }
                else if (i == 2)
                {
                    //oledPutstrXY(72, 54, OLED_RIGHT, (char*)c->login);
                    oledPutstrXY(0, 48, OLED_LEFT, (char*)c->login);
                }
                else
                {
                    //oledPutstrXY(184, 54, OLED_LEFT, (char*)c->login);
                    oledPutstrXY(255, 48, OLED_RIGHT, (char*)c->login);
                }
                
                // Store address in array, fetch next address
                addresses[i] = temp_child_address;
                temp_child_address = c->nextChildAddress;
                i++;
            }
            
            // Update led_mask & bitmap
            if ((i != 4) || (c->nextChildAddress == NODE_ADDR_NULL))
            {
                led_mask |= LED_MASK_RIGHT;
                oledFillXY(174, 24, 22, 18, 0x00);
            }
            for (j = i; j < 4; j++)
            {
                led_mask |= (1 << j);
            }
            
            // Display picture
            oledFlipBuffers(0,0);
            
            // Set temp_child_address to last address
            temp_child_address = addresses[i-1];
            
            // Get touched quarter and check its validity
            j = getTouchedPositionAnswer(led_mask);
            if (j == -1)
            {
                // Time out, return nothing
                temp_child_address = NODE_ADDR_NULL;
                action_chosen = TRUE;
            }
            else if (j < i)
            {
                temp_child_address = addresses[j];
                action_chosen = TRUE;
            }
            else if (j == TOUCHPOS_LEFT)
            {
                // Get back to the initial child
                while ((i--) > 1)
                {
                    temp_child_address = c->prevChildAddress;
                    readChildNode(h, c, temp_child_address);
                }
                // If there is a previous child, go back 4 indexes
                if (c->prevChildAddress != NODE_ADDR_NULL)
                {
                    i = 4;
                    while(i--)
                    {
                        temp_child_address = c->prevChildAddress;
                        readChildNode(h, c, temp_child_address);
                    }
                }
                i = 0;
            }
            else if ((j == TOUCHPOS_RIGHT) && (i == 4) && (c->nextChildAddress != NODE_ADDR_NULL))
            {
                // If there are more nodes to display
                temp_child_address = c->nextChildAddress;
                i = 0;
            }
            else
            {
                // Wrong position, get back to the initial child
                while ((i--) > 1)
                {
                    temp_child_address = c->prevChildAddress;
                    readChildNode(h, c, temp_child_address);
                }
            }
        }
        
        // Get back to other screen
        guiGetBackToCurrentScreen();
    }

    return temp_child_address;
}
