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
#define CORRECT_BOOTKEY                     0xBEEF
#define BOOTLOADER_BOOTKEY                  0xD0D0
#define BOOTLOADER_PWDOK_KEY                0xAB

// Eeprom size
#define EEPROM_SIZE                         1024

// Boot key, 2 bytes long
#define EEP_BOOTKEY_ADDR                    0
// This is a boolean indicating if the bootloader password has been set, 1 byte long
#define EEP_BOOT_PWD_SET                    (EEP_BOOTKEY_ADDR + 2)
// This is the start address of the password bootloader, PACKET_EXPORT_SIZE long
#define EEP_BOOT_PWD                        (EEP_BOOT_PWD_SET + 1)
// This is the EEPROM address where we start to store user_id <> smart card id & AES nonce matches, one entry is SMCID_UID_MATCH_ENTRY_LENGTH long
#define EEP_SMC_IC_USER_MATCH_START_ADDR    (EEP_BOOT_PWD + PACKET_EXPORT_SIZE) 


#endif /* EEPROM_ADDRESSES_H_ */
