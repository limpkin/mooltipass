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
/*!  \file     gui.h
*    \brief    General user interface
*    Created:  22/6/2014
*    Author:   Mathieu Stephan
*/


#ifndef GUI_H_
#define GUI_H_

#include "node_mgmt.h"

/* Defines */
// Wheel interface
#define WHEEL_TICK_INCREMENT            32
// Timers
#define SCREEN_TIMER_DEL                60000
#define LIGHT_TIMER_DEL                 16000
#define USER_INTER_DEL                  6000
// Screen defines
#define SCREEN_DEFAULT_NINSERTED        0
#define SCREEN_DEFAULT_INSERTED_LCK     1
#define SCREEN_DEFAULT_INSERTED_NLCK    2
#define SCREEN_DEFAULT_INSERTED_INVALID 3
#define SCREEN_SETTINGS                 4

/* Structs */
typedef struct
{
    const char* line1;
    char* line2;
    const char* line3;
    char* line4;
} confirmationText_t;

/* Prototypes */
uint16_t guiAskForLoginSelect(mgmtHandle* h, pNode* p, cNode* c, uint16_t parentNodeAddress);
RET_TYPE guiAskForConfirmation(uint8_t nb_args, confirmationText_t* text_object);
RET_TYPE guiGetPinFromUser(uint16_t* pin_code, const char* string);
void guiDisplayInformationOnScreen(const char* string);
RET_TYPE guiDisplayInsertSmartCardScreenAndWait(void);
void informGuiOfCurrentContext(char* context);
RET_TYPE guiHandleSmartcardInserted(void);
void guiSetCurrentScreen(uint8_t screen);
RET_TYPE guiCardUnlockingProcess(void);
void guiDisplayProcessingScreen(void);
void guiHandleSmartcardRemoved(void);
void guiGetBackToCurrentScreen(void);
void activityDetectedRoutine(void);
void guiMainLoop(void);

#endif /* GUI_H_ */