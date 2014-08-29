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
/*! \file   logic_fwflash_storage.h
 *  \brief  Logic for storing/getting fw data in the dedicated flash storage
 *  Copyright [2014] [Mathieu Stephan]
 */


#ifndef LOGIC_FWFLASH_STORAGE_H_
#define LOGIC_FWFLASH_STORAGE_H_

#include "defines.h"

// Defines
#define BITMAP_ID_OFFSET        64
// String IDs
#define ID_STRING_PROCESSING    0
#define ID_STRING_CARD_BLOCKED  1
#define ID_STRING_PB_CARD       2
#define ID_STRING_WRONG_PIN     3
#define ID_STRING_REMOVE_CARD   4
#define ID_STRING_INSERT_OTHER  5
#define ID_STRING_FAILED        6
#define ID_STRING_PIN_CHANGED   7
#define ID_STRING_PIN_NCGHANGED 8
#define ID_STRING_USER_ADDED    9
#define ID_STRING_USER_NADDED   10
#define ID_STRING_CARD_UNLOCKED 11
#define ID_STRING_CARDID_NFOUND 12
#define ID_STRING_INSERT_NCARD  13
#define ID_STRING_DONE          14
#define ID_STRING_CARD_REMOVED  15
#define ID_STRING_AREYOUSURE    16
#define ID_STRING_AREYOURLSURE  17
#define ID_STRING_OTHECARDFUSER 18
#define ID_STRING_ENTERLOGINQ   19
#define ID_STRING_ENTERPASSQ    20
#define ID_STRING_APPROVEMEMOP  21
#define ID_STRING_INSERT_PIN    22
#define ID_STRING_NEW_PINQ      23
#define ID_STRING_CONF_PIN      24
#define ID_STRING_CONFACCESSTO  25
#define ID_STRING_WITHTHISLOGIN 26
#define ID_STRING_CONF_NEWCREDS 27
#define ID_STRING_ADDUSERNAME   28
#define ID_STRING_ON            29
#define ID_STRING_CHANGEPASSFOR 30
#define ID_STRING_WRONGPIN1LEFT 31
#define ID_STRING_WRONGPIN2LEFT 32
#define ID_STRING_WRONGPIN3LEFT 33
#define ID_STRING_NEWMP_USER    34

// Prototypes
RET_TYPE getStoredFileAddr(uint16_t fileId, uint16_t* addr);
char* readStoredStringToBuffer(uint8_t stringID);

#endif /* LOGIC_FWFLASH_STORAGE_H_ */