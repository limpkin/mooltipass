from mooltipass_hid_device import *
from mooltipass_defines import *
import firmwareBundlePackAndSign
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
	

def mpmSecPickeWrite(data, outfile):
	f = open(outfile, "w+b")
	pickle.dump(data, f)
	f.close()
		
def mpmSecPickleRead(filename):
	f = open(filename)
	data = pickle.load(f)
	f.close()
	return data
	
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

def mooltipassMiniSecCheck(mooltipass_device, old_firmware, new_firmware, graphics_bundle):	
	# Check for valid firmware
	if not os.path.isfile(old_firmware):
		print "Wrong path to old firmware"
		return
		
	# Check for valid firmware
	if not os.path.isfile(new_firmware):
		print "Wrong path to new firmware"
		return	

	# Check for private key
	if not os.path.isfile("key.bin"):
		print "Couldn't find private key!"
		return
		
	# Check for export folder
	if not os.path.isdir("export"):
		print "Couldn't find export folder"
		return
		
	# Check versions
	old_firmware_version = mpmSecGetVersionNumberFromHex(old_firmware)
	new_firmware_version = mpmSecGetVersionNumberFromHex(new_firmware)
	print "Old FW version: " + old_firmware_version + ", new FW version: " + new_firmware_version
	if old_firmware_version > new_firmware_version:
		print "New FW version is older than old FW version!"
		return
		
	# Read private key
	# to fix! we don't use rsa anymore!
	#key = RSA.importKey(mpmSecPickleRead("key.bin"))
	
	# Ask user to choose export file
	file_list = glob.glob("export/*.txt")
	if len(file_list) == 0:
		print "No init file available!"
		return False
	elif len(file_list) == 1:
		print "Using init file", file_list[0]
		initdata = mpmSecPickleRead(file_list[0])
	else:
		for i in range(0, len(file_list)):
			print str(i) + ": " + file_list[i]
		picked_file = raw_input("Choose file: ")
		if int(picked_file) >= len(file_list):
			print "Out of bounds"
			return False
		else:
			initdata = mpmSecPickleRead(file_list[int(picked_file)])
	
	# Decrypt init data
	decrypted_data = key.decrypt(initdata)
	items = decrypted_data.split('|')
	mooltipass_id = items[0]
	mooltipass_aes_key1 = items[1]
	mooltipass_aes_key2 = items[2]
	mooltipass_req_uid_key = items[3]
	mooltipass_uid = items[4].strip()
	print "MPM ID: " + mooltipass_id 
	print "AES KEY 1: " + mooltipass_aes_key1 
	print "AES KEY 2: " + mooltipass_aes_key2
	print "REQ KEY: " + mooltipass_req_uid_key
	print "UID: " + mooltipass_uid
	
	# Start requesting random data from the MP
	rand_aes_key1 = array('B')
	rand_aes_key2 = array('B')
	rand_req_uid_key = array('B')
	set_mooltipass_password_random_payload = array('B')
	mooltipass_device.getInternalDevice().sendHidPacket(mpmSecGetPacketForCommand(CMD_GET_RANDOM_NUMBER, 0, None))
	data = mooltipass_device.getInternalDevice().receiveHidPacketWithTimeout()
	rand_aes_key1.extend(data[0:32])
	mooltipass_device.getInternalDevice().sendHidPacket(mpmSecGetPacketForCommand(CMD_GET_RANDOM_NUMBER, 0, None))
	data = mooltipass_device.getInternalDevice().receiveHidPacketWithTimeout()
	rand_aes_key2.extend(data[0:32])
	mooltipass_device.getInternalDevice().sendHidPacket(mpmSecGetPacketForCommand(CMD_GET_RANDOM_NUMBER, 0, None))
	data = mooltipass_device.getInternalDevice().receiveHidPacketWithTimeout()
	rand_req_uid_key.extend(data[DATA_INDEX:DATA_INDEX+16])
	set_mooltipass_password_random_payload.extend(rand_aes_key1)
	set_mooltipass_password_random_payload.extend(rand_aes_key2[0:30])
	rand_aes_key1_string = "".join(format(x, "02x") for x in rand_aes_key1)
	rand_aes_key2_string = "".join(format(x, "02x") for x in rand_aes_key2)
	
	# Test - set UID
	mooltipass_device.getInternalDevice().sendHidPacket(mpmSecGetPacketForCommand(CMD_SET_UID, 22, [0]*22))
	if mooltipass_device.getInternalDevice().receiveHidPacketWithTimeout()[DATA_INDEX] == 0x01:
		print "FAIL - Other UID set!"
	else:
		print "OK - Couldn't set new UID"
		
	# Test - Get UID with erroneous req key
	mooltipass_device.getInternalDevice().sendHidPacket(mpmSecGetPacketForCommand(CMD_GET_UID, 16, rand_req_uid_key))
	data = mooltipass_device.getInternalDevice().receiveHidPacket()
	if data[LEN_INDEX] == 0x01:
		print "OK - Couln't fetch UID with random key"
	else:
		print "FAIL - Fetched UID with random key"
	
	# Test - Get UID with good req key
	mooltipass_device.getInternalDevice().sendHidPacket(mpmSecGetPacketForCommand(CMD_GET_UID, 16, array('B', mooltipass_req_uid_key.decode("hex"))))
	data = mooltipass_device.getInternalDevice().receiveHidPacket()
	if data[LEN_INDEX] == 0x01:
		print "FAIL - Couln't fetch UID"
	else:
		print "OK - Fetched UID"
		if data[DATA_INDEX:DATA_INDEX+6] == array('B', mooltipass_uid.decode("hex")):
			print "OK - UID fetched is correct!"
		else:
			print "FAIL - UID fetched is different than the one provided!"
			print data[DATA_INDEX:DATA_INDEX+6], "instead of", array('B', mooltipass_uid.decode("hex"))
	
	# Test - Set New AES KEY 1 & KEY 2		
	mooltipass_device.getInternalDevice().sendHidPacket(mpmSecGetPacketForCommand(CMD_SET_BOOTLOADER_PWD, 62, set_mooltipass_password_random_payload))
	if mooltipass_device.getInternalDevice().receiveHidPacket()[DATA_INDEX] == 0x01:
		print "FAIL - New Mooltipass password was set"
	else:
		print "OK - Couldn't set new Mooltipass password"
		
	# Todo: implement password upload check
		
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
	
	