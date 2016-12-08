from mooltipass_hid_device import *
from Crypto.PublicKey import RSA
from mooltipass_defines import *
from os.path import isfile, join
from array import array
from time import sleep
from os import listdir
try:
	import seccure
except ImportError:
	pass
import pyqrcode
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
	

def pickle_write(data, outfile):
	f = open(outfile, "w+b")
	pickle.dump(data, f)
	f.close()
		
def pickle_read(filename):
	f = open(filename)
	data = pickle.load(f)
	f.close()
	return data
	
# Decrypt production file
def decryptMiniProdFile(key):	
	# List files in the export folder
	export_file_names = [f for f in listdir("export/") if isfile(join("export/", f))]
		
	# Ask for Mooltipass ID
	mooltipass_ids = raw_input("Enter Mooltipass ID: ")
	
	# Generate a list with Mooltipass IDs
	mooltipass_ids_list = mooltipass_ids.split(' ')
	
	# Find Mooltipass ID in files
	for mooltipass_id in mooltipass_ids_list:
		for file_name in export_file_names:
			if "Mooltipass-" + mooltipass_id in file_name:
				print "Found export file:", file_name
				data = pickle_read(join("export/",file_name))			
				decrypted_data = seccure.decrypt(data, key, curve='secp521r1/nistp521')
				items = decrypted_data.split('|')
				#print decrypted_data
				# Mooltipass ID | aes key 1 | aes key 2 | request ID key | UID, flush write
				
				if True:
					# Print Mooltipass ID | aes key 1 | aes key 2 | request ID key | UID (might get quite big)
					data_qr = pyqrcode.create(decrypted_data)
					print(key1_qr.terminal(quiet_zone=1))
					raw_input("Press enter:")
				else:
					# This is just here in case you need it....
					# key1
					key1 = items[1]
					key1_qr = pyqrcode.create(key1)
					print ""
					print "AES key 1:", key1
					print(key1_qr.terminal(quiet_zone=1))
					
					# key2
					key2 = items[2]
					key2_qr = pyqrcode.create(key2)
					print ""
					print "AES key 2:", key2
					print(key2_qr.terminal(quiet_zone=1))
					
					# Request UID
					request = items[3]
					request_qr = pyqrcode.create(request)
					print "Request UID key:", request
					print(request_qr.terminal(quiet_zone=1))
					
					# UID
					request = items[4]
					request_qr = pyqrcode.create(request)
					print "UID :", request
					print(request_qr.terminal(quiet_zone=1))

		
# Get a packet to send for a given command and payload
def mpmInitGetPacketForCommand(cmd, len, data):
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

