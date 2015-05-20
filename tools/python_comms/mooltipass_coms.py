from array import array
from time import sleep
from Crypto.PublicKey import RSA
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
from keyboard import *

USB_VID                 = 0x16D0
USB_PID                 = 0x09A0

LEN_INDEX               = 0x00
CMD_INDEX               = 0x01
DATA_INDEX              = 0x02
PREV_ADDRESS_INDEX      = 0x02
NEXT_ADDRESS_INDEX      = 0x04
NEXT_CHILD_INDEX        = 0x06
SERVICE_INDEX           = 0x08
DESC_INDEX              = 6
LOGIN_INDEX             = 37
NODE_SIZE				= 132

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

def keyboardSend(epout, data1, data2):
	packetToSend = array('B')
	packetToSend.append(data1)
	packetToSend.append(data2)
	sendHidPacket(epout, CMD_USB_KEYBOARD_PRESS, 0x02, packetToSend)
	time.sleep(0.05)

def keyboardTestKey(epout, KEY, MODIFIER):
	if( KEY in KEYTEST_BAN_LIST ): return ''
	keyboardSend(epout, KEY, MODIFIER)
	keyboardSend(epout, KEY, MODIFIER)
	keyboardSend(epout, KEY_RETURN, 0)
	string = raw_input()
	if (string == ''):
		return string
	return string[0]

def keyboardKeyMap(epout, key):
	if ( (key & 0x3F) == KEY_EUROPE_2 ):
		if (key & (SHIFT_MASK|ALTGR_MASK) == (SHIFT_MASK|ALTGR_MASK)):
			return keyboardTestKey(epout, KEY_EUROPE_2_REAL, KEY_SHIFT|KEY_RIGHT_ALT)
		elif (key & SHIFT_MASK):
			return keyboardTestKey(epout, KEY_EUROPE_2_REAL, KEY_SHIFT)
		elif (key & ALTGR_MASK):
			return keyboardTestKey(epout, KEY_EUROPE_2_REAL, KEY_RIGHT_ALT)
		else:
			return keyboardTestKey(epout, KEY_EUROPE_2_REAL, 0)

	elif (key & (SHIFT_MASK|ALTGR_MASK) == (SHIFT_MASK|ALTGR_MASK)):
		return keyboardTestKey(epout, key & ~(SHIFT_MASK|ALTGR_MASK), KEY_SHIFT|KEY_RIGHT_ALT)
			
	elif (key & SHIFT_MASK):
		return keyboardTestKey(epout, key & ~SHIFT_MASK, KEY_SHIFT)

	if (key & ALTGR_MASK):
		return keyboardTestKey(epout, key & ~ALTGR_MASK, KEY_RIGHT_ALT)

	else:
		return keyboardTestKey(epout, key, 0)

def keyboardTest(epout):
	fileName = raw_input("Name of the Keyboard (example: ES): ");

	# dictionary to store the
	Layout_dict = dict()

	# No modifier combinations
	for bruteforce in range(KEY_EUROPE_2, KEY_SLASH+1):
		output = keyboardKeyMap(epout, bruteforce)
		if (output == ''): continue
		if output in Layout_dict:
			print "Already stored"
			print bruteforce
		else:
			Layout_dict.update({output: bruteforce})

	# SHIFT combinations
	for bruteforce in range(KEY_EUROPE_2, KEY_SLASH+1):
		output = keyboardKeyMap(epout, SHIFT_MASK|bruteforce)
		if (output == ''): continue
		if output in Layout_dict:
			print "Already stored"
			print SHIFT_MASK|bruteforce
		else:
			Layout_dict.update({output : SHIFT_MASK|bruteforce})

	# ALTGR combinations
	for bruteforce in range(KEY_EUROPE_2, KEY_SLASH+1):
		output = keyboardKeyMap(epout, ALTGR_MASK|bruteforce)
		if (output == ''): continue
		if output in Layout_dict:
			print "Already stored"
			print ALTGR_MASK|bruteforce
		else:
			Layout_dict.update({output : ALTGR_MASK|bruteforce})

	# ALTGR + SHIFT combinations
	for bruteforce in range(KEY_EUROPE_2, KEY_SLASH+1):
		output = keyboardKeyMap(epout, SHIFT_MASK|ALTGR_MASK|bruteforce)
		if (output == ''): continue
		if output in Layout_dict:
			print "Already stored"
			print SHIFT_MASK|ALTGR_MASK|bruteforce
		else:
			Layout_dict.update({output : SHIFT_MASK|ALTGR_MASK|bruteforce})


	hid_define_str = "const uint8_t PROGMEM keyboardLUT_"+fileName+"[95] = \n{\n"
	img_contents = array('B')

	for key in KeyboardAscii:
		if(key not in Layout_dict):
			#print key + " Not found"
			Layout_dict.update({key:0})
		#else:
			#print "BruteForced: " + key

		""" Format C code """
		keycode = hex(Layout_dict[key])+","

		# Write img file
		img_contents.append(Layout_dict[key])

		# Handle special case
		if(key == '\\'):
			comment = " // " + hex(ord(key)) + " '" + key + "'\n"
		else:
			comment = " // " + hex(ord(key)) + " " + key + "\n"

		newline = "%4s%-5s" % (" ", keycode) + comment
		# add new line into existing string
		hid_define_str = hid_define_str + newline

	# finish C array
	hid_define_str = hid_define_str + "};"
	print hid_define_str

	# finish img file
	img_file = open("_"+fileName+"_keyb_lut.img", "wb")
	img_file.write(img_contents)
	img_file.close()

	# Save C array into .c file
	text_file = open("keymap_"+fileName+".c", "w")
	text_file.write(hid_define_str)
	text_file.close()
	
def pickle_write(data, outfile):
        f = open(outfile, "w+b")
        pickle.dump(data, f)
        f.close()
		
def pickle_read(filename):
        f = open(filename)
        data = pickle.load(f)
        f.close()
        return data
		
def receiveHidPacket(epin):
	try :
		data = epin.read(epin.wMaxPacketSize, timeout=15000)
		return data
	except usb.core.USBError as e:
		#print e
		sys.exit("Mooltipass didn't send a packet")

def receiveHidPacketWithTimeout(epin):
	try :
		data = epin.read(epin.wMaxPacketSize, timeout=15000)
		return data
	except usb.core.USBError as e:
		return None


def sendHidPacket(epout, cmd, len, data):
	# data to send
	arraytosend = array('B')

	# if command copy it otherwise copy the data
	if cmd != 0:
		arraytosend.append(len)
		arraytosend.append(cmd)

	# add the data
	if data is not None:
		arraytosend.extend(data)

	#print arraytosend
	#print arraytosend

	# send data
	epout.write(arraytosend)

def sendCustomPacket(epin, epout):
	command = raw_input("CMD ID: ")
	packet = array('B')
	temp_bool = 0
	length = 0

	#fill packet
	packet.append(0)
	packet.append(int(command, 16))

	#loop until packet is filled
	while temp_bool == 0 :
		try :
			intval = int(raw_input("Byte %s: " %length), 16)
			packet.append(intval)
			length = length + 1
		except ValueError :
			temp_bool = 1

	#update packet length
	packet[0] = length

	#ask for how many packets to receiveHidPacket
	packetstoreceive = input("How many packets to be received: ")

	#send packet
	sendHidPacket(epout, 0, 0, packet)

	#receive packets
	for i in range (0, packetstoreceive):
		print receiveHidPacket(epin)

def	uploadBundle(epin, epout):
	# Empty set password packet
	mooltipass_password = array('B')
	for i in range(62):
		mooltipass_password.append(0)
	success_status = 0
	sendHidPacket(epout, CMD_IMPORT_MEDIA_START, 62, mooltipass_password)
	# Check that the import command worked
	if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
		# Open bundle file
		bundlefile = open('bundle.img', 'rb')
		packet_to_send = array('B')
		byte = bundlefile.read(1)
		bytecounter = 0
		# While we haven't finished looping through the bytes
		while byte != '':
			# Add byte to current packet
			packet_to_send.append(struct.unpack('B', byte)[0])
			# Increment byte counter
			bytecounter = bytecounter + 1
			# Read new byte
			byte = bundlefile.read(1)
			# If packet full, send it
			if bytecounter == 33:
				sendHidPacket(epout, CMD_IMPORT_MEDIA, 33, packet_to_send)
				packet_to_send = array('B')
				bytecounter = 0
				# Check ACK
				if receiveHidPacket(epin)[DATA_INDEX] != 0x01:
					print "Error in upload"
					raw_input("press enter to acknowledge")
		# Send the remaining bytes
		sendHidPacket(epout, CMD_IMPORT_MEDIA, bytecounter, packet_to_send)
		# Wait for ACK
		receiveHidPacket(epin)
		# Inform we sent everything
		sendHidPacket(epout, CMD_IMPORT_MEDIA_END, 0, None)
		# Check ACK
		if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
			success_status = 1
		# Close file
		bundlefile.close()
	else:
		success_status = 0
		print "fail!!!"
		print "likely causes: mooltipass already setup"

	return success_status
	
