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
/*!  \file     mini_gui_credentials_functions.c
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
#include "mini_inputs.h"
#include "node_mgmt.h"
#include "node_mgmt.h"
#include "oledmini.h"
#include "defines.h"
#include "delays.h"
#include "anim.h"
#include "gui.h"
#include "usb.h"
#ifdef MINI_VERSION
// String offset counters for scrolling
uint8_t string_offset_cntrs[3];
// Number of extra chars for current displayed string
uint8_t string_extra_chars[3];

/*! \fn     miniIncrementScrolledTexts(void)
*   \brief  Change offset char for the currently displayed strings
*/
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

/*! \fn     miniResetStringOffsetCounters(void)
*   \brief  Reset offset counter of the currently displayed strings
*/
void miniResetStringOffsetCounters(void)
{
     memset((void*)string_offset_cntrs, 0x00, sizeof(string_offset_cntrs));
}

/*! \fn     miniDisplayCredentialAtPosition(uint8_t position, char* credential)
*   \brief  Display a given credential at a position for the wheel picking menu
*   \param  position    The position (0 to 2)
*   \param  credential  Text to display
*/
void miniDisplayCredentialAtPosition(uint8_t position, char* credential)
{
    uint8_t x_coordinates[] = {SCROLL_LINE_TEXT_FIRST_XPOS, SCROLL_LINE_TEXT_SECOND_XPOS, SCROLL_LINE_TEXT_THIRD_XPOS};
    uint8_t y_coordinates[] = {THREE_LINE_TEXT_FIRST_POS, THREE_LINE_TEXT_SECOND_POS, THREE_LINE_TEXT_THIRD_POS};

    #ifndef LOGIN_FAV_MENU_LEFT_ALIGN
        string_extra_chars[position] = strlen(credential) - miniOledPutstrXY(x_coordinates[position], y_coordinates[position], OLED_RIGHT, (char*)credential + string_offset_cntrs[position]);
    #else
        // In the future we may want to add an extra parameter to specify if we shift or not to 17 to gain a few pixels
        miniOledSetMaxTextY(x_coordinates[position]);
        string_extra_chars[position] = strlen(credential) - miniOledPutstrXY(17, y_coordinates[position], OLED_LEFT, (char*)credential + string_offset_cntrs[position]);  
        miniOledResetMaxTextY();      
    #endif
}

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
        temp_conf_text.lines[0] = (char*)p->service;
        temp_conf_text.lines[1] = readStoredStringToBuffer(ID_STRING_CONFACCESSTO);
        temp_conf_text.lines[2] = (char*)c->login;

        // Prompt user for confirmation, flash the screen
        if ((bypass_confirmation == TRUE) || (guiAskForConfirmation(0xF0 | 3, &temp_conf_text) == RETURN_OK))
        {
            picked_child = first_child_address;
        }
    }
    else
    {
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
            if ((string_refresh_needed != FALSE) || (hasTimerExpired(TIMER_CAPS, FALSE) == TIMER_EXPIRED))
            {
                if(string_refresh_needed != FALSE)
                {
                    // Reset counters
                    miniResetStringOffsetCounters();
                }
                else
                {
                    // Implement scrolling
                    miniIncrementScrolledTexts();
                }

                // Scrolling timer expired
                activateTimer(TIMER_CAPS, SCROLLING_DEL);

                // Clear LCD, init temporary vars
                miniOledClearFrameBuffer();
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
        if ((string_refresh_needed != FALSE) || (hasTimerExpired(TIMER_CAPS, FALSE) == TIMER_EXPIRED))
        {
            if(string_refresh_needed != FALSE)
            {
                // Reset counters
                miniResetStringOffsetCounters();
            }
            else
            {
                // Implement scrolling
                miniIncrementScrolledTexts();
                // 10/11/2016: only scroll selected line
                string_offset_cntrs[0] = 0;
                string_offset_cntrs[2] = 0;
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

            miniOledClearFrameBuffer();
            miniOledBitmapDrawFlash(121, 0, BITMAP_SCROLL_WHEEL, OLED_SCROLL_NONE);
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
                        // Login and service are guaranteed to be 0 terminated because of the readchild / readparent functions
                        c->login[-1] = '/';
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
}

/*! \fn     loginSelectionScreen(void)
*   \brief  Screen displayed to let the user choose/find a login
*   \return Valid parent node address or 0 otherwise
*/
uint16_t loginSelectionScreen(void)
{
    uint16_t first_address = getLastParentAddress();
    uint16_t cur_address_selected = NODE_ADDR_NULL;
    uint16_t prev_next_fletter_parents_addr[3];
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
        if ((string_refresh_needed != FALSE) || (hasTimerExpired(TIMER_CAPS, FALSE) == TIMER_EXPIRED))
        {
            if(string_refresh_needed != FALSE)
            {
                // Reset counters
                miniResetStringOffsetCounters();
            }
            else
            {
                // Implement scrolling
                miniIncrementScrolledTexts();                
                // 10/11/2016: only scroll selected line
                string_offset_cntrs[0] = 0;
                string_offset_cntrs[2] = 0;
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

            miniOledClearFrameBuffer();
            miniOledSetMinTextY(16);
            miniOledBitmapDrawFlash(0, 0, BITMAP_LOGIN_LPANE, OLED_SCROLL_NONE);
            miniOledBitmapDrawFlash(121, 0, BITMAP_SCROLL_WHEEL, OLED_SCROLL_NONE);
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
            getPreviousNextFirstLetterForGivenLetter(current_fchar, fchar_array, prev_next_fletter_parents_addr);
            displayCenteredCharAtPosition(fchar_array[0], 5, 1, FONT_8BIT16);
            displayCenteredCharAtPosition(fchar_array[1], 5, 6, FONT_PROFONT_14);
            displayCenteredCharAtPosition(fchar_array[2], 5, 26, FONT_8BIT16);
            miniOledFlushEntireBufferToDisplay();
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
        else if (wheel_action == WHEEL_ACTION_CLICK_UP)
        {
            if ((nb_parent_nodes >= 3) && (prev_next_fletter_parents_addr[0] != NODE_ADDR_NULL))
            {
                // Move to the previous first letter credential
                string_refresh_needed = TRUE;

                // Read previous letter first node, first displayed parent is the previous node
                readParentNode(&temp_pnode, prev_next_fletter_parents_addr[0]);
                if (temp_pnode.prevParentAddress != NODE_ADDR_NULL)
                {
                    first_address = temp_pnode.prevParentAddress;
                } 
                else
                {
                    first_address = getLastParentAddress();
                }
            }
        }
        else if (wheel_action == WHEEL_ACTION_CLICK_DOWN)
        {
            if ((nb_parent_nodes >= 3) && (prev_next_fletter_parents_addr[2] != NODE_ADDR_NULL))
            {
                // Move to the next first letter credential
                string_refresh_needed = TRUE;

                // Read next letter first node, first displayed parent is the previous node
                readParentNode(&temp_pnode, prev_next_fletter_parents_addr[2]);
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
}

#ifdef ENABLE_CREDENTIAL_MANAGEMENT
/*! \fn     managementActionSelectionScreen(void)
*   \brief  Screen displayed to let the user choose a management action
*   \return Management action ID
*/
uint8_t managementActionSelectionScreen(void)
{
    uint8_t string_refresh_needed = TRUE;
    RET_TYPE wheel_action;
    uint8_t i, j;

    uint8_t action_selected = 0;

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

            // Start looping, starting from the first displayed management action
            j = action_selected - 1;

            if(action_selected == 0)
            {
                j = ONDEVICE_CRED_MGMT_ACTION_NB - 1;
            }

            i = 0;

            miniOledClearFrameBuffer();
            miniOledBitmapDrawFlash(121, 0, BITMAP_SCROLL_WHEEL, OLED_SCROLL_NONE);
            // Display the possible actions
            while(i != 3)
            {

                // Print management action at the correct slot
                miniDisplayCredentialAtPosition(i, readStoredStringToBuffer(ID_STRING_MGMT_CREATE + j)/* mgmt_actions[j] */);

                // Second action displayed is the chosen one
                if (i == 1)
                {
                    action_selected = j;
                }

                // Visit next slot
                j = (j+1) % ONDEVICE_CRED_MGMT_ACTION_NB;
                i++;
            }

            miniOledFlushEntireBufferToDisplay();
            string_refresh_needed = FALSE;
        }

        // Get wheel action
        wheel_action = miniGetWheelAction(FALSE, FALSE);

        // User validated the selected action
        if (wheel_action == WHEEL_ACTION_SHORT_CLICK)
        {
            return action_selected;
        }
        else if (wheel_action == WHEEL_ACTION_DOWN)
        {
            // Move to the next menu entry
            string_refresh_needed = TRUE;
            action_selected = (action_selected + 1) % ONDEVICE_CRED_MGMT_ACTION_NB;
        }
        else if (wheel_action == WHEEL_ACTION_UP)
        {
            // Move to the previous menu entry
            string_refresh_needed = TRUE;

            if (--action_selected == UINT8_MAX)
            {
                action_selected = ONDEVICE_CRED_MGMT_ACTION_NB - 1;
            }
        }
        else if (wheel_action == WHEEL_ACTION_LONG_CLICK)
        {
            return ONDEVICE_CRED_MGMT_ACTION_NONE;
        }

        if ((hasTimerExpired(TIMER_USERINT, TRUE) == TIMER_EXPIRED) || (isSmartCardAbsent() == RETURN_OK))
        {
            return ONDEVICE_CRED_MGMT_ACTION_NONE;
        }
    }
}
#endif

#endif
