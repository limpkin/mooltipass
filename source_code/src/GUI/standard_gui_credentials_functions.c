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
/*!  \file     standard_gui_credentials_functions.c
*    \brief    General user interface - credentials functions
*    Created:  22/6/2014
*    Author:   Mathieu Stephan
*/
#include <string.h>
#include "smart_card_higher_level_functions.h"
#include "touch_higher_level_functions.h"
#include "gui_credentials_functions.h"
#include "gui_screen_functions.h"
#include "gui_basic_functions.h"
#include "logic_aes_and_comms.h"
#include "timer_manager.h"
#include "oled_wrapper.h"
#include "mini_inputs.h"
#include "node_mgmt.h"
#include "node_mgmt.h"
#include "defines.h"
#include "delays.h"
#include "anim.h"
#include "gui.h"
#include "usb.h"
#ifdef HARDWARE_OLIVIER_V1
// Last first matching parent address we saw
uint16_t last_matching_parent_addr = NODE_ADDR_NULL;
// Last number of matching parent address
uint8_t last_matching_parent_number = 0;
#ifdef MINI_VERSION
// String offset counters for scrolling
uint8_t string_offset_cntrs[3];
// Number of extra chars for current displayed string
uint8_t string_extra_chars[3];
#endif


/*! \fn     displayCredentialAtSlot(uint8_t slot, const char* text)
*   \brief  Display text at a given slot when choosing between credentials/favorites
*   \param  slot                The slot number
*   \param  text                The text to display
*   \param  truncate_index      Index at which we truncate
*/
void displayCredentialAtSlot(uint8_t slot, char* text, uint8_t truncate_index)
{
    char temp_disptext[40];
    int8_t yoffset = 0;
    
    if (slot & 0x08)
    {
        yoffset =  (slot & 0x02)? -10:10;
    }
    
    // Truncate and display string
    memcpy(temp_disptext, text, sizeof(temp_disptext));
    temp_disptext[sizeof(temp_disptext)-1] = 0;
    temp_disptext[truncate_index] = 0;
    oledPutstrXY((slot & 0x01)*0xFF, 1 + (slot & 0x02)*24 + yoffset, (slot & 0x01)*OLED_RIGHT, temp_disptext);
}

/*! \fn     miniIncrementScrolledTexts(void)
*   \brief  Change offset char for the currently displayed strings
*/
#ifdef MINI_VERSION
void miniIncrementScrolledTexts(void)
{
    for (uint8_t i = 0; i < sizeof(string_extra_chars); i++)
    {
        if (string_extra_chars[i] > 0)
        {
            if (string_offset_cntrs[i]++ == string_extra_chars[i])
            {
                string_offset_cntrs[i] = 0;
            }
        }
    }
}
#endif

/*! \fn     miniDisplayCredentialAtPosition(uint8_t position, char* credential)
*   \brief  Display a given credential at a position for the wheel picking menu
*   \param  position    The position (0 to 2)
*   \param  credential  Text to display
*/
#ifdef MINI_VERSION
void miniDisplayCredentialAtPosition(uint8_t position, char* credential)
{
    uint8_t x_coordinates[] = {SCROLL_LINE_TEXT_FIRST_XPOS, SCROLL_LINE_TEXT_SECOND_XPOS, SCROLL_LINE_TEXT_THIRD_XPOS};
    uint8_t y_coordinates[] = {THREE_LINE_TEXT_FIRST_POS, THREE_LINE_TEXT_SECOND_POS, THREE_LINE_TEXT_THIRD_POS};

    string_extra_chars[position] = strlen(credential) - miniOledPutstrXY(x_coordinates[position], y_coordinates[position], OLED_RIGHT, (char*)credential + string_offset_cntrs[position]);
}
#endif