def checkSecuritySettings(epin, epout):
	correct_password = raw_input("Enter mooltipass password: ")
	correct_key = raw_input("Enter request key: ")
	
	# Mooltipass password to be set
	mooltipass_password = array('B')
	
	print "Getting first random half"
	sendHidPacket(epout, CMD_GET_RANDOM_NUMBER, 0, None)
	data = receiveHidPacketWithTimeout(epin)
	mooltipass_password.extend(data[DATA_INDEX:DATA_INDEX+32])
	print "Getting second random half"
	sendHidPacket(epout, CMD_GET_RANDOM_NUMBER, 0, None)
	data2 = receiveHidPacketWithTimeout(epin)
	mooltipass_password.extend(data2[DATA_INDEX:DATA_INDEX+30])
	print "Getting random number for UID & request key"
	request_key_and_uid = array('B')
	sendHidPacket(epout, CMD_GET_RANDOM_NUMBER, 0, None)
	request_key_and_uid.extend(receiveHidPacketWithTimeout(epin)[DATA_INDEX:DATA_INDEX+22])	
	print "Getting random data..."
	sendHidPacket(epout, CMD_GET_RANDOM_NUMBER, 0, None)
	random_data = receiveHidPacketWithTimeout(epin)[DATA_INDEX:DATA_INDEX+32]
	sendHidPacket(epout, CMD_GET_RANDOM_NUMBER, 0, None)
	mooltipass_password.extend(receiveHidPacketWithTimeout(epin)[DATA_INDEX:DATA_INDEX+30])
	print "Done... starting test"
		
	sendHidPacket(epout, CMD_SET_UID, 22, request_key_and_uid)
	if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
		print "Trying to set another key... success! (this is bad)"
	else:
		print "Trying to set another key... fail! (this is good)"
		
	sendHidPacket(epout, CMD_GET_UID, 16, array('B', correct_key.decode("hex")))
	data = receiveHidPacket(epin)
	if data[LEN_INDEX] == 0x01:
		print "Trying to fetch UID... fail!"
	else:
		print "Trying to fetch UID... success!"
		if data[DATA_INDEX:DATA_INDEX+6] == request_key_and_uid[16:16+6]:
			print "UID fetched is the same as the one sent!"
		else:
			print "UID fetched is different than the one sent!"
	
	sendHidPacket(epout, CMD_GET_UID, 16, random_data[0:16])
	data = receiveHidPacket(epin)
	if data[LEN_INDEX] == 0x01:
		print "Trying to fetch UID with random key... fail! (this is good)"
	else:
		print "Trying to fetch UID with random key... success! (this is bad)"
		
	sendHidPacket(epout, CMD_SET_BOOTLOADER_PWD, 62, mooltipass_password)
	if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
		print "Setting another bootloader password... success! (this is bad)"
	else:
		print "Setting another bootloader password... fail! (this is good)"
	
	sendHidPacket(epout, CMD_JUMP_TO_BOOTLOADER, 62, random_data)
	print "Sending jump to bootloader with random password... did it work?"
	raw_input("Press enter")
	
	sendHidPacket(epout, CMD_JUMP_TO_BOOTLOADER, 62, array('B', correct_password.decode("hex")))
	print "Sending jump to bootloader with good password... did it work?"
	raw_input("Press enter")
	
def unlockMooltipass():
	password = raw_input("Enter password: ")
	print array('B', password.decode("hex"))
	sendHidPacket(epout, CMD_JUMP_TO_BOOTLOADER, 62, array('B', password.decode("hex")))
	print "Sending jump to bootloader with good password... did it work?"
	
def decryptprodfile():
	file_name = raw_input("Enter file name: ")
	
	# Read key
	if not os.path.isfile("key.bin"):
		print "key error!!!"
		return
	key = RSA.importKey(pickle_read("key.bin"))
	data = pickle_read(file_name)
	print key.decrypt(data)

def mooltipassInit(hid_device, intf, epin, epout):
	# Ask for Mooltipass ID
	try :
		mp_id = int(raw_input("Enter Mooltipass ID: "))
		print ""
	except ValueError :
		mp_id = 0
		print ""
	
	# Read public key
	if not os.path.isfile("publickey.bin"):
		print "public key error!!!"
		return
	public_key = RSA.importKey(pickle_read("publickey.bin"))

	# Loop
	try:
		temp_bool = 0
		while temp_bool == 0:
			# Operation success state
			success_status = 1

			# Empty set password packet
			mooltipass_password = array('B')

			# We need 62 random bytes to set them as a password for the Mooltipass
			sys.stdout.write('Step 1... ')
			sys.stdout.flush()
			#print "Getting first random half"
			sendHidPacket(epout, CMD_GET_RANDOM_NUMBER, 0, None)
			data = receiveHidPacketWithTimeout(epin)
			mooltipass_password.extend(data[DATA_INDEX:DATA_INDEX+32])
			#print "Getting second random half"
			sendHidPacket(epout, CMD_GET_RANDOM_NUMBER, 0, None)
			data2 = receiveHidPacketWithTimeout(epin)
			mooltipass_password.extend(data2[DATA_INDEX:DATA_INDEX+30])
			#print "Getting random number for UID & request key"
			request_key_and_uid = array('B')
			sendHidPacket(epout, CMD_GET_RANDOM_NUMBER, 0, None)
			request_key_and_uid.extend(receiveHidPacketWithTimeout(epin)[DATA_INDEX:DATA_INDEX+22])

			# Check that we actually received data
			if data == None or data2 == None:
				success_status = 0
				print "fail!!!"
				print "likely causes: defective crystal or power supply"

			# Send our bundle
			if success_status == 1:
				sys.stdout.write('Step 2... ')
				sys.stdout.flush()
				sendHidPacket(epout, CMD_IMPORT_MEDIA_START, 62, mooltipass_password)
				# Check that the import command worked
				if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
					# Open bundle file
					bundlefile = open('bundle_tutorial.img', 'rb')
					packet_to_send = array('B')
					byte = bundlefile.read(1)
					bytecounter = 0
					# While we haven't finished looping through the bytes
					while byte != '':
						# Add byte to current packet
						packet_to_send.append(struct.unpack('B', byte)[0])
						# Increment byte counter
						bytecounter = bytecounter + 1
						# Read new byte
						byte = bundlefile.read(1)
						# If packet full, send it
						if bytecounter == 33:
							sendHidPacket(epout, CMD_IMPORT_MEDIA, 33, packet_to_send)
							packet_to_send = array('B')
							bytecounter = 0
							# Check ACK
							if receiveHidPacket(epin)[DATA_INDEX] != 0x01:
								print "Error in upload"
								raw_input("press enter to acknowledge")
					# Send the remaining bytes
					sendHidPacket(epout, CMD_IMPORT_MEDIA, bytecounter, packet_to_send)
					# Wait for ACK
					receiveHidPacket(epin)
					# Inform we sent everything
					sendHidPacket(epout, CMD_IMPORT_MEDIA_END, 0, None)
					# Check ACK
					if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
						success_status = 1
					# Close file
					bundlefile.close()
				else:
					success_status = 0
					print "fail!!!"
					print "likely causes: mooltipass already setup"

			# Inform the Mooltipass that the bundle is sent so it can start functional test
			if success_status == 1:
				sys.stdout.write('Step 3... ')
				sys.stdout.flush()
				magic_key = array('B')
				magic_key.append(0)
				magic_key.append(148)
				sendHidPacket(epout, CMD_SET_MOOLTIPASS_PARM, 2, magic_key)
				if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
					success_status = 1
					print ""
					print "Please follow the on screen instructions on the mooltipass"
				else:
					success_status = 0
					print "fail!!!"
					print "likely causes: none"

			# Wait for the mooltipass to inform the script that the test was successfull
			temp_bool2 = False
			sys.stdout.write('Waiting for functional test result...')
			sys.stdout.flush()
			while temp_bool2 != True:
				test_result = receiveHidPacketWithTimeout(epin)
				if test_result == None:
					sys.stdout.write('.')
					sys.stdout.flush()
				else:
					if test_result[CMD_INDEX] == CMD_FUNCTIONAL_TEST_RES and test_result[DATA_INDEX] == 0:
						success_status = 1
						print " ok!"
					else:
						success_status = 0
						print " fail!!!"
						print "Please look at the screen to know the cause"
					temp_bool2 = True
					
			# Send set password packet
			if success_status == 1:
				sys.stdout.write('Step 4... ')
				sys.stdout.flush()
				sendHidPacket(epout, CMD_SET_UID, 22, request_key_and_uid)
				if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
					# Update Success status
					success_status = 1
				else:
					success_status = 0
					print "fail!!!"
					print "likely causes: mooltipass already setup"

			# Send set password packet
			if success_status == 1:
				sys.stdout.write('Step 5... ')
				sys.stdout.flush()
				sendHidPacket(epout, CMD_SET_BOOTLOADER_PWD, 62, mooltipass_password)
				if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
					# Write Mooltipass ID in file together with random bytes, flush write
					string_export = str(mp_id)+"|"+"".join(format(x, "02x") for x in mooltipass_password)+"|"+"".join(format(x, "02x") for x in request_key_and_uid)+"\r\n"
					pickle_write(public_key.encrypt(string_export, 32), time.strftime("export/%Y-%m-%d-%H-%M-%S-Mooltipass-")+str(mp_id)+".txt")
					# Update Success status
					success_status = 1
					print ""
				else:
					success_status = 0
					print "fail!!!"
					print "likely causes: mooltipass already setup"

			# COMMENT THE NEXT LINE FOR PRODUCTION!
			#sendHidPacket(epout, CMD_JUMP_TO_BOOTLOADER, 62, mooltipass_password)

			if success_status == 1:
				# Let the user know it is done
				print "Setting up Mooltipass #"+str(mp_id)+" DONE"
				print "PLEASE WRITE \""+str(mp_id)+"\" ON THE BACK STICKER"
				# Increment Mooltipass ID
				mp_id = mp_id + 1
			else:
				print "---------------------------------------------------------"
				print "---------------------------------------------------------"
				print "Setting up Mooltipass #", mp_id, "FAILED"
				print "PLEASE PUT AWAY THIS MOOLTIPASS!!!!"
				print "---------------------------------------------------------"
				print "---------------------------------------------------------"

			# Disconnect this device
			print "\r\nPlease disconnect this Mooltipass"

			# Wait for no answer to ping
			temp_bool2 = 0
			while temp_bool2 == 0:
				# prepare ping packet
				ping_packet = array('B')
				ping_packet.append(0)
				ping_packet.append(CMD_PING)
				try:
					# try to send ping packet
					epout.write(ping_packet)
					try :
						# try to receive answer
						data = epin.read(epin.wMaxPacketSize, timeout=10000)
					except usb.core.USBError as e:
						#print e
						temp_bool2 = 1
				except usb.core.USBError as e:
					#print e
					temp_bool2 = 1
				time.sleep(.5)

			# Connect another device
			print "Connect other Mooltipass"

			# Wait for findHidDevice to return something
			temp_bool2 = 0;
			while temp_bool2 == 0:
				hid_device, intf, epin, epout = findHIDDevice(USB_VID, USB_PID, False)
				if hid_device is not None:
					temp_bool2 = 1
				time.sleep(.5)

			# Delay
			time.sleep(1)

			# New Mooltipass detected
			print "New Mooltipass detected"
			print ""
	except KeyboardInterrupt:
		print "File written, everything ok"

