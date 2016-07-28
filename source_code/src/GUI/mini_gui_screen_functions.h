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
/*!  \file     mini_gui_screen_functions.h
*    \brief    General user interface - screen functions
*    Created:  22/6/2014
*    Author:   Mathieu Stephan
*/


#ifndef MINI_GUI_SCREEN_FUNCTIONS_H_
#define MINI_GUI_SCREEN_FUNCTIONS_H_

#include "defines.h"
#include "gui.h"

// Global vars
#ifdef MINI_VERSION
    #define THREE_LINE_TEXT_FIRST_POS       0
    #define THREE_LINE_TEXT_SECOND_POS      10
    #define THREE_LINE_TEXT_THIRD_POS       21
    #define TWO_LINE_TEXT_FIRST_POS         5
    #define TWO_LINE_TEXT_SECOND_POS        16
    #define SCROLL_LINE_TEXT_FIRST_XPOS     121
    #define SCROLL_LINE_TEXT_SECOND_XPOS    116
    #define SCROLL_LINE_TEXT_THIRD_XPOS     121
#endif

// Prototypes
RET_TYPE guiAskForConfirmation(uint8_t nb_args, confirmationText_t* text_object);
RET_TYPE guiAskForNewPin(volatile uint16_t* new_pin, uint8_t message_id);
void guiDisplayInformationOnScreenAndWait(uint8_t stringID);
void guiDisplaySmartcardUnlockedScreen(uint8_t* username);
void guiDisplayInformationOnScreen(uint8_t stringID);
void guiDisplayLoginOrPasswordOnScreen(char* text);
void guiScreenLoop(uint8_t touch_detect_result);
void guiDisplayRawString(uint8_t stringID);
void guiSetCurrentScreen(uint8_t screen);
void guiDisplayProcessingScreen(void);
void guiGetBackToCurrentScreen(void);
void guiDisplayGoingToSleep(void);
uint8_t getCurrentScreen(void);


#endif /* MINI_GUI_SCREEN_FUNCTIONS_H_ */