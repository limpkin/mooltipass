#!/usr/bin/env python
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

def bundlePackAndSign(bundleName, firmwareName, oldAesKey, newAesKey, updateFileName):
	# Rather than at the beginning of the files, constants are here
	HASH_LENGH = 128/8									# Hash length (128 bits)
	AES_KEY_LENGTH = 256/8								# AES key length (256 bits)
	FW_MAX_LENGTH = 28672								# Maximum firmware length, depends on size allocated to bootloader
	FLASH_SECTOR_0_LENGTH = 264*8						# Length in bytes of sector 0a in external flash (to change for 16Mb & 32Mb flash!)
	STORAGE_SPACE = 65536 - FLASH_SECTOR_0_LENGTH		# Uint16_t addressing space - sector 0a length (dedicated to other storage...)
	BUNDLE_MAX_LENGTH = STORAGE_SPACE - FW_MAX_LENGTH - HASH_LENGH - AES_KEY_LENGTH
	
	# Robust RNG
	rng = Random.new()

	# Check that all required files are here
	if isfile(bundleName):
		print "Bundle file found"
	else:
		print "Couldn't find bundle file"
		return False
		
	if isfile(firmwareName):
		print "Firmware file found"
	else:
		print "Couldn't find firmware file"
		return False
	
	# Read bundle and firmware data
	firmware = IntelHex(firmwareName)
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
		print "Bundle file is ", len(bundle), "bytes long"
		
	if len(firmware) > FW_MAX_LENGTH:
		print "Firmware file too long:", len(firmware), "bytes long"
		return False
	else:	
		print "Firmware file is ", len(firmware), "bytes long"
		
	# Convert & check the provided aes keys
	new_aes_key = array('B', newAesKey.decode("hex"))
	if len(new_aes_key) != AES_KEY_LENGTH:
		print "Wrong New AES Key Length:", len(new_aes_key)
		return False
		
	old_aes_key = array('B', oldAesKey.decode("hex"))
	if len(old_aes_key) != AES_KEY_LENGTH:
		print "Wrong Old AES Key Length:", len(oldAesKey)
		return False
		
	cipher = AES.new(old_aes_key, AES.MODE_ECB, array('B',[0]*AES.block_size))	# IV ignored in ECB
	enc_password = cipher.encrypt(new_aes_key)
	if len(enc_password) != AES_KEY_LENGTH:
		print "Encoded password is too long!"
		return False
		
	# Generate beginning of update file data: bundle | padding | firmware | new aes key encoded
	update_file_data = array('B')
	update_file_data.extend(bytearray(bundle))
	update_file_data.extend(array('B',[0]*(STORAGE_SPACE-HASH_LENGH-AES_KEY_LENGTH-FW_MAX_LENGTH-len(bundle))))
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
		
	# Append it to update file data: bundle | padding | firmware | new aes key encoded | cbcmac
	update_file_data.extend(bytearray(cbc_mac))
	
	# Check length
	if len(update_file_data) != STORAGE_SPACE:
		print "Problem with update file length!"
		return False
		
	# Write our update image file
	data_fd = open(updateFileName, 'wb')
	data_fd.write(update_file_data)
	data_fd.close()
	print "Update file written!"
	return True
	
	# Re read our file to make sure of its length
	#fd = open("updatefile.img", 'rb')
	#update_file = fd.read()
	#fd.close()	
	#print "Update file length:", len(update_file), "bytes"
	#return