def setCurrentTimeout(epin, epout):
	packetToSend = array('B')
	packetToSend.append(2)
	choice = input("How many seconds: ")
	print ""
	packetToSend.append(choice)
	sendHidPacket(epout, CMD_SET_MOOLTIPASS_PARM, 2, packetToSend)
	if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
		print "Parameter changed"
	else:
		print "Couldn't change parameter"

def setCurrentKeyboard(epin, epout):
	packetToSend = array('B')
	packetToSend.append(1)
	print "0) EN_EN"
	print "1) FR_FR"
	print "2) ES_ES"
	choice = input("Make your choice: ")
	print ""
	packetToSend.append(128+18+choice)
	sendHidPacket(epout, CMD_SET_MOOLTIPASS_PARM, 2, packetToSend)
	if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
		print "Parameter changed"
	else:
		print "Couldn't change parameter"

def setCurrentTimeoutLockEn(epin, epout):
	packetToSend = array('B')
	packetToSend.append(3)
	choice = input("TRUE (1), FALSE (0): ")
	print ""
	packetToSend.append(choice)
	sendHidPacket(epout, CMD_SET_MOOLTIPASS_PARM, 2, packetToSend)
	if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
		print "Parameter changed"
	else:
		print "Couldn't change parameter"

def setCurrentTimeoutLock(epin, epout):
	packetToSend = array('B')
	packetToSend.append(4)
	choice = input("How many minutes: ")
	print ""
	packetToSend.append(choice)
	sendHidPacket(epout, CMD_SET_MOOLTIPASS_PARM, 2, packetToSend)
	if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
		print "Parameter changed"
	else:
		print "Couldn't change parameter"

def setGenericParameter(epin, epout, choice):
	packetToSend = array('B')
	packetToSend.append(choice)
	intval = int(raw_input("Hex value: "), 16)
	print ""
	packetToSend.append(intval)
	sendHidPacket(epout, CMD_SET_MOOLTIPASS_PARM, 2, packetToSend)
	if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
		print "Parameter changed"
	else:
		print "Couldn't change parameter"


def readCurrentUser(epin, epout):
	sendHidPacket(epout, CMD_READ_CARD_LOGIN, 0, None)
	data = receiveHidPacket(epin)
	if data[LEN_INDEX] == 1:
		print "Card not inserted"
	else:
		print "Current user:", "".join(map(chr, data[DATA_INDEX:])).split(b"\x00")[0]

def readCurrentPass(epin, epout):
	sendHidPacket(epout, CMD_READ_CARD_PASS, 0, None)
	data = receiveHidPacket(epin)
	if data[LEN_INDEX] == 1:
		print "Card not inserted or request not accepted"
	else:
		print "Current password:", "".join(map(chr, data[DATA_INDEX:])).split(b"\x00")[0]

def setCurrentUser(epin, epout):
	user = raw_input("New username: ")
	print "Please accept prompts on the Mooltipass"

	# Check that the context doesn't exist
	sendHidPacket(epout, CMD_SET_CARD_LOGIN, len(user)+1, array('B', user + b"\x00"))
	if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
		print "Username changed"
	else:
		print "Couldn't change username"

def setCurrentPass(epin, epout):
	user = raw_input("New password: ")
	print "Please accept prompts on the Mooltipass"

	# Check that the context doesn't exist
	sendHidPacket(epout, CMD_SET_CARD_PASS, len(user)+1, array('B', user + b"\x00"))
	if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
		print "Password changed"
	else:
		print "Couldn't change password"

def randomBytesGeneration(epin, epout):
	f = open('randombytes.bin', 'wb')

	for i in range(0, 1000000/32) :
		print i*32, "bytes out of 1M\r",
		sendHidPacket(epout, CMD_GET_RANDOM_NUMBER, 0, None)
		data = receiveHidPacket(epin)
		data[DATA_INDEX:DATA_INDEX+32].tofile(f)
		f.flush()

	f.close()

def unlockSmartcard(epin, epout):
	unlockPacket = array('B')
	pincode = raw_input("Enter pin code: ")
	unlockPacket.append(int(pincode, 16)/256)
	unlockPacket.append(int(pincode, 16)%256)
	# send packet, check answer
	sendHidPacket(epout, CMD_RESET_CARD, 2, unlockPacket)
	if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
		print "Smartcard erased"
	else:
		print "Couldn't erase smartcard"

def addServiceAndUser(epin, epout):
	tempPacket = array('B')
	service = raw_input("Service name: ")
	username = raw_input("Username: ")
	password = raw_input("Password: ")
	print "Please accept prompts on the Mooltipass"

	# Check that the context doesn't exist
	sendHidPacket(epout, CMD_CONTEXT, len(service)+1, array('B', service + b"\x00"))
	if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
		print "Service exists"
	else:
		print "Service doesn't exist, adding it"
		# Send the add context packet
		sendHidPacket(epout, CMD_ADD_CONTEXT, len(service)+1, array('B', service + b"\x00"))
		if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
			print "Service added"
		else:
			print "Couldn't add service"
			return

	# Set context
	sendHidPacket(epout, CMD_CONTEXT, len(service)+1, array('B', service + b"\x00"))
	if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
		print "Service set"
	else:
		print "Service couldn't be set"
		return

	# Add user
	sendHidPacket(epout, CMD_SET_LOGIN, len(username)+1, array('B', username + b"\x00"))
	if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
		print "User set"
	else:
		print "User couldn't be set"
		return

	# Change password
	sendHidPacket(epout, CMD_SET_PASSWORD , len(password)+1, array('B', password + b"\x00"))
	if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
		print "Password changed"
	else:
		print "Password couldn't be changed"
		
