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
/*!  \file     eeprom_addresses.h
*    \brief    Data addresses in EEPROM
*    Created:  09/6/2014
*    Author:   Mathieu Stephan
*/


#ifndef EEPROM_ADDRESSES_H_
#define EEPROM_ADDRESSES_H_

#include "usb_cmd_parser.h"

// EEPROM defines
#define CORRECT_BOOTKEY                     0xDEAD
#define BOOTLOADER_BOOTKEY                  0xD0D0
#define BOOTLOADER_PWDOK_KEY                0xAB
#define USER_RESERVED_SPACE_IN_EEP          34

// Eeprom size
#define EEPROM_SIZE                         1024

// Reserved space at the end of the eeprom
#define UID_REQUEST_KEY_SET_BOOL_SIZE       1
#define UID_REQUEST_KEY_SIZE                16
#define UID_SIZE                            6
#define BOOTKEY_SIZE                        2
#define EEPROM_END_RESERVED                 (UID_REQUEST_KEY_SET_BOOL_SIZE + UID_REQUEST_KEY_SIZE + UID_SIZE + BOOTKEY_SIZE)

// Boot key, 2 bytes long
#define EEP_BOOTKEY_ADDR                    0
// This is a boolean indicating if the bootloader password has been set, 1 byte long
#define EEP_BOOT_PWD_SET                    (EEP_BOOTKEY_ADDR + BOOTKEY_SIZE)
// This is the start address of the bootloader password, PACKET_EXPORT_SIZE long (62 bytes for 64 bytes packets)
#define EEP_BOOT_PWD                        (EEP_BOOT_PWD_SET + 1)
// This is the beginning of the zone containing various variables, USER_RESERVED_SPACE_IN_EEP long
#define EEP_USER_DATA_START_ADDR            (EEP_BOOT_PWD + PACKET_EXPORT_SIZE)
// This is the EEPROM address where we start to store user_id <> smart card id & AES nonce matches, one entry is SMCID_UID_MATCH_ENTRY_LENGTH long
#define EEP_SMC_IC_USER_MATCH_START_ADDR    (EEP_USER_DATA_START_ADDR + USER_RESERVED_SPACE_IN_EEP)

// Number of user_id <> smart card id & AES nonce entries is computed in this .h file
#include "logic_eeprom.h"

// This is the beginning of the reserved space that contains the uid request key, uid and backup bootkey
#define EEP_RESERVED_SPACE_START_ADDR       (EEP_SMC_IC_USER_MATCH_START_ADDR + (NB_MAX_SMCID_UID_MATCH_ENTRIES*SMCID_UID_MATCH_ENTRY_LENGTH))
// This is the boolean to know if the uid request key has been set
#define EEP_UID_REQUEST_KEY_SET_ADDR        (EEP_RESERVED_SPACE_START_ADDR)
// This is the key that needs to be presented to allow for UID presentation
#define EEP_UID_REQUEST_KEY_ADDR            (EEP_UID_REQUEST_KEY_SET_ADDR + UID_REQUEST_KEY_SET_BOOL_SIZE)
// This is the Mooltipass UID
#define EEP_UID_ADDR                        (EEP_UID_REQUEST_KEY_ADDR + UID_REQUEST_KEY_SIZE)
// This is a copy of the bootkey
#define EEP_BACKUP_BOOTKEY_ADDR             (EEP_UID_ADDR + UID_SIZE)

#endif /* EEPROM_ADDRESSES_H_ */
