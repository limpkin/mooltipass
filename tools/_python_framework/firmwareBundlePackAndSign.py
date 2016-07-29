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

from os.path import isfile, join
from intelhex import IntelHex
from Crypto.Cipher import AES
from Crypto import Random
from array import array
from os import listdir
from struct import *
import sys
import os

def bundlePackAndSign(bundleName, firmwareName, oldAesKey, newAesKey, updateFileName, verbose):
	# Rather than at the beginning of the files, constants are here
	HASH_LENGH = 128/8									# Hash length (128 bits)
	AES_KEY_LENGTH = 256/8								# AES key length (256 bits)
	FW_VERSION_LENGTH = 4								# Length of the firmware version in the bundle
	AES_KEY_UPDATE_FLAG_LGTH = 1						# Length of the tag which specifies a firmware udpate
	FW_MAX_LENGTH = 28672								# Maximum firmware length, depends on size allocated to bootloader
	FLASH_SECTOR_0_LENGTH = 264*8						# Length in bytes of sector 0a in external flash (to change for 16Mb & 32Mb flash!)
	STORAGE_SPACE = 65536 - FLASH_SECTOR_0_LENGTH		# Uint16_t addressing space - sector 0a length (dedicated to other storage...)
	BUNDLE_MAX_LENGTH = STORAGE_SPACE - FW_MAX_LENGTH - HASH_LENGH - AES_KEY_LENGTH - FW_VERSION_LENGTH - AES_KEY_UPDATE_FLAG_LGTH
	
	# Robust RNG
	rng = Random.new()
	
	# Extracted firmware version
	firmware_version = None
	
	# AES Key Update Bool
	aes_key_update_bool = True

	# Check that all required files are here
	if isfile(bundleName):
		if verbose == True:
			print "Bundle file found"
	else:
		print "Couldn't find bundle file"
		return False
		
	if isfile(firmwareName):
		if verbose == True:
			print "Firmware file found"
	else:
		print "Couldn't find firmware file"
		return False
	
	# Read bundle and firmware data
	firmware = IntelHex(firmwareName)
	firmware_bin = firmware.tobinarray()
	fd = open(bundleName, 'rb')
	bundle = fd.read()
	fd.close()	
	
	# Check that the firmware data actually starts at address 0
	if firmware.minaddr() != 0:
		print "Firmware start address isn't correct"
		return False
		
	# Check that the bundle & firmware data aren't bigger than they should be
	if len(bundle) > BUNDLE_MAX_LENGTH:
		print "Bundle file too long:", len(bundle), "bytes long"
		return False
	else:	
		if verbose == True:
			print "Bundle file is", len(bundle), "bytes long"
		
	if len(firmware) > FW_MAX_LENGTH:
		print "Firmware file too long:", len(firmware), "bytes long"
		return False
	else:	
		if verbose == True:
			print "Firmware file is", len(firmware), "bytes long"
	
	if verbose == True:
		print "Remaining space in MCU flash:", FW_MAX_LENGTH - len(firmware), "bytes"
	
	if verbose == True:
		print "Remaining space in bundle:", STORAGE_SPACE - FW_MAX_LENGTH - HASH_LENGH - AES_KEY_LENGTH - FW_VERSION_LENGTH - AES_KEY_UPDATE_FLAG_LGTH - len(bundle), "bytes"
		
	# Beta testers devices have their aes key set to 00000... and the bootloader will always perform a key update
	if oldAesKey == "0000000000000000000000000000000000000000000000000000000000000000" and newAesKey == None:
		if verbose == True:
			print "Bundle update for beta testers unit, setting 00000... as new AES key"
		newAesKey = "0000000000000000000000000000000000000000000000000000000000000000"
		
	# If no new aes key is specified, don't set the aes key update flag
	if newAesKey == None:
		if verbose == True:
			print "No new AES key set"
		aes_key_update_bool = False
	else:
		if verbose == True:
			print "Encrypting new AES key"
		
	# If needed, check the new aes key
	if aes_key_update_bool == True:
		new_aes_key = array('B', newAesKey.decode("hex"))
		if len(new_aes_key) != AES_KEY_LENGTH:
			print "Wrong New AES Key Length:", len(new_aes_key)
			return False
		
	# Convert & check the old aes key
	old_aes_key = array('B', oldAesKey.decode("hex"))
	if len(old_aes_key) != AES_KEY_LENGTH:
		print "Wrong Old AES Key Length:", len(oldAesKey)
		return False

	# Get version number
	for i in range(len(firmware_bin) - 3):
		if chr(firmware_bin[i]) == 'v' and \
		chr(firmware_bin[i + 1]) >= '1' and chr(firmware_bin[i + 1]) <= '9' and \
		chr(firmware_bin[i + 2]) == '.' and \
		chr(firmware_bin[i + 3]) >= '0' and chr(firmware_bin[i + 3]) <= '9':
			firmware_version = firmware_bin[i:i+4]
			if verbose == True:
				print "Extracted firmware version:", "".join(chr(firmware_version[j]) for j in range(0, 4))
			break;
			
	# Check if we extracted the firmware version and it has the correct length
	if firmware_version == None or len(firmware_version) != FW_VERSION_LENGTH:
		print "Problem while extracting firmware version"
		return False

	# If needed, encrypt the new AES key with the old one
	if aes_key_update_bool == True:
		cipher = AES.new(old_aes_key, AES.MODE_ECB, array('B',[0]*AES.block_size))	# IV ignored in ECB
		enc_password = cipher.encrypt(new_aes_key)
		if len(enc_password) != AES_KEY_LENGTH:
			print "Encoded password is too long!"
			return False
	else:
		enc_password = [255]*AES_KEY_LENGTH
		
	# Generate beginning of update file data: bundle | padding | firmware version | new aes key bool | firmware | padding | new aes key encoded
	update_file_data = array('B')
	update_file_data.extend(bytearray(bundle))
	update_file_data.extend(array('B',[0]*(STORAGE_SPACE-HASH_LENGH-AES_KEY_LENGTH-FW_MAX_LENGTH-FW_VERSION_LENGTH-AES_KEY_UPDATE_FLAG_LGTH-len(bundle))))
	update_file_data.extend(firmware_version)
	if aes_key_update_bool == True:
		update_file_data.append(255)
	else:
		update_file_data.append(0)
	update_file_data.extend(firmware.tobinarray())
	update_file_data.extend(array('B',[0]*(STORAGE_SPACE-HASH_LENGH-AES_KEY_LENGTH-len(update_file_data))))
	update_file_data.extend(bytearray(enc_password))
	
	# Check length
	if len(update_file_data) != (STORAGE_SPACE - HASH_LENGH):
		print "Problem with update file length!"
		return False
		
	# Generate CBCMAC, IV is ZEROS
	cipher = AES.new(old_aes_key, AES.MODE_CBC, array('B',[0]*AES.block_size))
	cbc_mac = cipher.encrypt(update_file_data)[-AES.block_size:]
		
	# Append it to update file data: bundle | padding | firmware version | new aes key bool | firmware | padding | new aes key encoded | cbcmac
	update_file_data.extend(bytearray(cbc_mac))
	
	# Check length
	if len(update_file_data) != STORAGE_SPACE:
		print "Problem with update file length!"
		return False
		
	# Write our update image file
	data_fd = open(updateFileName, 'wb')
	data_fd.write(update_file_data)
	data_fd.close()
	if verbose == True:
		print "Update file written!"
	return True
	
	# Re read our file to make sure of its length
	#fd = open("updatefile.img", 'rb')
	#update_file = fd.read()
	#fd.close()	
	#print "Update file length:", len(update_file), "bytes"
	#return