def getDecodedDataForService(epin, epout):
	tempPacket = array('B')
	service = raw_input("Service name: ")
	print "Please accept prompts on the Mooltipass"

	# Check that the context doesn't exist
	sendHidPacket(epout, CMD_SET_DATA_SERVICE, len(service)+1, array('B', service + b"\x00"))
	if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
		print "Service exists"
	else:
		print "Service doesn't exist"
		return
	
	time1 = time.time()
	sendHidPacket(epout, CMD_READ_32B_IN_DN, 0, None)
	answer = receiveHidPacket(epin)
	time2 = time.time()
	while answer[LEN_INDEX] != 1:
		print answer[DATA_INDEX:DATA_INDEX+32]
		print "Data received, took", (time2 - time1)*1000.0, "ms"
		time1 = time.time()
		sendHidPacket(epout, CMD_READ_32B_IN_DN, 0, None)
		answer = receiveHidPacket(epin)
		time2 = time.time()
		
def addRandomDataForService(epin, epout):
	tempPacket = array('B')
	service = raw_input("Service name: ")
	print "Please accept prompts on the Mooltipass"

	# Check that the context doesn't exist
	sendHidPacket(epout, CMD_SET_DATA_SERVICE, len(service)+1, array('B', service + b"\x00"))
	if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
		print "Service exists"
	else:
		print "Service doesn't exist, adding it"
		# Send the add context packet
		sendHidPacket(epout, CMD_ADD_DATA_SERVICE, len(service)+1, array('B', service + b"\x00"))
		if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
			print "Service added"
		else:
			print "Couldn't add service"
			return
    
	# Set context
	sendHidPacket(epout, CMD_SET_DATA_SERVICE, len(service)+1, array('B', service + b"\x00"))
	if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
		print "Service set"
	else:
		print "Service couldn't be set"
		return

	# Add data
	nb_bytes = input("How many 32 bytes blocks to send: ")
	nb_bytes = nb_bytes * 32
	data = 1
	while nb_bytes != 0:
		data_packet = array('B');
		# Update number of bytes left to be sent
		nb_bytes = nb_bytes - 32;
		# If we sent all the bytes, set the flag
		if nb_bytes == 0:
			data_packet.append(1)
		else:
			data_packet.append(0)
		for i in range(0, 32):
			data_packet.append(data)
		time1 = time.time()
		sendHidPacket(epout, CMD_WRITE_32B_IN_DN, 32 + 1, data_packet)
		data = (data + 1)%256
		if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
			time2 = time.time()
			print "Data sent, took", (time2 - time1)*1000.0, "ms"
		else:
			print "Data couldn't be sent"
			return

def checkPasswordForService(epin, epout):
	tempPacket = array('B')
	service = raw_input("Service name: ")
	username = raw_input("Username: ")
	password = raw_input("Password: ")
	print "Please accept prompts on the Mooltipass"

	# Check that the context doesn't exist
	sendHidPacket(epout, CMD_CONTEXT, len(service)+1, array('B', service + b"\x00"))
	if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
		print "Service exists"
	else:
		print "Service doesn't exist"
		return

	# Add user
	sendHidPacket(epout, CMD_SET_LOGIN, len(username)+1, array('B', username + b"\x00"))
	if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
		print "User set"
	else:
		print "User couldn't be set"
		return

	# Change password
	sendHidPacket(epout, CMD_CHECK_PASSWORD , len(password)+1, array('B', password + b"\x00"))
	if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
		print "Password OK"
	else:
		print "Password NOK"
		
def credGen(epin, epout):
	for i in range(0, 40):
		tempPacket = array('B')
		service = ''.join(random.SystemRandom().choice(string.ascii_lowercase + string.digits) for _ in range(10))
		username = ''.join(random.SystemRandom().choice(string.ascii_lowercase + string.digits) for _ in range(10))
		# Check that the context doesn't exist
		sendHidPacket(epout, CMD_CONTEXT, len(service)+1, array('B', service + b"\x00"))
		if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
			print "Service exists"
		else:
			print "Service doesn't exist, adding it"
			# Send the add context packet
			sendHidPacket(epout, CMD_ADD_CONTEXT, len(service)+1, array('B', service + b"\x00"))
			if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
				print "Service added"
			else:
				print "Couldn't add service"
				continue

		# Set context
		sendHidPacket(epout, CMD_CONTEXT, len(service)+1, array('B', service + b"\x00"))
		if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
			print "Service set"
		else:
			print "Service couldn't be set"
			continue

		# Add user
		sendHidPacket(epout, CMD_SET_LOGIN, len(username)+1, array('B', username + b"\x00"))
		if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
			print "User set"
		else:
			print "User couldn't be set"
			continue
		

def favoritePrint(epin, epout):
	favoriteArg = array('B')
	favoriteArg.append(0)

	# get user profile
	sendHidPacket(epout, CMD_START_MEMORYMGMT, 0, None)
	print "Please accept memory management mode on the MP"
	while receiveHidPacket(epin)[DATA_INDEX] != 1:
		print "Please accept memory management mode on the MP"
		sendHidPacket(epout, CMD_START_MEMORYMGMT, 0, None)
		data = receiveHidPacket(epin)

	# loop through fav slots
	for count in range(0, 14):
		favoriteArg[0] = count
		# request favorite
		sendHidPacket(epout, CMD_GET_FAVORITE, 1, favoriteArg)
		fav_data = receiveHidPacket(epin)
		# check if it is defined
		if fav_data[DATA_INDEX+0] != 0 or fav_data[DATA_INDEX+1] != 0:
			# read parent node
			sendHidPacket(epout, CMD_READ_FLASH_NODE, 2, fav_data[DATA_INDEX+0:DATA_INDEX+2])
			# read it
			data_parent = receiveHidPacket(epin)
			data_parent.extend(receiveHidPacket(epin)[DATA_INDEX:])
			data_parent.extend(receiveHidPacket(epin)[DATA_INDEX:])
			# read child node
			sendHidPacket(epout, CMD_READ_FLASH_NODE, 2, fav_data[DATA_INDEX+2:DATA_INDEX+4])
			# read it
			data_child = receiveHidPacket(epin)
			data_child.extend(receiveHidPacket(epin)[DATA_INDEX:])
			data_child.extend(receiveHidPacket(epin)[DATA_INDEX:])
			# truncate data to get service name
			print "slot", count, "service:", "".join(map(chr, data_parent[DATA_INDEX+SERVICE_INDEX:])).split(b"\x00")[0], "login:", "".join(map(chr, data_child[DATA_INDEX+LOGIN_INDEX:])).split(b"\x00")[0]
		else:
			print "slot", count, "doesn't have a favorite"

	# end memory management mode
	sendHidPacket(epout, CMD_END_MEMORYMGMT, 0, None)
	receiveHidPacket(epin)

