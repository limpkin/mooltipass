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

#include "aes256_ctr.h"
#include "smartcard.h"
#include "defines.h"

/** Defines **/
#define SMCID_UID_MATCH_ENTRY_LENGTH    (1 + SMARTCARD_CPZ_LENGTH + AES256_CTR_LENGTH)
#define CHECK_PASSWORD_TIMER_VAL        4000
#define CREDENTIAL_TIMER_VALIDITY       1000
#define AES_ENCR_DECR_TIMER_VAL         300
#define CTR_FLASH_MIN_INCR              64

/** Prototypes **/
RET_TYPE getUserIdFromSmartCardCPZ(uint8_t* buffer, uint8_t* nonce, uint8_t* userid);
RET_TYPE writeSmartCardCPZForUserId(uint8_t* buffer, uint8_t* nonce, uint8_t userid);
RET_TYPE checkPasswordForContext(uint8_t* password, uint8_t length);
RET_TYPE setPasswordForContext(uint8_t* password, uint8_t length);
void initEncryptionHandling(uint8_t* aes_key, uint8_t* nonce);
RET_TYPE setLoginForContext(uint8_t* name, uint8_t length);
RET_TYPE setCurrentContext(uint8_t* name, uint8_t length);
RET_TYPE addNewUserAndNewSmartCard(uint16_t pin_code);
RET_TYPE addNewContext(uint8_t* name, uint8_t length);
RET_TYPE initUserFlashContext(uint8_t user_id);
RET_TYPE getPasswordForContext(char  *buffer);
uint8_t getSmartCardInsertedUnlocked(void);
RET_TYPE getLoginForContext(char *buffer);
void clearSmartCardInsertedUnlocked(void);
void setSmartCardInsertedUnlocked(void);
void firstTimeUserHandlingInit(void);
uint8_t getNumberOfKnownUsers(void);
uint8_t getNumberOfKnownCards(void);
RET_TYPE findUserId(uint8_t userid);
void eraseFlashUsersContents(void);
void userHandlingTick(void);

#endif /* USERHANDLING_H_ */
