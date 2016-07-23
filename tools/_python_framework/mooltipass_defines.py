#!/usr/bin/env python2
#
# Copyright (c) 2016 Mathieu Stephan
# All rights reserved.
#
# CDDL HEADER START
#
# The contents of this file are subject to the terms of the
# Common Development and Distribution License (the "License").
# You may not use this file except in compliance with the License.
#
# You can obtain a copy of the license at src/license_cddl-1.0.txt
# or http://www.opensolaris.org/os/licensing.
# See the License for the specific language governing permissions
# and limitations under the License.
#
# When distributing Covered Code, include this CDDL HEADER in each
# file and include the License file at src/license_cddl-1.0.txt
# If applicable, add the following below this CDDL HEADER, with the
# fields enclosed by brackets "[]" replaced with your own identifying
# information: Portions Copyright [yyyy] [name of copyright owner]
#
# CDDL HEADER END

# Timeout for reading data in ms
USB_READ_TIMEOUT		= 10000

# Device VID & PID
USB_VID                 = 0x16D0
USB_PID                 = 0x09A0

# TLV Field indexes
LEN_INDEX               = 0x00
CMD_INDEX               = 0x01
DATA_INDEX              = 0x02

# Field Indexes
PREV_ADDRESS_INDEX      = 0x02
NEXT_ADDRESS_INDEX      = 0x04
NEXT_CHILD_INDEX        = 0x06
SERVICE_INDEX           = 0x08
DESC_INDEX              = 6
LOGIN_INDEX             = 37

# Sizes
NODE_SIZE				= 132
DEVICE_PASSWORD_SIZE	= 62

# Command IDs
CMD_EXPORT_FLASH_START  = 0x8A
CMD_EXPORT_FLASH        = 0x8B
CMD_EXPORT_FLASH_END    = 0x8C
CMD_IMPORT_FLASH_BEGIN  = 0x8D
CMD_IMPORT_FLASH        = 0x8E
CMD_IMPORT_FLASH_END    = 0x8F
CMD_EXPORT_EEPROM_START = 0x90
CMD_EXPORT_EEPROM       = 0x91
CMD_EXPORT_EEPROM_END   = 0x92
CMD_IMPORT_EEPROM_BEGIN = 0x93
CMD_IMPORT_EEPROM       = 0x94
CMD_IMPORT_EEPROM_END   = 0x95
CMD_ERASE_EEPROM        = 0x96
CMD_ERASE_FLASH         = 0x97
CMD_ERASE_SMC           = 0x98
CMD_DRAW_BITMAP         = 0x99
CMD_SET_FONT            = 0x9A
CMD_USB_KEYBOARD_PRESS  = 0x9B
CMD_STACK_FREE          = 0x9C
CMD_CLONE_SMARTCARD     = 0x9D
CMD_DEBUG               = 0xA0
CMD_PING                = 0xA1
CMD_VERSION             = 0xA2
CMD_CONTEXT             = 0xA3
CMD_GET_LOGIN           = 0xA4
CMD_GET_PASSWORD        = 0xA5
CMD_SET_LOGIN           = 0xA6
CMD_SET_PASSWORD        = 0xA7
CMD_CHECK_PASSWORD      = 0xA8
CMD_ADD_CONTEXT         = 0xA9
CMD_SET_BOOTLOADER_PWD  = 0xAA
CMD_JUMP_TO_BOOTLOADER  = 0xAB
CMD_GET_RANDOM_NUMBER   = 0xAC
CMD_START_MEMORYMGMT    = 0xAD
CMD_IMPORT_MEDIA_START  = 0xAE
CMD_IMPORT_MEDIA        = 0xAF
CMD_IMPORT_MEDIA_END    = 0xB0
CMD_SET_MOOLTIPASS_PARM = 0xB1
CMD_GET_MOOLTIPASS_PARM = 0xB2
CMD_RESET_CARD          = 0xB3
CMD_READ_CARD_LOGIN     = 0xB4
CMD_READ_CARD_PASS      = 0xB5
CMD_SET_CARD_LOGIN      = 0xB6
CMD_SET_CARD_PASS       = 0xB7
CMD_ADD_UNKNOWN_CARD    = 0xB8
CMD_MOOLTIPASS_STATUS   = 0xB9
CMD_FUNCTIONAL_TEST_RES = 0xBA
CMD_SET_DATE            = 0xBB
CMD_SET_UID             = 0xBC
CMD_GET_UID             = 0xBD
CMD_SET_DATA_SERVICE    = 0xBE
CMD_ADD_DATA_SERVICE    = 0xBF
CMD_WRITE_32B_IN_DN     = 0xC0
CMD_READ_32B_IN_DN      = 0xC1
CMD_GET_CUR_CPZ		    = 0xC2
CMD_READ_FLASH_NODE     = 0xC5
CMD_WRITE_FLASH_NODE    = 0xC6
CMD_GET_FAVORITE        = 0xC7
CMD_SET_FAVORITE        = 0xC8
CMD_GET_STARTING_PARENT = 0xC9
CMD_SET_STARTING_PARENT = 0xCA
CMD_GET_CTRVALUE        = 0xCB
CMD_SET_CTRVALUE        = 0xCC
CMD_ADD_CARD_CPZ_CTR    = 0xCD
CMD_GET_CARD_CPZ_CTR    = 0xCE
CMD_CARD_CPZ_CTR_PACKET = 0xCF
CMD_GET_30_FREE_SLOTS   = 0xD0
CMD_GET_DN_START_PARENT = 0xD1
CMD_SET_DN_START_PARENT = 0xD2
CMD_END_MEMORYMGMT      = 0xD3
CMD_GET_DESCRIPTION		= 0xD4
CMD_UNLOCK_WITH_PIN		= 0xD5