def favoriteSelectionScreen(epin, epout):
	found_credential_sets = array('B')
	next_service_addr = array('B')
	next_child_addr = array('B')
	service_number_i = 0

	# get user profile
	sendHidPacket(epout, CMD_START_MEMORYMGMT, 0, None)
	print "Please accept memory management mode on the MP"
	while receiveHidPacket(epin)[DATA_INDEX] != 1:
		print "Please accept memory management mode on the MP"
		sendHidPacket(epout, CMD_START_MEMORYMGMT, 0, None)

	# get starting node
	sendHidPacket(epout, CMD_GET_STARTING_PARENT, 0, None)
	data = receiveHidPacket(epin)

	# print "starting node is address is", data[DATA_INDEX] + data[DATA_INDEX+1]*256
	next_service_addr.append(data[DATA_INDEX])
	next_service_addr.append(data[DATA_INDEX+1])
	next_child_addr.append(data[DATA_INDEX]);
	next_child_addr.append(data[DATA_INDEX]);

	# start printing credentials, loop until next service address is equal to node_addr_null
	while next_service_addr[0] != 0 or next_service_addr[1] != 0:
		# print empty line
		print ""
		# request parent node
		sendHidPacket(epout, CMD_READ_FLASH_NODE, 2, next_service_addr)
		# read it
		data_parent = receiveHidPacket(epin)
		data_parent.extend(receiveHidPacket(epin)[DATA_INDEX:])
		data_parent.extend(receiveHidPacket(epin)[DATA_INDEX:])
		# extract next child address
		next_child_addr[0] = data_parent[DATA_INDEX+NEXT_CHILD_INDEX]
		next_child_addr[1] = data_parent[DATA_INDEX+NEXT_CHILD_INDEX+1]
		# truncate data to get service name
		print "service found:", "".join(map(chr, data_parent[DATA_INDEX+SERVICE_INDEX:])).split(b"\x00")[0]
		# loop in the child nodes
		while next_child_addr[0] != 0 or next_child_addr[1] != 0:
			# store parent and child addresses for future tagging
			found_credential_sets.extend(next_service_addr)
			found_credential_sets.extend(next_child_addr)
			# request child node
			sendHidPacket(epout, CMD_READ_FLASH_NODE, 2, next_child_addr)
			# read it
			data_child = receiveHidPacket(epin)
			data_child.extend(receiveHidPacket(epin)[DATA_INDEX:])
			data_child.extend(receiveHidPacket(epin)[DATA_INDEX:])
			# extract next child address
			next_child_addr[0] = data_child[DATA_INDEX+NEXT_ADDRESS_INDEX]
			next_child_addr[1] = data_child[DATA_INDEX+NEXT_ADDRESS_INDEX+1]
			# truncate data to get login
			print service_number_i, "- login:", "".join(map(chr, data_child[DATA_INDEX+LOGIN_INDEX:])).split(b"\x00")[0], ", description:", "".join(map(chr, data_child[DATA_INDEX+DESC_INDEX:])).split(b"\x00")[0]
			service_number_i += 1
		# extract next parent address (see gNode def)
		next_service_addr[0] = data_parent[DATA_INDEX+NEXT_ADDRESS_INDEX]
		next_service_addr[1] = data_parent[DATA_INDEX+NEXT_ADDRESS_INDEX+1]

	# ask the user to pick a favorite
	print ""
	user_fav = 1000
	while user_fav >= service_number_i:
		user_fav = input("Choose your favorite: ")

	fav_slot_id = 1000
	while fav_slot_id > 14:
		fav_slot_id = input("Choose your slotid: ")

	# prepare packet to send
	favorite_set_packet = array('B')
	favorite_set_packet.append(fav_slot_id)
	favorite_set_packet.extend(found_credential_sets[user_fav*4:user_fav*4+4])

	# send packet, check answer
	sendHidPacket(epout, CMD_SET_FAVORITE, 5, favorite_set_packet)
	if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
		print "Favorite added on the Mooltipass"
	else:
		print "Couldn't add favorite on Mooltipass"

	# end memory management mode
	sendHidPacket(epout, CMD_END_MEMORYMGMT, 0, None)
	receiveHidPacket(epin)
	
def exportUser(epin, epout):
	parent_nodes_addr_export = list()
	parent_nodes_export = list()
	child_nodes_addr_export = list()
	child_nodes_export = list()
	cpz_ctr_export = list()
	favorites_export = list()
	next_service_addr = array('B')
	next_child_addr = array('B')

	# start memory management
	sendHidPacket(epout, CMD_START_MEMORYMGMT, 0, None)
	print "Please accept memory management mode on the MP"
	while receiveHidPacket(epin)[DATA_INDEX] != 1:
		print "Please accept memory management mode on the MP"
		sendHidPacket(epout, CMD_START_MEMORYMGMT, 0, None)
		
	# get favorites
	favoriteArg = array('B')
	favoriteArg.append(0)
	# loop through fav slots
	for count in range(0, 14):
		favoriteArg[0] = count
		# request favorite
		sendHidPacket(epout, CMD_GET_FAVORITE, 1, favoriteArg)
		favorites_export.append(receiveHidPacket(epin)[DATA_INDEX:DATA_INDEX+4])
	
	# write the favorites
	pickle_write(favorites_export, "favorites.txt")

	# get starting node
	sendHidPacket(epout, CMD_GET_STARTING_PARENT, 0, None)
	data = receiveHidPacket(epin)

	# print starting node
	print "Starting node address is at", format(data[DATA_INDEX] + data[DATA_INDEX+1]*256, '#04X')
	next_service_addr.append(data[DATA_INDEX])
	next_service_addr.append(data[DATA_INDEX+1])
	next_child_addr.append(data[DATA_INDEX]);
	next_child_addr.append(data[DATA_INDEX]);
	
	# write the starting node
	pickle_write(next_service_addr, "starting_node.txt")

	# start printing credentials, loop until next service address is equal to node_addr_null
	while next_service_addr[0] != 0 or next_service_addr[1] != 0:
		# request parent node
		sendHidPacket(epout, CMD_READ_FLASH_NODE, 2, next_service_addr)
		# read it and keep the node part
		data_parent = receiveHidPacket(epin)
		data_parent.extend(receiveHidPacket(epin))
		data_parent.extend(receiveHidPacket(epin))
		data_parent = data_parent[DATA_INDEX:DATA_INDEX+NODE_SIZE]
		# store node data together with its address
		print "Found parent node at", format(next_service_addr[0] + next_service_addr[1]*256, '#04X'), "- service name:", "".join(map(chr, data_parent[SERVICE_INDEX:])).split(b"\x00")[0]
		parent_nodes_addr_export.append(next_service_addr)
		parent_nodes_export.append(data_parent)
		# extract next child address
		next_child_addr[0] = data_parent[NEXT_CHILD_INDEX]
		next_child_addr[1] = data_parent[NEXT_CHILD_INDEX+1]
		# loop in the child nodes
		while next_child_addr[0] != 0 or next_child_addr[1] != 0:
			# request child node
			sendHidPacket(epout, CMD_READ_FLASH_NODE, 2, next_child_addr)
			# read it
			data_child = receiveHidPacket(epin)
			data_child.extend(receiveHidPacket(epin))
			data_child.extend(receiveHidPacket(epin))
			data_child = data_child[DATA_INDEX:DATA_INDEX+NODE_SIZE]
			# truncate data to get login
			print "Found child node at", format(next_child_addr[0] + next_child_addr[1]*256, '#04X'), "- login:", "".join(map(chr, data_child[LOGIN_INDEX:])).split(b"\x00")[0]
			child_nodes_addr_export.append(next_child_addr)
			child_nodes_export.append(data_child)
			# extract next child address
			next_child_addr = array('B')
			next_child_addr.append(data_child[NEXT_ADDRESS_INDEX])
			next_child_addr.append(data_child[NEXT_ADDRESS_INDEX+1])
		# extract next parent address (see gNode def)
		next_service_addr = array('B')
		next_service_addr.append(data_parent[NEXT_ADDRESS_INDEX])
		next_service_addr.append(data_parent[NEXT_ADDRESS_INDEX+1])

	# write the export
	pickle_write(parent_nodes_addr_export, "parent_nodes_addr.txt")
	pickle_write(parent_nodes_export, "parent_nodes.txt")
	pickle_write(child_nodes_addr_export, "child_nodes_addr.txt")
	pickle_write(child_nodes_export, "child_nodes.txt")
	
	# get the CPZ & CTR LUT entries
	sendHidPacket(epout, CMD_GET_CARD_CPZ_CTR, 0, None)
	temp_bool = True
	while temp_bool == True:
		received_data = receiveHidPacket(epin)
		# check if we received end of export packet
		if received_data[CMD_INDEX] == CMD_GET_CARD_CPZ_CTR:
			temp_bool = False
		else:
			cpz_ctr_export.append(received_data[DATA_INDEX:DATA_INDEX + received_data[LEN_INDEX]])
		
	# write the export
	pickle_write(cpz_ctr_export, "cpz_ctr_export.txt")
	
	# get the user CTR value
	sendHidPacket(epout, CMD_GET_CTRVALUE, 0, None)
	ctr_packet = receiveHidPacket(epin)
	pickle_write(ctr_packet[DATA_INDEX:DATA_INDEX + ctr_packet[LEN_INDEX]], "ctr_value.txt")
	
	# end memory management mode
	sendHidPacket(epout, CMD_END_MEMORYMGMT, 0, None)
	receiveHidPacket(epin)
	
def importUser(epin, epout):
	#print "parent nodes:"
	#print pickle_read("parent_nodes.txt")
	#print "child nodes:"
	#print pickle_read("child_nodes.txt")
	#print "parent addr:"
	#print pickle_read("parent_nodes_addr.txt")
	#print "child addr:"
	#print pickle_read("child_nodes_addr.txt")
	#print "cpz ctr:"
	#print pickle_read("cpz_ctr_export.txt")
	#print "ctr:"
	#print pickle_read("ctr_value.txt")
	#print "starting:"
	#print pickle_read("starting_node.txt")
	
	# Check Mootipass status
	sendHidPacket(epout, CMD_MOOLTIPASS_STATUS, 0, None)
	if receiveHidPacket(epin)[DATA_INDEX] == 9:
		# Unknown card inserted
		print "Unknown card inserted"
	else:
		print "Unsupported mode"

