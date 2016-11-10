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
/*!  \file     logic_aes_and_comms.h
*    \brief    Firmware logic - encryption and communications
*    Created:  18/08/2014
*    Author:   Mathieu Stephan
*/
#include "usb_cmd_parser.h"
#include "node_mgmt.h"

#ifndef LOGIC_ENCRYPTION_H_
#define LOGIC_ENCRYPTION_H_

/** Defines **/
#define CHECK_PASSWORD_TIMER_VAL        4000
#define CREDENTIAL_TIMER_VALIDITY       1000
#define AES_ENCR_DECR_TIMER_VAL         20     // Timed at 5ms!
#define CTR_FLASH_MIN_INCR              64
#define AES_ROUTINE_ENC_SIZE            32

#if AES_ROUTINE_ENC_SIZE != NODE_CHILD_SIZE_OF_PASSWORD
    #error "Wrong password size"
#endif
#if AES_ROUTINE_ENC_SIZE != DATA_NODE_BLOCK_SIZ
    #error "Wrong data node block size"
#endif

/** Prototypes **/
void computeAndDisplayBlockSizeEncryptionResult(uint8_t* aes_key, uint8_t* data, uint8_t stringId);
uint16_t searchForLoginInGivenParent(uint16_t parent_addr, uint8_t* name);
uint16_t searchForServiceName(uint8_t* name, uint8_t mode, uint8_t type);
RET_TYPE addDataForDataContext(uint8_t* data, uint8_t last_packet_flag);
RET_TYPE addNewContext(uint8_t* name, uint8_t length, uint8_t type);
void encryptOneAesBlockWithKeyEcb(uint8_t* aes_key, uint8_t* data);
RET_TYPE setPasswordForContext(uint8_t* password, uint8_t length);
void initEncryptionHandling(uint8_t* aes_key, uint8_t* nonce);
RET_TYPE setLoginForContext(uint8_t* name, uint8_t length);
RET_TYPE get32BytesDataForCurrentService(uint8_t* buffer);
RET_TYPE setCurrentContext(uint8_t* name, uint8_t type);
RET_TYPE setDescriptionForContext(uint8_t* description);
RET_TYPE checkPasswordForContext(uint8_t* password);
RET_TYPE getDescriptionForContext(char* buffer);
RET_TYPE getPasswordForContext(char* buffer);
uint8_t getSmartCardInsertedUnlocked(void);
void initUserFlashContext(uint8_t user_id);
RET_TYPE getLoginForContext(char* buffer);
void clearSmartCardInsertedUnlocked(void);
void setSmartCardInsertedUnlocked(void);
void eraseFlashUsersContents(void);
void ctrPreEncryptionTasks(void);
void favoritePickingLogic(void);
void loginSelectLogic(void);

#ifdef ENABLE_CREDENTIAL_MANAGEMENT
/* charset bitfield significance */
#define CHARSET_BIT_ALPHA_UPPER 0x0080 /* A-Z */
#define CHARSET_BIT_ALPHA_LOWER 0x0040 /* a-z */
#define CHARSET_BIT_NUM         0x0020 /* 0-9 */
#define CHARSET_BIT_SPECIALS1   0x0010 /* ?!,.:;*+-=/ */
#define CHARSET_BIT_SPECIALS2   0x0008 /* ()[]{}<> */
#define CHARSET_BIT_SPECIALS3   0x0004 /* \"'`^|~ */
#define CHARSET_BIT_SPECIALS4   0x0002 /* _#$%&@ */
#define CHARSET_BIT_SPACE       0x0001 /* 'whitespace' */

/* charset sizes */
#define CHARSET_SIZE_ALPHA_UPPER 26
#define CHARSET_SIZE_ALPHA_LOWER 26
#define CHARSET_SIZE_NUM         10
#define CHARSET_SIZE_SPECIALS1   11
#define CHARSET_SIZE_SPECIALS2   8
#define CHARSET_SIZE_SPECIALS3   7
#define CHARSET_SIZE_SPECIALS4   6
#define CHARSET_SIZE_SPACE       1

RET_TYPE askUserToSaveToFlash(pNode *p, cNode *c, uint16_t pAddr, uint16_t cAddr);
uint16_t askUserToSelectCharset(uint16_t original_flags);
RET_TYPE generateRandomPassword(uint8_t *password, uint8_t length, uint16_t flags);
void sendOrDisplayString(char* str, uint8_t is_password);
void managementActionPickingLogic(void);
void loginManagementSelectLogic(void);
#endif

#endif /* LOGIC_ENCRYPTION_H_ */