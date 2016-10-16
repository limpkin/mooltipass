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

#include "eeprom_addresses.h"
#include "aes256_ctr.h"
#include "smartcard.h"
#include "defines.h"

/** Enums **/
enum lock_feature_t                         {LF_EN_MASK = 0x01, LF_ENT_KEY_MASK = 0x02, LF_LOGIN_MASK = 0x04, LF_WIN_L_SEND_MASK = 0x08};

/** Defines **/
// The entry is stored as User ID -> CPZ -> CTR (25 bytes)
#define SMCID_UID_MATCH_ENTRY_LENGTH    (1 + SMARTCARD_CPZ_LENGTH + AES256_CTR_LENGTH)
// Total number of LUT entries. LUT is located near the end of the eeprom with reserved bytes at the end
#define NB_MAX_SMCID_UID_MATCH_ENTRIES  ((EEPROM_SIZE - EEP_SMC_IC_USER_MATCH_START_ADDR - EEPROM_END_RESERVED)/SMCID_UID_MATCH_ENTRY_LENGTH)
// Correct key to prevent mooltipass settings reinit
#define USER_PARAM_CORRECT_INIT_KEY         0xE8
// Mooltipass eeprom parameters define
#define USER_PARAM_INIT_KEY_PARAM           0
#define KEYBOARD_LAYOUT_PARAM               1
#define USER_INTER_TIMEOUT_PARAM            2
#define LOCK_TIMEOUT_ENABLE_PARAM           3
#define LOCK_TIMEOUT_PARAM                  4
#define TOUCH_DI_PARAM                      5
#define TOUCH_WHEEL_OS_PARAM_OLD            6
#define TOUCH_PROX_OS_PARAM                 7
#define OFFLINE_MODE_PARAM                  8
#define SCREENSAVER_PARAM                   9
#define TOUCH_CHARGE_TIME_PARAM             10
#define TOUCH_WHEEL_OS_PARAM0               11
#define TOUCH_WHEEL_OS_PARAM1               12
#define TOUCH_WHEEL_OS_PARAM2               13
#define FLASH_SCREEN_PARAM                  14
#define USER_REQ_CANCEL_PARAM               15
#define TUTORIAL_BOOL_PARAM                 16
#define SCREEN_SAVER_SPEED_PARAM            17
#define LUT_BOOT_POPULATING_PARAM           18
#define KEY_AFTER_LOGIN_SEND_BOOL_PARAM     19
#define KEY_AFTER_LOGIN_SEND_PARAM          20
#define KEY_AFTER_PASS_SEND_BOOL_PARAM      21
#define KEY_AFTER_PASS_SEND_PARAM           22
#define DELAY_AFTER_KEY_ENTRY_BOOL_PARAM    23
#define DELAY_AFTER_KEY_ENTRY_PARAM         24
#define INVERTED_SCREEN_AT_BOOT_PARAM       25
#define MINI_OLED_CONTRAST_CURRENT_PARAM    26
#define MINI_LED_ANIM_MASK_PARAM            27
#define MINI_KNOCK_DETECT_ENABLE_PARAM      28
#define MINI_KNOCK_THRES_PARAM              29
#define LOCK_UNLOCK_FEATURE_PARAM           30
#define HASH_DISPLAY_FEATURE_PARAM          31
#define RANDOM_INIT_PIN_PARAM               32
// we are full.
#define FIRST_USER_PARAM                    KEYBOARD_LAYOUT_PARAM

/** Prototypes **/
RET_TYPE getUserIdFromSmartCardCPZ(uint8_t* buffer, uint8_t* nonce, uint8_t* userid);
RET_TYPE writeSmartCardCPZForUserId(uint8_t* buffer, uint8_t* nonce, uint8_t userid);
uint8_t controlEepromParameter(uint8_t val, uint8_t lowerBound, uint8_t upperBound);
RET_TYPE findAvailableUserId(uint8_t* userid, uint8_t* nb_users_free);
RET_TYPE addNewUserForExistingCard(uint8_t* nonce, uint8_t* user_id);
void setMooltipassParameterInEeprom(uint8_t param, uint8_t val);
RET_TYPE addNewUserAndNewSmartCard(volatile uint16_t* pin_code);
uint8_t getMooltipassParameterInEeprom(uint8_t param);
void outputLUTEntriesForGivenUser(uint8_t userID);
void deleteUserIdFromSMCUIDLUT(uint8_t userid);
void firstTimeUserHandlingInit(void);
void mooltipassParametersInit(void);

#endif /* LOGIC_EEPROM_H_ */