def recoveryProc(epin, epout):
	found_credential_sets = array('B')
	next_node_addr = array('B')
	data_service_addresses = list()
	data_service_names = list()
	data_service_nodes = list()
	service_addresses = list()
	service_names = list()
	service_nodes = list()
	login_addresses = list()
	login_names = list()
	login_nodes = list()
	pointed_logins = list()
	data_nodes = list()
	pointed_data_nodes = list()
	data_node_addresses = list()
	favorite_parent_addresses = list()
	favorite_child_addresses = list()

	# get user profile
	sendHidPacket(epout, CMD_START_MEMORYMGMT, 0, None)
	print "Please accept memory management mode on the MP"
	while receiveHidPacket(epin)[DATA_INDEX] != 1:
		print "Please accept memory management mode on the MP"
		sendHidPacket(epout, CMD_START_MEMORYMGMT, 0, None)
		
	# find mooltipass version
	sendHidPacket(epout, CMD_VERSION, 0, None)
	data = receiveHidPacket(epin)
	
	# print Mooltipass version, compute number of available pages and nodes per page
	print "Mooltipass has " + str(data[DATA_INDEX]) + "Mb of data"
	if data[DATA_INDEX] >= 16:
		number_of_pages = 256 * data[DATA_INDEX]
	else:
		number_of_pages = 512 * data[DATA_INDEX]
	if data[DATA_INDEX] >= 16:
		nodes_per_page = 4
	else:
		nodes_per_page = 2

	# get starting node
	sendHidPacket(epout, CMD_GET_STARTING_PARENT, 0, None)
	starting_node_addr = receiveHidPacket(epin)[DATA_INDEX:DATA_INDEX+2]

	# print starting node
	print "Starting node address is at", format(starting_node_addr[0] + starting_node_addr[1]*256, '#04X')
	
	# get data starting node
	sendHidPacket(epout, CMD_GET_DN_START_PARENT, 0, None)
	data_starting_node_addr = receiveHidPacket(epin)[DATA_INDEX:DATA_INDEX+2]

	# print data starting node
	print "Starting data node address is at", format(data_starting_node_addr[0] + data_starting_node_addr[1]*256, '#04X')
	
	# get favorites
	favoriteArg = array('B')
	favoriteArg.append(0)
	# loop through fav slots
	for count in range(0, 14):
		favoriteArg[0] = count
		# request favorite
		sendHidPacket(epout, CMD_GET_FAVORITE, 1, favoriteArg)
		fav_data = receiveHidPacket(epin)[DATA_INDEX:DATA_INDEX+4]
		favorite_parent_addresses.append(fav_data[0] + fav_data[1]*256)
		favorite_child_addresses.append(fav_data[2] + fav_data[3]*256)
	
	# start looping through the slots
	completion_percentage = 0
	#for pagei in range(128, number_of_pages):
	for pagei in range(128, 400):
		if int(float(float(pagei-128) / (float(number_of_pages) - 128)) * 100) != completion_percentage:
			completion_percentage = int(float(float(pagei) / (float(number_of_pages) - 128)) * 100)
			print "Scanning: " + str(completion_percentage) + "%, address", format(next_node_addr[0] + next_node_addr[1]*256, '#04X')
		for nodei in range(0, nodes_per_page):
			# request node
			next_node_addr = array('B')
			next_node_addr.append((nodei + (pagei << 3)) & 0x00FF)
			next_node_addr.append((pagei >> 5) & 0x00FF)
			#print "Scanning", format(next_node_addr[0] + next_node_addr[1]*256, '#04X')
			sendHidPacket(epout, CMD_READ_FLASH_NODE, 2, next_node_addr)
			# see if we are allowed
			node_data = receiveHidPacket(epin)
			if node_data[LEN_INDEX] > 1:
				# receive the two other packets
				node_data.extend(receiveHidPacket(epin)[DATA_INDEX:])
				node_data.extend(receiveHidPacket(epin)[DATA_INDEX:])
				if node_data[DATA_INDEX+1] & 0x20 == 0x00:
					if node_data[DATA_INDEX+1] & 0xC0 == 0x00:
						# if we found a parent node, store it along its address and service name
						print "Found parent node at", format(next_node_addr[0] + next_node_addr[1]*256, '#04X'), "- service name:", "".join(map(chr, node_data[DATA_INDEX+SERVICE_INDEX:])).split(b"\x00")[0]
						service_names.append("".join(map(chr, node_data[DATA_INDEX+SERVICE_INDEX:])).split(b"\x00")[0])
						service_addresses.append(next_node_addr[0] + next_node_addr[1]*256)
						service_nodes.append(node_data[DATA_INDEX:])
					elif node_data[DATA_INDEX+1] & 0xC0 == 0x40:
						# if we found a child node, store it along its address and login name
						print "Found child node at", format(next_node_addr[0] + next_node_addr[1]*256, '#04X'), "- login:", "".join(map(chr, node_data[DATA_INDEX+LOGIN_INDEX:])).split(b"\x00")[0], "- ctr:", format(node_data[DATA_INDEX+34], '#02X'), format(node_data[DATA_INDEX+35], '#02X'), format(node_data[DATA_INDEX+36], '#02X')
						login_names.append("".join(map(chr, node_data[DATA_INDEX+LOGIN_INDEX:])).split(b"\x00")[0])
						login_addresses.append(next_node_addr[0] + next_node_addr[1]*256)
						pointed_logins.append(next_node_addr[0] + next_node_addr[1]*256)
						login_nodes.append(node_data[DATA_INDEX:])
					elif node_data[DATA_INDEX+1] & 0xC0 == 0x80:
						# if we found a parent node, store it along its address and service name
						print "Found parent data node at", format(next_node_addr[0] + next_node_addr[1]*256, '#04X'), "- service name:", "".join(map(chr, node_data[DATA_INDEX+SERVICE_INDEX:])).split(b"\x00")[0], "- ctr:", format(node_data[DATA_INDEX+129], '#02X'), format(node_data[DATA_INDEX+130], '#02X'), format(node_data[DATA_INDEX+131], '#02X')
						data_service_names.append("".join(map(chr, node_data[DATA_INDEX+SERVICE_INDEX:])).split(b"\x00")[0])
						data_service_addresses.append(next_node_addr[0] + next_node_addr[1]*256)
						data_service_nodes.append(node_data[DATA_INDEX:])
						#raw_input("confirm")
					elif node_data[DATA_INDEX+1] & 0xC0 == 0xC0:
						# if we found a child node, store it along its address
						print "Found data node at", format(next_node_addr[0] + next_node_addr[1]*256, '#04X')
						data_node_addresses.append(next_node_addr[0] + next_node_addr[1]*256)
						pointed_data_nodes.append(next_node_addr[0] + next_node_addr[1]*256)
						data_nodes.append(node_data[DATA_INDEX:])
						
	#raw_input("Scan finish, press enter to start checks")

	# sort service list together with addresses list
	if len(service_addresses) > 0:
		service_names, service_addresses, service_nodes = (list(t) for t in zip(*sorted(zip(service_names, service_addresses, service_nodes))))
	if len(data_service_addresses) > 0:
		data_service_names, data_service_addresses, data_service_nodes = (list(t) for t in zip(*sorted(zip(data_service_names, data_service_addresses, data_service_nodes))))
	
	# Check correct parent
	if len(service_addresses) > 0:
		correct_starting_parent = array('B')
		correct_starting_parent.append(service_addresses[0] & 0x00FF)
		correct_starting_parent.append((service_addresses[0] >> 8) & 0x00FF)
		if starting_node_addr != correct_starting_parent:
			print starting_node_addr
			print correct_starting_parent
			print "Current starting node is", format(starting_node_addr[0] + starting_node_addr[1]*256, '#04X'), "should be", format(correct_starting_parent[0] + correct_starting_parent[1]*256, '#04X')
			raw_input("Press enter to correct")
			print "Starting parent set to", format(service_addresses[0], '#04X')
			sendHidPacket(epout, CMD_SET_STARTING_PARENT, 2, correct_starting_parent)
			receiveHidPacket(epin)
	else:
		correct_starting_parent = array('B')
		correct_starting_parent.append(0)
		correct_starting_parent.append(0)
		sendHidPacket(epout, CMD_SET_STARTING_PARENT, 2, correct_starting_parent)
		receiveHidPacket(epin)		
	
	# Check correct parent
	if len(data_service_addresses) > 0:
		correct_starting_parent = array('B')
		correct_starting_parent.append(data_service_addresses[0] & 0x00FF)
		correct_starting_parent.append((data_service_addresses[0] >> 8) & 0x00FF)
		if data_starting_node_addr != correct_starting_parent:
			print "Current data starting node is", format(data_starting_node_addr[0] + data_starting_node_addr[1]*256, '#04X'), "should be", format(correct_starting_parent[0] + correct_starting_parent[1]*256, '#04X')
			raw_input("Press enter to correct")
			print "Starting parent set to", format(data_service_addresses[0], '#04X')
			sendHidPacket(epout, CMD_SET_DN_START_PARENT, 2, correct_starting_parent)
			receiveHidPacket(epin)
	else:
		correct_starting_parent = array('B')
		correct_starting_parent.append(0)
		correct_starting_parent.append(0)
		sendHidPacket(epout, CMD_SET_DN_START_PARENT, 2, correct_starting_parent)
		receiveHidPacket(epin)		
	
	# check parent addresses validity
	for i in range(len(service_nodes)):
		next_child_addr = service_nodes[i][6] + (service_nodes[i][7] * 256)
		next_node_addr = service_nodes[i][4] + (service_nodes[i][5] * 256)
		prev_node_addr = service_nodes[i][2] + (service_nodes[i][3] * 256)
		# if next child address different than NODE_ADDR_NULL
		if next_child_addr != 0:
			# if we can find a child whose address corresponds...
			if next_child_addr in login_addresses:
				print "Checked first child for parent", format(service_addresses[i], '#04X'), "at address", format(next_child_addr, '#04X')
				pointed_logins.remove(next_child_addr)
			else:
				print "Wrong first child address for parent", format(service_addresses[i], '#04X'), "at address", format(next_child_addr, '#04X')
				raw_input("confirm")
		# Compute the normal prev and next in the linked list
		if i == 0:
			normal_prev_node = 0
		else:
			normal_prev_node = service_addresses[i-1]
		if i == len(service_nodes) - 1:
			normal_next_node = 0
		else:
			normal_next_node = service_addresses[i+1]
		# Check the prev and next nodes
		if next_node_addr == normal_next_node:
			print "Checked next node for parent", format(service_addresses[i], '#04X'), "at address", format(next_node_addr, '#04X')
		else:
			print "Wrong next node address for parent", format(service_addresses[i], '#04X'), "at address", format(next_node_addr, '#04X'), "should be", format(normal_next_node, '#04X')
			raw_input("confirm")
		if prev_node_addr == normal_prev_node:
			print "Checked prev node for parent", format(service_addresses[i], '#04X'), "at address", format(next_node_addr, '#04X')
		else:
			print "Wrong prev node address for parent", format(service_addresses[i], '#04X'), "at address", format(next_node_addr, '#04X'), "should be", format(normal_prev_node, '#04X')
			raw_input("confirm")	
	
	# Same thing for data parents
	for i in range(len(data_service_nodes)):
		next_child_addr = data_service_nodes[i][6] + (data_service_nodes[i][7] * 256)
		next_node_addr = data_service_nodes[i][4] + (data_service_nodes[i][5] * 256)
		prev_node_addr = data_service_nodes[i][2] + (data_service_nodes[i][3] * 256)
		# if next child address different than NODE_ADDR_NULL
		if next_child_addr != 0:
			# if we can find a child whose address corresponds...
			if next_child_addr in data_node_addresses:
				print "Checked first data child for parent", format(data_service_addresses[i], '#04X'), "at address", format(next_child_addr, '#04X')
				pointed_data_nodes.remove(next_child_addr)
			else:
				print "Wrong first data child address for parent", format(data_service_addresses[i], '#04X'), "at address", format(next_child_addr, '#04X')
				raw_input("confirm")
		# Compute the normal prev and next in the linked list
		if i == 0:
			normal_prev_node = 0
		else:
			normal_prev_node = data_service_addresses[i-1]
		if i == len(data_service_nodes) - 1:
			normal_next_node = 0
		else:
			normal_next_node = data_service_addresses[i+1]
		# Check the prev and next nodes
		if next_node_addr == normal_next_node:
			print "Checked next node for data parent", format(data_service_addresses[i], '#04X'), "at address", format(next_node_addr, '#04X')
		else:
			print "Wrong next node address for data parent", format(data_service_addresses[i], '#04X'), "at address", format(next_node_addr, '#04X'), "should be", format(normal_next_node, '#04X')
			raw_input("confirm")
		if prev_node_addr == normal_prev_node:
			print "Checked prev node for data parent", format(data_service_addresses[i], '#04X'), "at address", format(next_node_addr, '#04X')
		else:
			print "Wrong prev node address for data parent", format(data_service_addresses[i], '#04X'), "at address", format(next_node_addr, '#04X'), "should be", format(normal_prev_node, '#04X')
			raw_input("confirm")	

	# check child addresses validity
	for i in range(len(login_nodes)):
		next_node_addr = login_nodes[i][4] + (login_nodes[i][5] * 256)
		prev_node_addr = login_nodes[i][2] + (login_nodes[i][3] * 256)
		# if next child address different than NODE_ADDR_NULL
		if next_node_addr != 0:
			# if we can find a child whose address corresponds...
			if next_node_addr in login_addresses:
				print "Checked next node for child", format(login_addresses[i], '#04X'), "at address", format(next_node_addr, '#04X')
				try:
					pointed_logins.remove(next_node_addr)
				except:
					pass
			else:
				print "Wrong next node for child", format(login_addresses[i], '#04X'), "at address", format(next_node_addr, '#04X')
				raw_input("confirm")
		# if prev child address different than NODE_ADDR_NULL
		if prev_node_addr != 0:
			# if we can find a child whose address corresponds...
			if prev_node_addr in login_addresses:
				print "Checked prev node for child", format(login_addresses[i], '#04X'), "at address", format(prev_node_addr, '#04X')
				try:
					pointed_logins.remove(prev_node_addr)
				except:
					pass
			else:
				print "Wrong prev node for child", format(login_addresses[i], '#04X'), "at address", format(prev_node_addr, '#04X')
				raw_input("confirm")
				
	# Same thing for data nodes
	for i in range(len(data_nodes)):
		next_node_addr = data_nodes[i][2] + (data_nodes[i][3] * 256)
		# if next data address different than NODE_ADDR_NULL
		if next_node_addr != 0:
			# if we can find a child whose address corresponds...
			if next_node_addr in data_node_addresses:
				print "Checked next data node for child", format(data_node_addresses[i], '#04X'), "at address", format(next_node_addr, '#04X')
				try:
					pointed_data_nodes.remove(next_node_addr)
				except:
					pass
			else:
				print "Wrong next data node for child", format(data_node_addresses[i], '#04X'), "at address", format(next_node_addr, '#04X')
				raw_input("confirm")
				
	# find if we have child nodes that are not registered
	if len(pointed_logins) != 0:
		print "There are orphan child nodes!"
		for orphan_child in pointed_logins:
			print "Address:", format(orphan_child, '#04X'), "- login:", login_names[login_addresses.index(orphan_child)]
			choice = raw_input("Do you want to delete it? (yes/no): ")
			if choice == "yes":
				empty_packet = array('B')
				for i in range(62):
					empty_packet.append(255)
				empty_packet[0] = orphan_child & 0x00FF
				empty_packet[1] = (orphan_child >> 8) & 0x00FF
				empty_packet[2] = 0
				sendHidPacket(epout, CMD_WRITE_FLASH_NODE, 62, empty_packet)
				if receiveHidPacket(epin)[DATA_INDEX] != 0x01:
					print "Error in writing"
				empty_packet[2] = 1
				sendHidPacket(epout, CMD_WRITE_FLASH_NODE, 62, empty_packet)
				if receiveHidPacket(epin)[DATA_INDEX] != 0x01:
					print "Error in writing"
				empty_packet[2] = 2
				empty_packet = empty_packet[0:16]
				sendHidPacket(epout, CMD_WRITE_FLASH_NODE, 17, empty_packet)
				if receiveHidPacket(epin)[DATA_INDEX] != 0x01:
					print "Error in writing"
	else:
		print "No orphan nodes"
		
	# same thing for data nodes
	if len(pointed_data_nodes) != 0:
		print "There are orphan child data nodes!"
		for orphan_child in pointed_data_nodes:
			print "Address:", format(orphan_child, '#04X')
			choice = raw_input("Do you want to delete it? (yes/no): ")
			if choice == "yes":
				empty_packet = array('B')
				for i in range(62):
					empty_packet.append(255)
				empty_packet[0] = orphan_child & 0x00FF
				empty_packet[1] = (orphan_child >> 8) & 0x00FF
				empty_packet[2] = 0
				sendHidPacket(epout, CMD_WRITE_FLASH_NODE, 62, empty_packet)
				if receiveHidPacket(epin)[DATA_INDEX] != 0x01:
					print "Error in writing"
				empty_packet[2] = 1
				sendHidPacket(epout, CMD_WRITE_FLASH_NODE, 62, empty_packet)
				if receiveHidPacket(epin)[DATA_INDEX] != 0x01:
					print "Error in writing"
				empty_packet[2] = 2
				empty_packet = empty_packet[0:16]
				sendHidPacket(epout, CMD_WRITE_FLASH_NODE, 17, empty_packet)
				if receiveHidPacket(epin)[DATA_INDEX] != 0x01:
					print "Error in writing"
	else:
		print "No orphan data nodes"
		
	# check pointed favorites
	for i in range(0, len(favorite_parent_addresses)):
		# if we point to a valid node
		if favorite_parent_addresses[i] != 0:
			# todo: check that the child belongs to the parent
			if favorite_parent_addresses[i] in service_addresses and favorite_child_addresses[i] in login_addresses:
				print "Favorite number", i ,"checked"
			else:
				print "Couldn't verify favorite number", i ,"with parent address", format(favorite_parent_addresses[i], '#04X'), "and child address", format(favorite_child_addresses[i], '#04X')
				raw_input("confirm")
		
	# end memory management mode
	sendHidPacket(epout, CMD_END_MEMORYMGMT, 0, None)
	receiveHidPacket(epin)

