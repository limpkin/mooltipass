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
#include "usb.h"


/*! \fn     displayCredentialAtSlot(uint8_t slot, const char* text)
*   \brief  Display text at a given slot when choosing between credentials/favorites
*   \param  slot                The slot number
*   \param  text                The text to display
*/
void displayCredentialAtSlot(uint8_t slot, char* text)
{
    int8_t yoffset = 0;
    
    if (slot & 0x08)
    {
        yoffset =  (slot & 0x02)? -10:10;
    }
    
    oledPutstrXY((slot & 0x01)*0xFF, (slot & 0x02)*23 + yoffset, (slot & 0x01)*OLED_RIGHT, text);
}

/*! \fn     guiAskForLoginSelect(pNode* p, cNode* c, uint16_t parentNodeAddress)
*   \brief  Ask for user login selection / approval
*   \param  p                   Pointer to a parent node
*   \param  c                   Pointer to a child node
*   \param  parentNodeAddress   Address of the parent node
*   \return Valid child node address or 0 otherwise
*/
uint16_t guiAskForLoginSelect(pNode* p, cNode* c, uint16_t parentNodeAddress)
{
    uint16_t first_child_address, temp_child_address;
    uint16_t picked_child = NODE_ADDR_NULL;
    uint16_t addresses[4];
    uint8_t led_mask;
    int8_t i, j;
    
    // Read the parent node
    readParentNode(p, parentNodeAddress);
    
    // Read its first child address
    first_child_address = p->nextChildAddress;
    
    // Check if there are stored credentials
    if (first_child_address == NODE_ADDR_NULL)
    {
        return NODE_ADDR_NULL;
    }
    
    // Read child node
    readChildNode(c, first_child_address);
    
    // Check if there's only one child, that's a confirmation screen
    if (c->nextChildAddress == NODE_ADDR_NULL)
    {
        confirmationText_t temp_conf_text;
        
        // Prepare asking confirmation screen
        temp_conf_text.lines[0] = readStoredStringToBuffer(ID_STRING_CONFACCESSTO);
        temp_conf_text.lines[1] = (char*)p->service;
        temp_conf_text.lines[2] = readStoredStringToBuffer(ID_STRING_WITHTHISLOGIN);
        temp_conf_text.lines[3] = (char*)c->login;
        
        // Prompt user for confirmation
        if(guiAskForConfirmation(4, &temp_conf_text) == RETURN_OK)
        {
            picked_child = first_child_address;
        }
    }
    else
    {
        temp_child_address = first_child_address;
        uint8_t action_chosen = FALSE;
        
        while (action_chosen == FALSE)
        {
            // Draw asking bitmap
            oledClear();
            oledBitmapDrawFlash(0, 0, BITMAP_LOGIN, 0);
            
            // Write domain name on screen
            oledPutstrXY(0, 24, OLED_CENTRE, (char*)p->service);
            
            // Clear led_mask
            led_mask = 0;
            i = 0;
            
            // List logins on screen
            while ((temp_child_address != NODE_ADDR_NULL) && (i != 4))
            {
                // Read child node to get login
                readChildNode(c, temp_child_address);
                
                // Print Login at the correct slot
                displayCredentialAtSlot(i, (char*)c->login);            
                
                // Store address in array, fetch next address
                addresses[i] = temp_child_address;
                temp_child_address = c->nextChildAddress;
                i++;
            }
            
            // If nothing after, hide right arrow
            if ((i != 4) || (c->nextChildAddress == NODE_ADDR_NULL))
            {
                oledFillXY(174, 24, 22, 18, 0x00);
                led_mask |= LED_MASK_RIGHT;
            }
            
            // Light only the available choices
            for (j = i; j < 4; j++)
            {
                led_mask |= (1 << j);
            }
            
            // Display picture
            oledFlipBuffers(0,0);
            
            // Get touched quarter
            j = getTouchedPositionAnswer(led_mask);
            
            // Check its validity, knowing that by default we will return NODE_ADDR_NULL
            if (j == -1)
            {
                // Time out
                action_chosen = TRUE;
            }
            else if (j < i)
            {
                picked_child = addresses[j];
                action_chosen = TRUE;
            }
            else if (j == TOUCHPOS_LEFT)
            {                
                // If there is a previous children, go back 4 indexes
                if (addresses[0] != first_child_address)
                {
                    c->prevChildAddress = addresses[0];
                    for (i = 0; i < 5; i++)
                    {
                        temp_child_address = c->prevChildAddress;
                        readChildNode(c, temp_child_address);
                    }
                }
                else
                {
                    // otherwise, return
                    action_chosen = TRUE;
                }
            }
            else if ((j == TOUCHPOS_RIGHT) && (i == 4) && (c->nextChildAddress != NODE_ADDR_NULL))
            {
                // If there are more nodes to display, let it loop
                // temp_child_address = c->nextChildAddress;
            }
            else
            {
                // Wrong position
                temp_child_address = addresses[0];
            }
        }
    }
        
    // Get back to other screen
    guiGetBackToCurrentScreen();
    return picked_child;
}

