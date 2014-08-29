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
/*!  \file     gui_screen_functions.h
*    \brief    General user interface - screen functions
*    Created:  22/6/2014
*    Author:   Mathieu Stephan
*/ 


#ifndef GUI_SCREEN_FUNCTIONS_H_
#define GUI_SCREEN_FUNCTIONS_H_

#include "defines.h"
#include "gui.h"

RET_TYPE guiAskForConfirmation(uint8_t nb_args, confirmationText_t* text_object);
void guiDisplayInformationOnScreen(uint8_t stringID);
void guiScreenLoop(uint8_t touch_detect_result);
RET_TYPE guiAskForNewPin(uint16_t* new_pin);
void guiSetCurrentScreen(uint8_t screen);
void guiDisplayProcessingScreen(void);
void guiGetBackToCurrentScreen(void);
uint8_t getCurrentScreen(void);


#endif /* GUI_SCREEN_FUNCTIONS_H_ */