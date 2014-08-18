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
/*!  \file     logic_eeprom.h
*    \brief    Firmware logic - eeprom related tasks
*    Created:  18/08/2014
*    Author:   Mathieu Stephan
*/



#ifndef LOGIC_EEPROM_H_
#define LOGIC_EEPROM_H_

#include "aes256_ctr.h"
#include "smartcard.h"
#include "defines.h"

/** Defines **/
// The entry is stored as CPZ -> CTR -> User ID
#define SMCID_UID_MATCH_ENTRY_LENGTH    (SMARTCARD_CPZ_LENGTH + AES256_CTR_LENGTH + 1)

/** Prototypes **/
RET_TYPE getUserIdFromSmartCardCPZ(uint8_t* buffer, uint8_t* nonce, uint8_t* userid);
RET_TYPE writeSmartCardCPZForUserId(uint8_t* buffer, uint8_t* nonce, uint8_t userid);
RET_TYPE addNewUserAndNewSmartCard(uint16_t pin_code);
void firstTimeUserHandlingInit(void);
RET_TYPE findUserId(uint8_t userid);


#endif /* LOGIC_EEPROM_H_ */