/*! \fn     favoriteSelectionScreen(pNode* p, cNode* c)
*   \brief  Screen displayed to let the user choose a favorite
*   \param  p                   Pointer to a parent node
*   \param  c                   Pointer to a child node
*   \return Valid child node address or 0 otherwise
*/
uint16_t favoriteSelectionScreen(pNode* p, cNode* c)
{
    uint16_t picked_child = NODE_ADDR_NULL;
    uint16_t parentAddresses[USER_MAX_FAV];
    uint16_t childAddresses[USER_MAX_FAV];
    uint16_t tempparaddr, tempchildaddr;
    uint8_t action_chosen = FALSE;
    uint8_t nbFavorites = 0;
    uint8_t offset = 0;
    uint8_t led_mask;
    int8_t i, j;
    
    // Browse through the favorites
    for (i = 0; i < USER_MAX_FAV; i++)
    {
        // Read favorite, check that it is valid
        readFav(i, &tempparaddr, &tempchildaddr);
        
        // If so, store it in our know addresses
        if (tempparaddr != NODE_ADDR_NULL)
        {
            parentAddresses[nbFavorites] = tempparaddr;
            childAddresses[nbFavorites++] = tempchildaddr;
        }
    }    
    
    // If no favorite, return
    if (nbFavorites == 0)
    {
        return NODE_ADDR_NULL;
    }
    
    // Loop until the user chooses smth
    while (action_chosen != TRUE)
    {
        // Draw asking bitmap
        oledClear();
        oledBitmapDrawFlash(0, 0, BITMAP_LOGIN, 0);
        
        // Clear led_mask
        led_mask = 0;
        i = 0;
        
        // List logins on screen
        while (((offset + i) < nbFavorites) && (i != 4))
        {
            // Read child node to get login
            readChildNode(c, childAddresses[offset+i]);
            readParentNode(p, parentAddresses[offset+i]);
            
            // Print service / login on screen
            displayCredentialAtSlot(i+((i&0x02)<<2), (char*)c->login);
            displayCredentialAtSlot(i+((~i&0x02)<<2), (char*)p->service);
            
            // Increment i
            i++;
        }
        
        // If nothing after, hide right arrow
        if ((i != 4) || ((offset+i) == nbFavorites))
        {
            oledFillXY(174, 24, 22, 18, 0x00);
            led_mask |= LED_MASK_RIGHT;
        }
        
        // Light only the available choices
        for (j = i; j < 4; j++)
        {
            led_mask |= (1 << j);
        }
        
        // Display picture
        oledFlipBuffers(0,0);
        
        // Get touched quarter
        j = getTouchedPositionAnswer(led_mask);
        
        // Check its validity, knowing that by default we will return NODE_ADDR_NULL
        if (j == -1)
        {
            // Time out
        }
        else if (j < i)
        {
            // Valid choice
            picked_child = childAddresses[offset+j];
            action_chosen = TRUE;
        }
        else if (j == TOUCHPOS_LEFT)
        {
            if (offset > 0)
            {
                offset -= 4;
            } 
            else
            {
                // User wants to go back
                action_chosen = TRUE;                
            }
        }
        else if ((j == TOUCHPOS_RIGHT) && (i == 4) && ((offset+i) != nbFavorites))
        {
            // If there are more nodes to display
            offset += 4;
        }
    }
    
    // Return selected child
    return picked_child;
}