from mooltipass_hid_device import *
from mooltipass_defines import *
import firmwareBundlePackAndSign
from Crypto.Cipher import AES
from intelhex import IntelHex
from array import array
from time import sleep
import platform
import usb.core
import usb.util
import os.path
import random
import struct
import string
import pickle
import copy
import time
import sys
import os
	
	
# Wait for device disconnect and reconnect
def mpmSecWaitForDeviceDisconRecon(mooltipass_device):
	print "Waiting for disconnect event"

	# Wait for no answer to ping
	temp_bool2 = 0
	while temp_bool2 == 0:
		try :
			# Send ping packet
			mooltipass_device.pingMooltipass()
		except usb.core.USBError as e:
			#print e
			temp_bool2 = 1
		time.sleep(.5)

	# Connect another device
	print "Waiting for connect event"

	# Wait for findHidDevice to return something
	temp_bool2 = False;
	while temp_bool2 == False:
		temp_bool2 = mooltipass_device.connect(False)
		time.sleep(.5)
		
# Get a packet to send for a given command and payload
def mpmSecGetPacketForCommand(cmd, len, data):
	# data to send
	arraytosend = array('B')

	# if command copy it otherwise copy the data
	if cmd != 0:
		arraytosend.append(len)
		arraytosend.append(cmd)

	# add the data
	if data is not None:
		arraytosend.extend(data)
		
	return arraytosend
	
# Get version number from hex file
def mpmSecGetVersionNumberFromHex(firmwareName):
	firmware = IntelHex(firmwareName)
	firmware_bin = firmware.tobinarray()
	for i in range(len(firmware_bin) - 3):
		if chr(firmware_bin[i]) == 'v' and chr(firmware_bin[i + 1]) >= '1' and chr(firmware_bin[i + 1]) <= '9' and chr(firmware_bin[i + 2]) == '.' and chr(firmware_bin[i + 3]) >= '0' and chr(firmware_bin[i + 3]) <= '9':
			firmware_version = "".join(map(chr, firmware_bin[i:i+4]))
			return firmware_version
	return ""
	
# Generate password for bundle upload
def generateBundlePasswordUpload(firmware_version, UIDKey, AESKey):
	to_be_encrypted = array('B')
	for i in range(AES_BLOCK_SIZE):
		to_be_encrypted.append(0)
	
	# Create firmware_version + UIDKey string
	for i in range(len(firmware_version)):
		to_be_encrypted[i] = ord(firmware_version[i])
	for i in range(UID_KEY_SIZE):
		to_be_encrypted[len(firmware_version) + i] = UIDKey[i]	
		
	# Encrypt string
	cipher = AES.new(AESKey, AES.MODE_CBC, array('B',[0]*AES.block_size))
	encrypted = cipher.encrypt(to_be_encrypted)[-AES.block_size:]
	
	# Format it as hexachar string
	password = array('B')
	for i in range(len(encrypted)):
		password.append(ord(encrypted[i]))
	text_password = "".join(format(x, "02x") for x in password)
	print text_password
	return text_password

