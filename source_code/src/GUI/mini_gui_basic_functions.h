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
/*!  \file     gui_mini_basic_functions.h
*    \brief    General user interface - basic functions
*    Created:  22/6/2014
*    Author:   Mathieu Stephan
*/


#ifndef MINI_GUI_BASIC_FUNCTIONS_H_
#define MINI_GUI_BASIC_FUNCTIONS_H_

#include "defines.h"
#ifdef MINI_VERSION

/* The following ENTRY_* values are used in the text-entry routine to define
 * the usable charset with special actions */

/* Global text/int entry definitions */
#define ENTRY_LONGCLICK_ERASE 2     /* number of wheel long clicks required to clear text entry field */
#define ENTRY_FIRST_CHAR      36    /* offset of initial selectable character -- 33 + 3 specials : 'A' */
#define ENTRY_NB_SPECIAL_CHAR 3     /* number of special characters (3: backspace, cancel, confirm) */

/* Font charset length definitions */
#define ENTRY_CHARSET_LENGTH  95                        /* usable font charset length -- 95 total chars in charset */
#define ENTRY_CHARSET_MAX     ENTRY_CHARSET_LENGTH - 1  /* maximum font char index */

/* Final charset (with special chars) length definitions */
#define ENTRY_NB_CHAR         ENTRY_CHARSET_LENGTH + ENTRY_NB_SPECIAL_CHAR /* charset + special chars */
#define ENTRY_LAST_CHAR       ENTRY_NB_CHAR - 1         /* last selectable character index, including specials */

/* Special character index definition */
#define ENTRY_CHAR_OK        ENTRY_CHARSET_LENGTH       /* index of special character: "OK" */
#define ENTRY_CHAR_CANCEL    ENTRY_CHARSET_LENGTH + 1   /* index of special character: "XX" */
#define ENTRY_CHAR_BACKSPACE ENTRY_CHARSET_LENGTH + 2   /* index of special character: "<-" */

/* Special characters appearance */
#define ENTRY_CHAR_OK_STR        "OK"   /* special character string used for display */
#define ENTRY_CHAR_CANCEL_STR    "XX"   /* special character string used for display */
#define ENTRY_CHAR_BACKSPACE_STR "<-"   /* special character string used for display */

RET_TYPE miniTextEntry(char * dst, uint8_t buflen, uint8_t filled_len, uint8_t min, uint8_t max, char * question);
void activityDetectedRoutine(void);
uint8_t isScreenSaverOn(void);
void guiMainLoop(void);

#endif
#endif /* MINI_GUI_BASIC_FUNCTIONS_H_ */