/*! \fn     guiAskForLoginSelect(pNode* p, cNode* c, uint16_t parentNodeAddress)
*   \brief  Ask for user login selection / approval
*   \param  p                   Pointer to a parent node
*   \param  c                   Pointer to a child node
*   \param  parentNodeAddress   Address of the parent node
*   \param  bypass_confirmation Bool to bypass authorisation request
*   \return Valid child node address or 0 otherwise
*/
uint16_t guiAskForLoginSelect(pNode* p, cNode* c, uint16_t parentNodeAddress, uint8_t bypass_confirmation)
{
    uint16_t first_child_address, temp_child_address;
    uint16_t picked_child = NODE_ADDR_NULL;
    
    // Check parent node address
    if (parentNodeAddress == NODE_ADDR_NULL)
    {
        return NODE_ADDR_NULL;
    }
    
    // Read the parent node
    readParentNode(p, parentNodeAddress);
    
    // Read its first child address
    first_child_address = p->nextChildAddress;
    
    // Check if there are stored credentials
    if (first_child_address == NODE_ADDR_NULL)
    {
        guiDisplayInformationOnScreenAndWait(ID_STRING_NO_CREDS);
        return NODE_ADDR_NULL;
    }
    
    // Read child node
    readChildNode(c, first_child_address);
    
    // Check if there's only one child, that's a confirmation screen
    if (c->nextChildAddress == NODE_ADDR_NULL)
    {
        confirmationText_t temp_conf_text;
        
        // Prepare asking confirmation screen
        #if defined(HARDWARE_OLIVIER_V1)
            temp_conf_text.lines[0] = readStoredStringToBuffer(ID_STRING_CONFACCESSTO);
            temp_conf_text.lines[1] = (char*)p->service;
            temp_conf_text.lines[2] = readStoredStringToBuffer(ID_STRING_WITHTHISLOGIN);
            temp_conf_text.lines[3] = (char*)c->login;
        #elif defined(MINI_VERSION)
            temp_conf_text.lines[0] = (char*)p->service;
            temp_conf_text.lines[1] = readStoredStringToBuffer(ID_STRING_CONFACCESSTO);
            temp_conf_text.lines[2] = (char*)c->login;
        #endif
        
        // Prompt user for confirmation, flash the screen
        #if defined(HARDWARE_OLIVIER_V1)
        if ((bypass_confirmation == TRUE) || (guiAskForConfirmation(0xF0 | 4, &temp_conf_text) == RETURN_OK))
        #elif defined(MINI_VERSION)
        if ((bypass_confirmation == TRUE) || (guiAskForConfirmation(0xF0 | 3, &temp_conf_text) == RETURN_OK))
        #endif
        {
            picked_child = first_child_address;
        }
    }
    else
    {
        #if defined(HARDWARE_OLIVIER_V1)
            temp_child_address = first_child_address;
            uint8_t action_chosen = FALSE;
            uint16_t addresses[4];
            uint8_t led_mask;
            int8_t i, j;
        
            while (action_chosen == FALSE)
            {
                // Draw asking bitmap
                oledBitmapDrawFlash(0, 0, BITMAP_LOGIN, 0);
            
                // Write domain name on screen
                char temp_string[INDEX_TRUNCATE_SERVICE_CENTER+1];
                memcpy(temp_string, (char*)p->service, sizeof(temp_string));
                temp_string[INDEX_TRUNCATE_SERVICE_CENTER] = 0;
                oledPutstrXY(0, 24, OLED_CENTRE, temp_string);
            
                // Clear led_mask
                led_mask = 0;
                i = 0;
            
                // List logins on screen
                while ((temp_child_address != NODE_ADDR_NULL) && (i != 4))
                {
                    // Read child node to get login
                    readChildNode(c, temp_child_address);
                
                    // Print Login at the correct slot
                    displayCredentialAtSlot(i, (char*)c->login, INDEX_TRUNCATE_LOGIN_FAV);            
                
                    // Store address in array, fetch next address
                    addresses[i] = temp_child_address;
                    temp_child_address = c->nextChildAddress;
                    i++;
                }
            
                // If nothing after, hide right arrow
                if ((i != 4) || (c->nextChildAddress == NODE_ADDR_NULL))
                {
                    oledFillXY(177, 25, 16, 14, 0x00);
                    led_mask |= LED_MASK_RIGHT;
                }
            
                // Light only the available choices
                for (j = i; j < 4; j++)
                {
                    led_mask |= (1 << j);
                }
            
                // Display picture
                oledDisplayOtherBuffer();
            
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
        #elif defined(MINI_VERSION)
            // Temp variables
            uint16_t last_child_address = first_child_address;
            temp_child_address = first_child_address;
            uint8_t string_refresh_needed = TRUE;
            picked_child = first_child_address;
            uint8_t action_chosen = FALSE;
            uint8_t cur_children_nb = 1;
            uint8_t nb_children = 0;
            RET_TYPE wheel_action;

            // Get number of children
            while(temp_child_address != NODE_ADDR_NULL)
            {
                nb_children++;
                readChildNode(c, temp_child_address);
                last_child_address = temp_child_address;
                temp_child_address = c->nextChildAddress;
            }

            // Clear pending detections & light up screen
            miniWheelClearDetections();
            activityDetectedRoutine();

            // Arm timer for scrolling (caps timer that isn't relevant here)
            activateTimer(TIMER_CAPS, SCROLLING_DEL);
        
            while (action_chosen == FALSE)
            {                        
                // If needed, re-compute the string offsets & extra chars
                if ((string_refresh_needed != FALSE) || (hasTimerExpired(TIMER_CAPS, TRUE) == TIMER_EXPIRED))
                {
                    if(string_refresh_needed != FALSE)
                    {
                        // Reset counters
                        memset((void*)string_offset_cntrs, 0x00, sizeof(string_offset_cntrs));
                    }
                    else
                    {
                        // Implement scrolling
                        miniIncrementScrolledTexts();
                    }

                    // Scrolling timer expired
                    activateTimer(TIMER_CAPS, SCROLLING_DEL);      

                    // Clear LCD, init temporary vars
                    oledClear();
                    char temp_string[10];
                    memset(temp_string, 0x00, sizeof(temp_string));
                    char* select_cred_line = readStoredStringToBuffer(ID_STRING_SELECT_CREDENTIAL);

                    // First line: service name
                    string_extra_chars[0] = strlen((char*)p->service) - miniOledPutCenteredString(THREE_LINE_TEXT_FIRST_POS, (char*)p->service + string_offset_cntrs[0]);

                    // Second line: "select credential" + x/total
                    itoa(cur_children_nb, temp_string, 10);
                    temp_string[strlen(temp_string)] = '/';
                    itoa(nb_children, temp_string+strlen(temp_string), 10);
                    strncat(select_cred_line, temp_string, TEXTBUFFERSIZE - strlen(select_cred_line) - 1);
                    miniOledPutCenteredString(THREE_LINE_TEXT_SECOND_POS, select_cred_line);     
                    
                    // Third line: chosen credential   
                    readChildNode(c, picked_child);
                    string_extra_chars[1] = strlen((char*)c->login) - miniOledPutCenteredString(THREE_LINE_TEXT_THIRD_POS, (char*)c->login + string_offset_cntrs[1]);
                    
                    // Flush to display
                    miniOledFlushEntireBufferToDisplay();
                    string_refresh_needed = FALSE;
                }
            
                // Get wheel action
                wheel_action = miniGetWheelAction(FALSE, FALSE);
            
                // Check its validity, knowing that by default we will return NODE_ADDR_NULL
                if (wheel_action == WHEEL_ACTION_SHORT_CLICK)
                {
                    action_chosen = TRUE;
                }
                else if (wheel_action == WHEEL_ACTION_DOWN)
                {
                    // Move to the next credential
                    string_refresh_needed = TRUE;

                    // Loop or go to previous
                    picked_child = c->prevChildAddress;
                    if (picked_child == NODE_ADDR_NULL)
                    {
                        picked_child = last_child_address;
                    }

                    // Update status display
                    if (cur_children_nb == 1)
                    {
                        cur_children_nb = nb_children;
                    } 
                    else
                    {
                        cur_children_nb--;
                    }
                }
                else if (wheel_action == WHEEL_ACTION_UP)
                {                 
                    // Move to the previous credential    
                    string_refresh_needed = TRUE;                    

                    // Loop or go to previous
                    picked_child = c->nextChildAddress;
                    if (picked_child == NODE_ADDR_NULL)
                    {
                        picked_child = first_child_address;
                    }  

                    // Update status display
                    if (cur_children_nb == nb_children)
                    {
                        cur_children_nb = 1;
                    } 
                    else
                    {
                        cur_children_nb++;
                    }                  
                }
                else if (wheel_action == WHEEL_ACTION_LONG_CLICK)
                {
                    // Long click to go back (checked by function caller)
                    return NODE_ADDR_NULL;
                }

                // Timeout or card removed
                if ((hasTimerExpired(TIMER_USERINT, TRUE) == TIMER_EXPIRED) || (isSmartCardAbsent() == RETURN_OK))
                {
                    return NODE_ADDR_NULL;
                }

                // Request cancelled by plugin
                if (usbCancelRequestReceived() == RETURN_OK)
                {
                    return NODE_ADDR_NULL;
                }
            }
        #endif
    }
    
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
#if defined(MINI_VERSION)
    uint8_t nbFavorites = 0, startIndex = 0, selectedIndex = 0;
    uint16_t cur_address_selected = NODE_ADDR_NULL;
    uint8_t string_refresh_needed = TRUE;
    uint16_t parentAddresses[USER_MAX_FAV];
    uint16_t childAddresses[USER_MAX_FAV];
    RET_TYPE wheel_action;
    uint8_t i, j;
    (void)c;
    
    // Browse through the favorites
    for (i = 0; i < USER_MAX_FAV; i++)
    {
        // Read favorite
        readFav(i, &parentAddresses[i], &childAddresses[i]);
        
        // If so, store it in our know addresses
        if (parentAddresses[i] != NODE_ADDR_NULL)
        {
            startIndex = i;
            nbFavorites++;
        }
    }
    
    // If no favorite, return
    if (nbFavorites == 0)
    {
        guiDisplayInformationOnScreenAndWait(ID_STRING_NOSTOREDFAV);
        return NODE_ADDR_NULL;
    }

    // Arm timer for scrolling (caps timer that isn't relevant here)
    activateTimer(TIMER_CAPS, SCROLLING_DEL);

    while(1)
    {
        // If needed, re-compute the string offsets & extra chars
        if ((string_refresh_needed != FALSE) || (hasTimerExpired(TIMER_CAPS, TRUE) == TIMER_EXPIRED))
        {
            if(string_refresh_needed != FALSE)
            {
                // Reset counters
                memset((void*)string_offset_cntrs, 0x00, sizeof(string_offset_cntrs));
            }
            else
            {
                // Implement scrolling
                miniIncrementScrolledTexts();
            }

            // Scrolling timer expired
            activateTimer(TIMER_CAPS, SCROLLING_DEL);

            // Start looping, starting from the first displayed favorite
            j = startIndex;

            // When less than 3 favorites, skip one slot
            if (nbFavorites < 3)
            {
                i = 1;
            }
            else
            {
                i = 0;
            }
            
            oledClear();
            oledBitmapDrawFlash(121, 0, BITMAP_SCROLL_WHEEL, OLED_SCROLL_NONE);
            // Display the favorites
            while(i != 3)
            {
                // Check that the favorite is valid
                if (parentAddresses[j] != NODE_ADDR_NULL)
                {
                    // Read parent & child node to get service & username
                    readParentNode(p, parentAddresses[j]);
                    readChildNode(c, childAddresses[j]);
                    
                    // Construct the string "service / username"
                    if (c->login[0] != 0)
                    {
                        // trick because the start of the login field isn't the start of the service node
                        #pragma GCC diagnostic push
                        #pragma GCC diagnostic ignored "-Warray-bounds"
                        c->login[-1] = '/';
                        c->login[sizeof(c->login)-1] = 0;
                        p->service[sizeof(p->service)-1] = 0;
                        strncat((char*)p->service, (char*)&(c->login[-1]), sizeof(p->service) - 1 - strnlen((char*)&(p->service[-1]), sizeof(p->service)));
                        #pragma GCC diagnostic pop
                    }

                    // Print service / username at the correct slot
                    miniDisplayCredentialAtPosition(i, (char*)p->service);                    

                    // Second favorite displayed is the chosen one
                    if (i == 1)
                    {
                        cur_address_selected = childAddresses[j];
                        selectedIndex = j;
                    }

                    // Stop if we only have one credential
                    if (nbFavorites == 1)
                    {
                        break;
                    }

                    // Visit next slot
                    j++;
                    i++;
                }
                else
                {
                    j++;
                }

                // Check if we need to loop
                if (j == USER_MAX_FAV)
                {
                    j = 0;
                }
            }

            miniOledFlushEntireBufferToDisplay();
            string_refresh_needed = FALSE;
        }

        // Get wheel action
        wheel_action = miniGetWheelAction(FALSE, FALSE);
        
        // User validated the selected credential
        if (wheel_action == WHEEL_ACTION_SHORT_CLICK)
        {
            readParentNode(p, parentAddresses[selectedIndex]);
            return cur_address_selected;
        }
        else if (wheel_action == WHEEL_ACTION_DOWN)
        {
            // Move to the next credential
            string_refresh_needed = TRUE;
            startIndex = selectedIndex;

            // Special case when there are only 2 credentials
            if (nbFavorites == 2)
            {
                startIndex = (startIndex+1)%USER_MAX_FAV;
            }
        }
        else if (wheel_action == WHEEL_ACTION_UP)
        {
            // Move to the previous credential
            string_refresh_needed = TRUE;

            do 
            {
                if (--startIndex == UINT8_MAX)
                {
                    startIndex = USER_MAX_FAV - 1;
                }
            } 
            while (parentAddresses[startIndex] == NODE_ADDR_NULL);
        }
        else if (wheel_action == WHEEL_ACTION_LONG_CLICK)
        {
            return NODE_ADDR_NULL;
        }

        if ((hasTimerExpired(TIMER_USERINT, TRUE) == TIMER_EXPIRED) || (isSmartCardAbsent() == RETURN_OK))
        {
            return NODE_ADDR_NULL;
        }
    }
#elif defined(HARDWARE_OLIVIER_V1)
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
        guiDisplayInformationOnScreenAndWait(ID_STRING_NOSTOREDFAV);
        return NODE_ADDR_NULL;
    }
    
    // Loop until the user chooses smth
    while (action_chosen != TRUE)
    {
        // Draw asking bitmap
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
            displayCredentialAtSlot(i+((i&0x02)<<2), (char*)c->login, INDEX_TRUNCATE_LOGIN_FAV);
            displayCredentialAtSlot(i+((~i&0x02)<<2), (char*)p->service, INDEX_TRUNCATE_LOGIN_FAV);
            
            // Increment i
            i++;
        }
        
        // If nothing after, hide right arrow
        if ((i != 4) || ((offset+i) == nbFavorites))
        {
            oledFillXY(177, 25, 16, 14, 0x00);
            led_mask |= LED_MASK_RIGHT;
        }
        
        // Light only the available choices
        for (j = i; j < 4; j++)
        {
            led_mask |= (1 << j);
        }
        
        // Display picture
        oledDisplayOtherBuffer();
        
        // Get touched quarter
        j = getTouchedPositionAnswer(led_mask);
        
        // Check its validity, knowing that by default we will return NODE_ADDR_NULL
        if (j == -1)
        {
            action_chosen = TRUE;
            // Time out
        }
        else if (j < i)
        {
            // Valid choice, load parent node as it will be used later
            readParentNode(p, parentAddresses[offset+j]);
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
#endif
}

#ifdef HARDWARE_OLIVIER_V1
/*! \fn     displayCurrentSearchLoginTexts(char* text)
*   \brief  Display current search login text
*   \param  text            Text to be displayed
*   \param  resultsarray    Pointer to the array in which to store the addresses
*   \param  search_index    Current search index (aka strlen(text))
*   \return Number of matching parents we displayed
*/
static inline uint8_t displayCurrentSearchLoginTexts(char* text, uint16_t* resultsarray, uint8_t search_index)
{
    uint16_t tempNodeAddr;
    pNode temp_pnode;
    uint8_t i, j;
    
    // Set font for search text
    oledSetFont(FONT_PROFONT_18);
    
    // Clear current text
    oledFillXY(100, 18, 50, 23, 0x00);
    
    // Display new search text
    oledPutstrXY(148, 17, OLED_RIGHT, text);
    
    // Set default font
    oledSetFont(FONT_DEFAULT);
    
    // Find the address of the first match
    tempNodeAddr = searchForServiceName((uint8_t*)text, COMPARE_MODE_COMPARE, SERVICE_CRED_TYPE);
    
    // Only change display if the first displayed service changed
    if (tempNodeAddr != last_matching_parent_addr)
    {
        last_matching_parent_addr = tempNodeAddr;
        
        for (i = 0; i < 4; i++)
        {
            oledFillXY((i&1)*170, 2+(i&2)*23, 84, 14, 0x00);
        }
        
        // Print the next 4 services (loop is until 5 for additional checks)
        i = 0;
        uint8_t temp_bool = TRUE;
        while ((temp_bool != FALSE) && (i != 5))
        {
            resultsarray[i] = tempNodeAddr;
            readParentNode(&temp_pnode, tempNodeAddr);
            
            // Display only first 4 services
            if (i < 4)
            {
                displayCredentialAtSlot(i, (char*)temp_pnode.service, INDEX_TRUNCATE_SERVICE_SEARCH);
            }
            // Loop around
            if (temp_pnode.nextParentAddress == NODE_ADDR_NULL)
            {
                tempNodeAddr = getStartingParentAddress();
            } 
            else
            {
                tempNodeAddr = temp_pnode.nextParentAddress;
            }
            i++;
            // Check that we haven't already displayed the next node
            for (j = 0; j < i; j++)
            {
                if (resultsarray[j] == tempNodeAddr)
                {
                    temp_bool = FALSE;
                }
            }
        }
        
        // Check if we could read 5 services after the given search text to know if we need to show the right arrow
        if (i == 5)
        {       
            // Compare our text with the last service text and see if they match
            if (strncmp(text, (char*)temp_pnode.service, search_index + 1) == 0)
            {
                // show arrow
                oledBitmapDrawFlash(176, 24, BITMAP_LOGIN_RARROW, 0);
            } 
            else
            {
                // hide arrow
                oledFillXY(176, 24, 16, 16, 0);
                i = 4;
            }
        }
        
        // Store and return number of children
        last_matching_parent_number = i;
    }
    
         
    // If the text is 4 chars long no need to display right arrow
    if ((search_index == SEARCHTEXT_MAX_LENGTH - 1) && (last_matching_parent_number > 4))
    {
        // hide arrow
        last_matching_parent_number = 4;
        oledFillXY(176, 24, 16, 16, 0);
    }
         
    return last_matching_parent_number;
}
#endif

/*! \fn     loginSelectionScreen(void)
*   \brief  Screen displayed to let the user choose/find a login
*   \return Valid parent node address or 0 otherwise
*/
uint16_t loginSelectionScreen(void)
{
#if defined(MINI_VERSION)
    uint16_t first_address = getLastParentAddress();
    uint16_t cur_address_selected = NODE_ADDR_NULL;
    uint8_t string_refresh_needed = TRUE;
    uint16_t temp_parent_address;
    uint8_t nb_parent_nodes;
    char current_fchar = 0;
    RET_TYPE wheel_action;
    char fchar_array[3];
    pNode temp_pnode;
    uint8_t i;

    // Read first parent node, see if there's more than 2 credentials
    readParentNode(&temp_pnode, getStartingParentAddress());
    if (getLastParentAddress() == getStartingParentAddress())
    {
        nb_parent_nodes = 1;
    }
    else if (temp_pnode.nextParentAddress == getLastParentAddress())
    {
        first_address = getStartingParentAddress();
        nb_parent_nodes = 2;
    }
    else
    {
        nb_parent_nodes = 3;
    }

    // Arm timer for scrolling (caps timer that isn't relevant here)
    activateTimer(TIMER_CAPS, SCROLLING_DEL);

    // Clear possible detections
    miniWheelClearDetections();

    while(1)
    {
        // If needed, re-compute the string offsets & extra chars
        if ((string_refresh_needed != FALSE) || (hasTimerExpired(TIMER_CAPS, TRUE) == TIMER_EXPIRED))
        {
            if(string_refresh_needed != FALSE)
            {
                // Reset counters
                memset((void*)string_offset_cntrs, 0x00, sizeof(string_offset_cntrs));
            }
            else
            {
                // Implement scrolling
                miniIncrementScrolledTexts();
            }

            // Scrolling timer expired
            activateTimer(TIMER_CAPS, SCROLLING_DEL);

            // Start looping, starting from the first displayed child
            temp_parent_address = first_address;

            // Skip one display slot if the real first parent is selected
            if (nb_parent_nodes < 3)
            {
                i = 1;
            } 
            else
            {
                i = 0;
            }
            
            oledClear();
            miniOledSetMinTextY(16);
            oledBitmapDrawFlash(0, 0, BITMAP_LOGIN_LPANE, OLED_SCROLL_NONE);
            oledBitmapDrawFlash(121, 0, BITMAP_SCROLL_WHEEL, OLED_SCROLL_NONE);
            // Display the parent nodes
            for (; (i < 3); i++)
            {
                // Read child node to get login
                readParentNode(&temp_pnode, temp_parent_address);
                
                // Print Login at the correct slot
                miniDisplayCredentialAtPosition(i, (char*)temp_pnode.service);                

                // Second child displayed is the chosen one
                if (i == 1)
                {
                    cur_address_selected = temp_parent_address;
                    current_fchar = temp_pnode.service[0];
                }
                
                // Fetch next address
                temp_parent_address = temp_pnode.nextParentAddress;
                if (temp_parent_address == NODE_ADDR_NULL)
                {
                    temp_parent_address = getStartingParentAddress();
                }

                // Stop if we only have one credential
                if (nb_parent_nodes == 1)
                {
                    break;
                }
            }

            // Display first letters
            uint8_t glyph_width;
            getPreviousNextFirstLetterForGivenLetter(current_fchar, fchar_array);
            miniOledSetFont(FONT_8BIT16);
            glyph_width = miniOledGlyphWidth(fchar_array[0]);
            miniOledSetXY(5-(glyph_width>>1), 1);
            miniOledPutch(fchar_array[0]);
            glyph_width = miniOledGlyphWidth(fchar_array[2]);
            miniOledSetXY(5-(glyph_width>>1), 26);
            miniOledPutch(fchar_array[2]);
            miniOledSetFont(FONT_PROFONT_14);
            glyph_width = miniOledGlyphWidth(fchar_array[1]);
            miniOledSetXY(5-(glyph_width>>1), 6);
            miniOledPutch(fchar_array[1]);
            miniOledFlushEntireBufferToDisplay();
            miniOledSetFont(FONT_DEFAULT);
            string_refresh_needed = FALSE;
            miniOledSetMinTextY(0);
        }

        // Get wheel action
        wheel_action = miniGetWheelAction(FALSE, FALSE);
        
        // User validated the selected credential
        if (wheel_action == WHEEL_ACTION_SHORT_CLICK)
        {
            return cur_address_selected;
        }
        else if (wheel_action == WHEEL_ACTION_DOWN)
        {
            // Move to the next credential
            string_refresh_needed = TRUE;
            first_address = cur_address_selected;

            // Special case when there are only 2 credentials
            if (nb_parent_nodes == 2)
            {
                if (first_address == getStartingParentAddress())
                {
                    first_address = getLastParentAddress();
                }
                else
                {
                    first_address = getStartingParentAddress();
                }
            }
        }
        else if (wheel_action == WHEEL_ACTION_UP)
        {
            // Move to the previous credential
            string_refresh_needed = TRUE;

            // Read child node to get previous node
            if (first_address == getStartingParentAddress())
            {
                first_address = getLastParentAddress();
            }
            else
            {
                readParentNode(&temp_pnode, first_address);
                first_address = temp_pnode.prevParentAddress;
            }
        }
        else if (wheel_action == WHEEL_ACTION_LONG_CLICK)
        {
            return NODE_ADDR_NULL;
        }

        if ((hasTimerExpired(TIMER_USERINT, TRUE) == TIMER_EXPIRED) || (isSmartCardAbsent() == RETURN_OK))
        {
            return NODE_ADDR_NULL;
        }
    }
#elif defined(HARDWARE_OLIVIER_V1)
    char currentText[SEARCHTEXT_MAX_LENGTH+1];
    uint8_t displayRefreshNeeded = TRUE;
    uint16_t ret_val = NODE_ADDR_NULL;
    uint8_t wasWheelReleased = TRUE;
    uint16_t tempParentAddresses[5];
    uint8_t currentStringIndex = 0;
    uint8_t nbMatchedParents= 0;
    uint8_t finished = FALSE;
    RET_TYPE temp_rettype;
    uint8_t led_mask = 0;
    int8_t temp_int8;
    
    // Set current text to a
    last_matching_parent_addr = NODE_ADDR_NULL;
    last_matching_parent_number = 0;
    memcpy(currentText, "a\x00\x00\x00\x00", sizeof(currentText));
    
    // Draw bitmap, display it and write active buffer
    oledBitmapDrawFlash(0, 0, BITMAP_LOGIN_FIND, 0);
    oledDisplayOtherBuffer();
    oledWriteActiveBuffer();
    
    // While the user hasn't chosen a credential
    while(!finished)
    {
        if (displayRefreshNeeded == TRUE)
        {
            nbMatchedParents = displayCurrentSearchLoginTexts(currentText, tempParentAddresses, currentStringIndex);
            displayRefreshNeeded = FALSE;
            
            // Light only the available choices and right arrow
            led_mask = 0;
            for (temp_int8 = nbMatchedParents; temp_int8 < 5; temp_int8++)
            {
                led_mask |= (1 << temp_int8);
            }
        }
        
        // Detect key touches
        temp_rettype = touchDetectionRoutine(led_mask);
        
        // Algo to differentiate a tap from a scroll
        if ((temp_rettype & RETURN_WHEEL_PRESSED) && (wasWheelReleased == TRUE))
        {
            // We use the timer dedicated to touch inhibit for min
            activateTimer(TIMER_TOUCH_INHIBIT, TAP_MIN_DEL);
            // We use the timer dedicated to caps detect for max
            activateTimer(TIMER_CAPS, TAP_MAX_DEL);
            // This works because we use the same clearing flags 
            wasWheelReleased = FALSE;
        }
        else if (temp_rettype & RETURN_WHEEL_RELEASED)
        {
            // Check if it's a tap and that the selected domain is valid
            if ((hasTimerExpired(TIMER_CAPS, FALSE) == TIMER_RUNNING) && (hasTimerExpired(TIMER_TOUCH_INHIBIT, FALSE) == TIMER_EXPIRED) && (getWheelTouchDetectionQuarter() < nbMatchedParents))
            {
                ret_val = tempParentAddresses[getWheelTouchDetectionQuarter()];
                finished = TRUE;
            }
            wasWheelReleased = TRUE;
        }
        
        // Send it to the touch wheel interface logic
        temp_int8 = touchWheelIntefaceLogic(temp_rettype);

        // Position increment / decrement
        if (temp_int8 != 0)
        {
            if ((currentText[currentStringIndex] == 0x7A) && (temp_int8 == 1))
            {
                // z->0 wrap
                currentText[currentStringIndex] = 0x2F;
            }
            if ((currentText[currentStringIndex] == 0x39) && (temp_int8 == 1))
            {
                // 9->a wrap
                currentText[currentStringIndex] = 0x60;
            }
            else if ((currentText[currentStringIndex] == 0x30) && (temp_int8 == -1))
            {
                // 0->z wrap
                currentText[currentStringIndex] = 0x7B;
            }
            else if ((currentText[currentStringIndex] == 0x61) && (temp_int8 == -1))
            {
                // a->9 wrap
                currentText[currentStringIndex] = 0x3A;
            }
            currentText[currentStringIndex] += temp_int8;
            displayRefreshNeeded = TRUE;
        }
        
        if ((isSmartCardAbsent() == RETURN_OK) || (hasTimerExpired(TIMER_USERINT, TRUE) == TIMER_EXPIRED))
        {
            // Smartcard removed or no interaction for a while
            finished = TRUE;
        }
        else if (temp_rettype & RETURN_LEFT_PRESSED)
        {
            // Change search index
            if (currentStringIndex > 0)
            {
                currentText[currentStringIndex--] = 0;
                displayRefreshNeeded = TRUE;
            } 
            else
            {
                finished = TRUE;
            }
        }
        else if ((temp_rettype & RETURN_RIGHT_PRESSED) && (nbMatchedParents > 4))
        {
            // Change search index only if we need to...
            currentText[++currentStringIndex] = 'a';
            displayRefreshNeeded = TRUE;
        }
    }
    
    // Set inactive buffer write by default
    oledWriteInactiveBuffer();
    
    return ret_val;
#endif
}    
#endif