def mooltipassMiniSecCheck(mooltipass_device, old_firmware, new_firmware, graphics_bundle, AESKey1, AESKey2, UIDReqKey, UIDKey):	
	# Check for valid firmware
	if not os.path.isfile(old_firmware):
		print "Wrong path to old firmware"
		return
		
	# Check for valid firmware
	if not os.path.isfile(new_firmware):
		print "Wrong path to new firmware"
		return	
		
	# Check versions
	old_firmware_version = mpmSecGetVersionNumberFromHex(old_firmware)
	new_firmware_version = mpmSecGetVersionNumberFromHex(new_firmware)
	print "Old FW version: " + old_firmware_version + ", new FW version: " + new_firmware_version
	if old_firmware_version > new_firmware_version:
		print "New FW version is older than old FW version!"
		return
		
	# Check Key sizes
	if len(AESKey1) != AES_KEY_SIZE*2:
		print "Wrong AES Key 1 size"
		return
	if len(AESKey2) != AES_KEY_SIZE*2:
		print "Wrong AES Key 2 size"
		return
	if len(UIDReqKey) != UID_REQUEST_KEY_SIZE*2:
		print "Wrong UID request key size"
		return
	if len(UIDKey) != UID_KEY_SIZE*2:
		print "Wrong UID key size"
		return
	
	# Convert string into arrays
	AESKey1_array = array('B', AESKey1.decode("hex"))
	AESKey2_array = array('B', AESKey2.decode("hex"))
	UIDReqKey_array = array('B', UIDReqKey.decode("hex"))
	UIDKey_array = array('B', UIDKey.decode("hex"))
	
	# Start requesting random data from the MP
	rand_aes_key1 = array('B')
	rand_aes_key2 = array('B')
	rand_req_uid_key = array('B')
	set_mooltipass_password_random_payload = array('B')
	mooltipass_device.getInternalDevice().sendHidPacket(mpmSecGetPacketForCommand(CMD_GET_RANDOM_NUMBER, 0, None))
	data = mooltipass_device.getInternalDevice().receiveHidPacketWithTimeout()
	rand_aes_key1.extend(data[0:AES_KEY_SIZE])
	mooltipass_device.getInternalDevice().sendHidPacket(mpmSecGetPacketForCommand(CMD_GET_RANDOM_NUMBER, 0, None))
	data = mooltipass_device.getInternalDevice().receiveHidPacketWithTimeout()
	rand_aes_key2.extend(data[0:AES_KEY_SIZE])
	mooltipass_device.getInternalDevice().sendHidPacket(mpmSecGetPacketForCommand(CMD_GET_RANDOM_NUMBER, 0, None))
	data = mooltipass_device.getInternalDevice().receiveHidPacketWithTimeout()
	rand_req_uid_key.extend(data[DATA_INDEX:DATA_INDEX+UID_REQUEST_KEY_SIZE])
	set_mooltipass_password_random_payload.extend(rand_aes_key1)
	set_mooltipass_password_random_payload.extend(rand_aes_key2[0:30])
	rand_aes_key1_string = "".join(format(x, "02x") for x in rand_aes_key1)
	rand_aes_key2_string = "".join(format(x, "02x") for x in rand_aes_key2)
	
	# Set read timeouts
	mooltipass_device.getInternalDevice().setReadTimeout(3333)
	
	# Test - set UID
	mooltipass_device.getInternalDevice().sendHidPacket(mpmSecGetPacketForCommand(CMD_SET_UID, (UID_REQUEST_KEY_SIZE+UID_KEY_SIZE), [0]*(UID_REQUEST_KEY_SIZE+UID_KEY_SIZE)))
	data = mooltipass_device.getInternalDevice().receiveHidPacketWithTimeout()
	if data == None or data[DATA_INDEX] != 0x01:
		print "OK - Couldn't set new UID"
	else:
		print "FAIL - Other UID set!"
		
	# Test - Get UID with erroneous req key
	mooltipass_device.getInternalDevice().sendHidPacket(mpmSecGetPacketForCommand(CMD_GET_UID, UID_REQUEST_KEY_SIZE, rand_req_uid_key))
	data = mooltipass_device.getInternalDevice().receiveHidPacket()
	if data[LEN_INDEX] == 0x01:
		print "OK - Couln't fetch UID with random key"
	else:
		print "FAIL - Fetched UID with random key"
	
	# Test - Get UID with good req key
	mooltipass_device.getInternalDevice().sendHidPacket(mpmSecGetPacketForCommand(CMD_GET_UID, UID_REQUEST_KEY_SIZE, UIDReqKey_array))
	data = mooltipass_device.getInternalDevice().receiveHidPacket()
	if data[LEN_INDEX] == 0x01:
		print "FAIL - Couln't fetch UID"
	else:
		print "OK - Fetched UID"
		if data[DATA_INDEX:DATA_INDEX+6] == UIDKey_array:
			print "OK - UID fetched is correct!"
		else:
			print "FAIL - UID fetched is different than the one provided!"
			print data[DATA_INDEX:DATA_INDEX+6], "instead of", UIDKey_array
	
	# Test - Set New AES KEY 1 & KEY 2		
	mooltipass_device.getInternalDevice().sendHidPacket(mpmSecGetPacketForCommand(CMD_SET_BOOTLOADER_PWD, 62, set_mooltipass_password_random_payload))
	data = mooltipass_device.getInternalDevice().receiveHidPacketWithTimeout()
	if data == None or data[DATA_INDEX] != 0x01:
		print "OK - Couldn't set new Mooltipass password"
	else:
		print "FAIL - New Mooltipass password was set"
	
	# Generate password for bundle upload
	text_password = generateBundlePasswordUpload(mooltipass_device.getMooltipassVersionAndVariant()[1], UIDKey_array, AESKey2_array)
		
	# Test nominal firmware update with good aes key and good password, no new key flag
	firmwareBundlePackAndSign.bundlePackAndSign(graphics_bundle, old_firmware, mooltipass_aes_key1, None, "updatefile.img", False)
	mooltipass_device.uploadBundle("0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000", "updatefile.img", False)
	mpmSecWaitForDeviceDisconRecon(mooltipass_device)
	usr_answer = raw_input("Tutorial displayed? [y/n]: ")
	if usr_answer == "y":
		print "OK - Same firmware upload with good AES key 1 & AES key 2 - no new key flag"
	else:
		print "FAIL - Same firmware upload with good AES key 1 & AES key 2 - no new key flag"
	# Still mooltipass_aes_key1
	
	# Generate password for bundle upload
	text_password = generateBundlePasswordUpload(mooltipass_device.getMooltipassVersionAndVariant()[1], UIDKey_array, AESKey2_array)
		
	# Test nominal firmware update with good aes key and good password 
	firmwareBundlePackAndSign.bundlePackAndSign(graphics_bundle, old_firmware, mooltipass_aes_key1, rand_aes_key1_string, "updatefile.img", False)
	mooltipass_device.uploadBundle("0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000", "updatefile.img", False)
	mpmSecWaitForDeviceDisconRecon(mooltipass_device)
	usr_answer = raw_input("Tutorial displayed? [y/n]: ")
	if usr_answer == "y":
		print "OK - Same firmware upload with good AES key 1 & AES key 2"
	else:
		print "FAIL - Same firmware upload with good AES key 1 & AES key 2"
	# Changed to rand_aes_key1_string
	
	# Generate password for bundle upload
	text_password = generateBundlePasswordUpload(mooltipass_device.getMooltipassVersionAndVariant()[1], UIDKey_array, AESKey2_array)
		
	# Test nominal firmware update with bad aes key and good password 
	firmwareBundlePackAndSign.bundlePackAndSign(graphics_bundle, old_firmware, mooltipass_aes_key1, rand_aes_key1_string, "updatefile.img", False)
	mooltipass_device.uploadBundle("0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000", "updatefile.img", False)
	mpmSecWaitForDeviceDisconRecon(mooltipass_device)
	usr_answer = raw_input("Tutorial displayed? [y/n]: ")
	if usr_answer == "y":
		print "FAIL - Same firmware upload with bad AES key 1 & good AES key 2"
	else:
		print "OK - Same firmware upload with bad AES key 1 & good AES key 2"
	# Still rand_aes_key1_string
	
	# Generate password for bundle upload
	text_password = generateBundlePasswordUpload(mooltipass_device.getMooltipassVersionAndVariant()[1], UIDKey_array, AESKey2_array)
		
	# Test nominal firmware update with good aes key and good password 
	firmwareBundlePackAndSign.bundlePackAndSign(graphics_bundle, new_firmware, rand_aes_key1_string, mooltipass_aes_key1, "updatefile.img", False)
	mooltipass_device.uploadBundle("0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000", "updatefile.img", False)
	mpmSecWaitForDeviceDisconRecon(mooltipass_device)
	usr_answer = raw_input("Tutorial displayed? [y/n]: ")
	if usr_answer == "y":
		print "OK - Newer firmware upload with good AES key 1 & AES key 2"
	else:
		print "FAIL - Newer firmware upload with good AES key 1 & AES key 2"
	# Changed to mooltipass_aes_key1
	
	# Generate password for bundle upload
	text_password = generateBundlePasswordUpload(mooltipass_device.getMooltipassVersionAndVariant()[1], UIDKey_array, AESKey2_array)
		
	# Test nominal firmware update with good aes key and good password but older firmware
	firmwareBundlePackAndSign.bundlePackAndSign(graphics_bundle, old_firmware, mooltipass_aes_key1, mooltipass_aes_key1, "updatefile.img", False)
	mooltipass_device.uploadBundle("0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000", "updatefile.img", False)
	mpmSecWaitForDeviceDisconRecon(mooltipass_device)
	usr_answer = raw_input("Tutorial displayed? [y/n]: ")
	if usr_answer == "y":
		print "FAIL - Older firmware upload with good AES key 1 & AES key 2"
	else:
		print "OK - Older firmware upload with good AES key 1 & AES key 2"
	# Still mooltipass_aes_key1
	
	# Timeout test for media upload
	mooltipass_device.getInternalDevice().sendHidPacket(mpmSecGetPacketForCommand(CMD_IMPORT_MEDIA_START, 62, [0]*62))
	print "Testing timeout... please wait 60 seconds"
	mpmSecWaitForDeviceDisconRecon(mooltipass_device)
	print "OK - Timeout Test"
	
	