def findHIDDevice(vendor_id, product_id, print_debug):
	# Find our device
	hid_device = usb.core.find(idVendor=vendor_id, idProduct=product_id)

	# Was it found?
	if hid_device is None:
		if print_debug:
			print "Device not found"
		return None, None, None, None

	# Device found
	if print_debug:
		print "Mooltipass found"

	# Different init codes depending on the platform
	if platform.system() == "Linux":
		# Need to do things differently
		try:
			hid_device.detach_kernel_driver(0)
			hid_device.reset()
		except Exception, e:
			pass # Probably already detached
	else:
		# Set the active configuration. With no arguments, the first configuration will be the active one
		try:
			hid_device.set_configuration()
		except Exception, e:
			if print_debug:
				print "Cannot set configuration the device:" , str(e)
			return None, None, None, None

	#for cfg in hid_device:
	#	print "configuration val:", str(cfg.bConfigurationValue)
	#	for intf in cfg:
	#		print "int num:", str(intf.bInterfaceNumber), ", int alt:", str(intf.bAlternateSetting)
	#		for ep in intf:
	#			print "endpoint addr:", str(ep.bEndpointAddress)

	# Get an endpoint instance
	cfg = hid_device.get_active_configuration()
	intf = cfg[(0,0)]

	# Match the first OUT endpoint
	epout = usb.util.find_descriptor(intf, custom_match = lambda e: usb.util.endpoint_direction(e.bEndpointAddress) == usb.util.ENDPOINT_OUT)
	if epout is None:
		hid_device.reset()
		return None, None, None, None
	#print "Selected OUT endpoint:", epout.bEndpointAddress

	# Match the first IN endpoint
	epin = usb.util.find_descriptor(intf, custom_match = lambda e: usb.util.endpoint_direction(e.bEndpointAddress) == usb.util.ENDPOINT_IN)
	if epin is None:
		hid_device.reset()
		return None, None, None, None
	#print "Selected IN endpoint:", epin.bEndpointAddress

	# prepare ping packet
	byte1 = random.randint(0, 255)
	byte2 = random.randint(0, 255)
	ping_packet = array('B')
	ping_packet.append(2)
	ping_packet.append(CMD_PING)
	ping_packet.append(byte1)
	ping_packet.append(byte2)

	time.sleep(0.5)
	try:
		# try to send ping packet
		epout.write(ping_packet)
		# try to receive one answer
		temp_bool = 0
		while temp_bool == 0:
			try :
				# try to receive answer
				data = epin.read(epin.wMaxPacketSize, timeout=2000)
				if data[CMD_INDEX] == CMD_PING and data[DATA_INDEX] == byte1 and data[DATA_INDEX+1] == byte2 :
					temp_bool = 1
					if print_debug:
						print "Mooltipass replied to our ping message"
				else:
					if print_debug:
						print "Cleaning remaining input packets"
				time.sleep(.5)
			except usb.core.USBError as e:
				if print_debug:
					print e
				return None, None, None, None
	except usb.core.USBError as e:
		if print_debug:
			print e
		return None, None, None, None

	# Return device & endpoints
	return hid_device, intf, epin, epout