def mooltipassMiniInit(mooltipass_device):	
	# Check for public key
	if not os.path.isfile("publickey.bin"):
		print "Couldn't find public key!"
		return
			
	# Check for export folder
	if not os.path.isdir("export"):
		print "Couldn't find export folder"
		return
	
	# Check for update bundle
	if not os.path.isfile("updatefile.img"):
		print "Couldn't find udpate file!"
		return
			
	# Read public key
	public_key = pickle_read("publickey.bin")

	# Loop
	try:
		temp_bool = 0
		while temp_bool == 0:
			# Operation success state
			success_status = True

			# Empty set password packet
			mooltipass_password = array('B')

			# We need 62 random bytes to set them as a password for the Mooltipass
			sys.stdout.write('Step 1... ')
			sys.stdout.flush()
			#print "Getting first random half"
			mooltipass_device.getInternalDevice().sendHidPacket(mpmInitGetPacketForCommand(CMD_GET_RANDOM_NUMBER, 0, None))
			data = mooltipass_device.getInternalDevice().receiveHidPacketWithTimeout()
			mooltipass_password.extend(data[DATA_INDEX:DATA_INDEX+32])
			#print "Getting second random half"
			mooltipass_device.getInternalDevice().sendHidPacket(mpmInitGetPacketForCommand(CMD_GET_RANDOM_NUMBER, 0, None))
			data2 = mooltipass_device.getInternalDevice().receiveHidPacketWithTimeout()
			mooltipass_password.extend(data2[DATA_INDEX:DATA_INDEX+30])
			#print "Getting random number for UID & request key"
			request_key_and_uid = array('B')
			mooltipass_device.getInternalDevice().sendHidPacket(mpmInitGetPacketForCommand(CMD_GET_RANDOM_NUMBER, 0, None))
			request_key_and_uid.extend(mooltipass_device.getInternalDevice().receiveHidPacketWithTimeout()[DATA_INDEX:DATA_INDEX+24])

			# Check that we actually received data
			if data == None or data2 == None:
				success_status = False
				print "fail!!!"
				print "likely causes: defective crystal or power supply"

			# Send our bundle
			if success_status == True:
				sys.stdout.write('Step 2... ')
				sys.stdout.flush()
				
				# Upload bundle, password is not used in that context
				success_status = mooltipass_device.uploadBundle("0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000", "updatefile.img", False)
				
				# For the mini version this procedure doesn't check the last return packet because in normal mode the device reboots
				if success_status == True:
					if mooltipass_device.getInternalDevice().receiveHidPacketWithTimeout()[DATA_INDEX] == 0x01:
						success_status = True
					else:
						success_status = False
						print "last packet fail!!!"
						print "likely causes: problem with external flash"
				else:
					success_status = False
					print "fail!!!"
					print "likely causes: problem with external flash"

			# Inform the Mooltipass that the bundle is sent so it can start functional test
			if success_status == True:
				sys.stdout.write('Step 3... ')
				sys.stdout.flush()
				magic_key = array('B')
				magic_key.append(0)
				magic_key.append(187)
				mooltipass_device.getInternalDevice().sendHidPacket(mpmInitGetPacketForCommand(CMD_SET_MOOLTIPASS_PARM, 2, magic_key))
				if mooltipass_device.getInternalDevice().receiveHidPacket()[DATA_INDEX] == 0x01:
					success_status = True
					print ""
				else:
					success_status = False
					print "fail!!!"
					print "likely causes: none"
			
			# Mooltipass Mini doesn't have LEDs anymore
			#if success_status == True:
			#	# Force tester to look at the LEDs
			#	raw_input("Press enter if LED1 is on: ")
			#	magic_key = array('B')
			#	magic_key.append(0)
			#	magic_key.append(149)
			#	mooltipass_device.getInternalDevice().sendHidPacket(mpmInitGetPacketForCommand(CMD_SET_MOOLTIPASS_PARM, 2, magic_key))
			#	mooltipass_device.getInternalDevice().receiveHidPacket()[DATA_INDEX]
			#	raw_input("Press enter if LED2 is on: ")
			#	magic_key = array('B')
			#	magic_key.append(0)
			#	magic_key.append(150)
			#	mooltipass_device.getInternalDevice().sendHidPacket(mpmInitGetPacketForCommand(CMD_SET_MOOLTIPASS_PARM, 2, magic_key))
			#	mooltipass_device.getInternalDevice().receiveHidPacket()[DATA_INDEX]
			#	raw_input("Press enter if LED3 is on: ")
			#	magic_key = array('B')
			#	magic_key.append(0)
			#	magic_key.append(151)
			#	mooltipass_device.getInternalDevice().sendHidPacket(mpmInitGetPacketForCommand(CMD_SET_MOOLTIPASS_PARM, 2, magic_key))
			#	mooltipass_device.getInternalDevice().receiveHidPacket()[DATA_INDEX]
			#	raw_input("Press enter if LED4 is on: ")
			#	magic_key = array('B')
			#	magic_key.append(0)
			#	magic_key.append(152)
			#	mooltipass_device.getInternalDevice().sendHidPacket(mpmInitGetPacketForCommand(CMD_SET_MOOLTIPASS_PARM, 2, magic_key))
			#	mooltipass_device.getInternalDevice().receiveHidPacket()[DATA_INDEX]

			# Wait for the mooltipass to inform the script that the test was successfull
			if success_status == True:
				temp_bool2 = False
				sys.stdout.write('Please follow the instructions on the mooltipass screen...')
				sys.stdout.flush()
				while temp_bool2 != True:
					test_result = mooltipass_device.getInternalDevice().receiveHidPacketWithTimeout()
					if test_result == None:
						sys.stdout.write('.')
						sys.stdout.flush()
					else:
						if test_result[CMD_INDEX] == CMD_FUNCTIONAL_TEST_RES and test_result[DATA_INDEX] == 0:
							success_status = True
							print " ok!"
						else:
							success_status = False
							print " fail!!!"
							print "Please look at the screen to know the cause"
						temp_bool2 = True
					
			# Send set password packet
			if success_status == True:
				sys.stdout.write('Step 4... ')
				sys.stdout.flush()
				# TO REMOVE
				#request_key_and_uid = [0]*24
				mooltipass_device.getInternalDevice().sendHidPacket(mpmInitGetPacketForCommand(CMD_SET_UID, 24, request_key_and_uid))
				if mooltipass_device.getInternalDevice().receiveHidPacket()[DATA_INDEX] == 0x01:
					# Update Success status
					success_status = True
				else:
					success_status = False
					print "fail!!!"
					print "likely causes: mooltipass already setup"

			# Send set password packet
			if success_status == True:
				sys.stdout.write('Step 5...\r\n')
				sys.stdout.flush()
				# TO REMOVE
				#mooltipass_password = [0]*62
				mooltipass_device.getInternalDevice().sendHidPacket(mpmInitGetPacketForCommand(CMD_SET_BOOTLOADER_PWD, 62, mooltipass_password))
				#print mooltipass_password
				if mooltipass_device.getInternalDevice().receiveHidPacket()[DATA_INDEX] == 0x01:
					# Set password success, ask mooltipass id
					mp_id = None
					while mp_id == None:
						try :
							mp_id = int(raw_input("Enter Mooltipass ID: MPM-"))
						except ValueError :
							mp_id = None
							print "Wrong entered value, please try again"
					# Write in file: Mooltipass ID | aes key 1 | aes key 2 | request ID key | UID, flush write
					aes_key1 = "".join(format(x, "02x") for x in mooltipass_password[:32])
					aes_key2 = "".join(format(x, "02x") for x in mooltipass_password[32:]) + "".join(format(x, "02x") for x in request_key_and_uid[22:])
					request_uid_key = "".join(format(x, "02x") for x in request_key_and_uid[0:16])
					uid = "".join(format(x, "02x") for x in request_key_and_uid[16:22])
					string_export = str(mp_id)+"|"+ aes_key1 +"|"+ aes_key2 +"|"+ request_uid_key +"|"+ uid +"\r\n"
					#print string_export
					pickle_write(seccure.encrypt(string_export, public_key, curve='secp521r1/nistp521'), time.strftime("export/%Y-%m-%d-%H-%M-%S-Mooltipass-")+str(mp_id)+".txt")					
					# Update Success status
					success_status = True
				else:
					success_status = False
					print "fail!!!"
					print "likely causes: mooltipass already setup"

			if success_status == True:
				# Let the user know it is done
				print "Setting up Mooltipass MPM-"+str(mp_id).zfill(4)+" DONE"
				# Increment Mooltipass ID
				mp_id = mp_id + 1
			else:
				print "|!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!|"
				print "|---------------------------------------------------------|"
				print "|---------------------------------------------------------|"
				print "|Setting up Mooltipass MPM-"+"XXXX"+" FAILED                  |"
				print "|                                                         |"                     
				print "|           PLEASE PUT AWAY THIS MOOLTIPASS!!!!           |"                     
				print "|---------------------------------------------------------|"
				print "|---------------------------------------------------------|"
				print "|!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!|"

			# Disconnect this device
			print "\r\nPlease disconnect this Mooltipass"

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
			print "Connect other Mooltipass"

			# Wait for findHidDevice to return something
			temp_bool2 = False;
			while temp_bool2 == False:
				temp_bool2 = mooltipass_device.connect(False)
				time.sleep(.5)

			# Delay
			time.sleep(1)

			# New Mooltipass detected
			print "New Mooltipass detected"
			print ""
	except KeyboardInterrupt:
		print "File written, everything ok"