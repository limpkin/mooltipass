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
/*!  \file     userhandling.h
*    \brief    Logic for user handling
*    Created:  09/6/2014
*    Author:   Mathieu Stephan
*/


#ifndef USERHANDLING_H_
#define USERHANDLING_H_

#include "defines.h"

/** Defines **/
#define SMCID_UID_MATCH_ENTRY_LENGTH    9

/** Prototypes **/
RET_TYPE getUserIdFromSmartCardCPZ(uint8_t* buffer, uint8_t* userid);
RET_TYPE writeSmartCardCPZForUserId(uint8_t* buffer, uint8_t userid);
void firstTimeUserHandlingInit(void);
uint8_t getNumberOfKnownUsers(void);
uint8_t getNumberOfKnownCards(void);
RET_TYPE findUserId(uint8_t userid);

#endif /* USERHANDLING_H_ */