if __name__ == '__main__':
	# Main function
	print ""
	print "Mooltipass USB client"

	# Search for the mooltipass and read hid data
	hid_device, intf, epin, epout = findHIDDevice(USB_VID, USB_PID, True)

	if hid_device is None:
		sys.exit(0)
		
	# Print Mootipass status
	sendHidPacket(epout, CMD_MOOLTIPASS_STATUS, 0, None)
	status_data = receiveHidPacket(epin)
	if status_data[DATA_INDEX] == 0:
		print "No card in Mooltipass"
	elif status_data[DATA_INDEX] == 1:
		print "Mooltipass locked"
	elif status_data[DATA_INDEX] == 3:
		print "Mooltipass locked, unlocking screen"
	elif status_data[DATA_INDEX] == 5:
		print "Mooltipass unlocked"
	elif status_data[DATA_INDEX] == 9:
		print "Unknown smartcard inserted"

	choice = 1
	while choice != 0:
		# print use
		print ""
		print "0) Quit"
		print "1) Add a favorite"
		print "2) See current favorites"
		print "3) Erase unknown smartcard"
		print "4) Store 1M random bytes"
		print "5) Add service and username"
		print "6) Change password for username in service"
		print "7) Read current user name"
		print "8) Read current user password"
		print "9) Change current user name"
		print "10) Change current password"
		print "11) Jump to bootloader"
		print "12) Change Mooltipass keyboard layout"
		print "13) Change Mooltipass interaction timeout"
		print "14) Custom packet"
		print "15) Mooltipass initialization process (ONLY FOR MANUFACTURER)"
		print "16) Keyboard Test"
		print "17) Change Mooltipass timeout bool"
		print "18) Change Mooltipass timeout"
		print "19) Change Mooltipass touch detection integrator"
		print "20) Change Mooltipass touch wheel over sample"
		print "21) Change Mooltipass touch proximity param"
		print "22) Upload Bundle"
		print "23) Recovery program"
		print "24) Credential generator"
		print "25) Set screen saver bool"
		print "26) Export current user"
		print "27) Import user to unknown card"
		print "28) Check password for service & login"
		print "29) Add a random block of data for new service"
		print "30) Get decoded data for given service"
		print "31) Check Mooltipass security settings"
		print "32) Generate RSA 4096 private/public key"
		print "33) Decrypt mooltipass prod file"
		print "34) Unlock mooltipass"
		choice = input("Make your choice: ")
		print ""

		if choice == 1:
			favoriteSelectionScreen(epin, epout)
		elif choice == 2:
			favoritePrint(epin, epout)
		elif choice == 3:
			unlockSmartcard(epin, epout)
		elif choice == 4:
			randomBytesGeneration(epin, epout)
		elif choice == 5:
			addServiceAndUser(epin, epout)
		elif choice == 6:
			addServiceAndUser(epin, epout)
		elif choice == 7:
			readCurrentUser(epin, epout)
		elif choice == 8:
			readCurrentPass(epin, epout)
		elif choice == 9:
			setCurrentUser(epin, epout)
		elif choice == 10:
			setCurrentPass(epin, epout)
		elif choice == 11:
			sendHidPacket(epout, CMD_JUMP_TO_BOOTLOADER, 0, None)
		elif choice == 12:
			setCurrentKeyboard(epin, epout)
		elif choice == 13:
			setCurrentTimeout(epin, epout)
		elif choice == 14:
			sendCustomPacket(epin, epout)
		elif choice == 15:
			mooltipassInit(hid_device, intf, epin, epout)
		elif choice == 16:
			keyboardTest(epout)
		elif choice == 17:
			setCurrentTimeoutLockEn(epin, epout)
		elif choice == 18:
			setCurrentTimeoutLock(epin, epout)
		elif choice == 19:
			setGenericParameter(epin, epout, choice-14)
		elif choice == 20:
			setGenericParameter(epin, epout, choice-14)
		elif choice == 21:
			setGenericParameter(epin, epout, choice-14)
		elif choice == 22:
			uploadBundle(epin, epout)
		elif choice == 23:
			recoveryProc(epin, epout)
		elif choice == 24:
			credGen(epin, epout)
		elif choice == 25:
			setGenericParameter(epin, epout, 9)
		elif choice == 26:
			exportUser(epin, epout)
		elif choice == 27:
			importUser(epin, epout)
		elif choice == 28:
			checkPasswordForService(epin, epout)
		elif choice == 29:
			addRandomDataForService(epin, epout)
		elif choice == 30:
			getDecodedDataForService(epin, epout)
		elif choice == 31:
			checkSecuritySettings(epin, epout)
		elif choice == 32:
			key = RSA.generate(4096)
			pickle_write(key.exportKey('DER'), "key.bin")
			pickle_write(key.publickey().exportKey('DER'), "publickey.bin")
			print "Key generated and exported"
		elif choice == 33:
			decryptprodfile()
		elif choice == 34:
			unlockMooltipass()

	hid_device.reset()

