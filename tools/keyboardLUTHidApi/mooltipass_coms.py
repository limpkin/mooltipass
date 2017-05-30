from array import array
from time import sleep
from keyboard import *
import platform
import os.path
import random
import struct
import string
import copy
import time
import sys
import os

if platform.system() == "Windows":
	from pywinusb import hid
	using_pywinusb = True
else:
	import hid
	using_pywinusb = False

# Buffer containing the received data, filled asynchronously
pywinusb_received_data = None

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
	elif len(string) < 2:
		return ''
	elif string[0] != string[1]:
		return ''
	else:	
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
	Layout_dict = {}

	# No modifier combinations
	for bruteforce in range(KEY_EUROPE_2, KEY_SLASH+1):
		output = keyboardKeyMap(epout, bruteforce)
		if (output == ''): continue
		if output in Layout_dict:
			print "'" + output + "'" + " already stored"
			Layout_dict[output].append(bruteforce)
		else:
			Layout_dict[output] = []
			Layout_dict[output].append(bruteforce)

	# SHIFT combinations
	for bruteforce in range(KEY_EUROPE_2, KEY_SLASH+1):
		output = keyboardKeyMap(epout, SHIFT_MASK|bruteforce)
		if (output == ''): continue
		if output in Layout_dict:
			print "'" + output + "'" + " already stored"
			Layout_dict[output].append(SHIFT_MASK|bruteforce)
		else:
			Layout_dict[output] = []
			Layout_dict[output].append(SHIFT_MASK|bruteforce)

	# ALTGR combinations
	for bruteforce in range(KEY_EUROPE_2, KEY_SLASH+1):
		output = keyboardKeyMap(epout, ALTGR_MASK|bruteforce)
		if (output == ''): continue
		if output in Layout_dict:
			print "'" + output + "'" + " already stored"
			Layout_dict[output].append(ALTGR_MASK|bruteforce)
		else:
			Layout_dict[output] = []
			Layout_dict[output].append(ALTGR_MASK|bruteforce)

	# ALTGR + SHIFT combinations
	for bruteforce in range(KEY_EUROPE_2, KEY_SLASH+1):
		output = keyboardKeyMap(epout, SHIFT_MASK|ALTGR_MASK|bruteforce)
		if (output == ''): continue
		if output in Layout_dict:
			print "'" + output + "'" + " already stored"
			Layout_dict[output].append(SHIFT_MASK|ALTGR_MASK|bruteforce)
		else:
			Layout_dict[output] = []
			Layout_dict[output].append(SHIFT_MASK|ALTGR_MASK|bruteforce)


	hid_define_str = "const uint8_t PROGMEM keyboardLUT_"+fileName+"[95] = \n{\n"
	img_contents = array('B')

	for key in KeyboardAscii:
		if(key not in Layout_dict):
			#print key + " Not found"
			Layout_dict[key] = [0]
		#else:
			#print "BruteForced: " + key
			
		if len(Layout_dict[key]) > 1:
			print "Multiple keys for '" + key + "'"
			i = 0
			for x in Layout_dict[key]:
				if x & (SHIFT_MASK|ALTGR_MASK) == (SHIFT_MASK|ALTGR_MASK):
					print str(i) + ": Shift + Altgr + ", key_val_to_key_text[x & ~SHIFT_MASK & ~ALTGR_MASK]
				elif x & (SHIFT_MASK) == (SHIFT_MASK):
					print str(i) + ": Shift + ", key_val_to_key_text[x & ~SHIFT_MASK & ~ALTGR_MASK]
				elif x & (ALTGR_MASK) == (ALTGR_MASK):
					print str(i) + ": Altgr + ", key_val_to_key_text[x & ~SHIFT_MASK & ~ALTGR_MASK]
				else:
					print str(i) + ": " + key_val_to_key_text[x & ~SHIFT_MASK & ~ALTGR_MASK]
				i += 1
				
			choice = input("Please select correct combination: ")
		else:
			choice = 0

		""" Format C code """
		keycode = hex(Layout_dict[key][choice])+","

		# Write img file
		img_contents.append(Layout_dict[key][choice])

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

def data_handler(data):
	#print("Raw data: {0}".format(data))
	global pywinusb_received_data
	pywinusb_received_data = data[1:]
		
def receiveHidPacket(epin):
	global pywinusb_received_data
	if using_pywinusb:
		while pywinusb_received_data == None:
			time.sleep(0.01)
		data_copy = pywinusb_received_data
		pywinusb_received_data = None
		return data_copy
	else:
		try :
			data = epin.read(64, timeout_ms=15000)
			return data
		except :
			sys.exit("Mooltipass didn't send a packet")

def sendHidPacket(epout, cmd, length, data):
	if using_pywinusb:
		buffer = [0x00]*65
		buffer[0] = 0
		
		# if command copy it otherwise copy the data
		if cmd != 0:
			buffer[1] = length
			buffer[2] = cmd
			buffer[3:3+len(data)] = data[:]
		else:
			buffer[1:1+len(data)] = data[:]
			
		epout.set_raw_data(buffer)
		epout.send()
	else:		
		# data to send
		arraytosend = array('B')

		# if command copy it otherwise copy the data
		if cmd != 0:
			arraytosend.append(length)
			arraytosend.append(cmd)

		# add the data
		if data is not None:
			arraytosend.extend(data)

		#print arraytosend
		#print arraytosend

		# send data
		epout.write(arraytosend)

if __name__ == '__main__':
	# Main function
	print ""
	print "Mooltipass Keyboard LUT Generation Tool"
	
	if using_pywinusb:
		# Look for our device
		filter = hid.HidDeviceFilter(vendor_id = 0x16d0, product_id = 0x09a0)
		hid_device = filter.get_devices()
		
		if len(hid_device) == 0:
			print "Mooltipass device not found"
			sys.exit(0)
			
		# Open device
		print "Mooltipass device found"
		device = hid_device[0]
		device.open()
		device.set_raw_data_handler(data_handler)		
		report = device.find_output_reports()		
		
		# Set data sending object
		data_sending_object = report[0]
		data_receiving_object = None
	else:
		# Look for our device and open it
		try:
			hid_device = hid.device(vendor_id=0x16d0, product_id=0x09a0)
			hid_device.open(vendor_id=0x16d0, product_id=0x09a0)
		except IOError, ex:
			print ex
			sys.exit(0)

		print "Device Found and Opened"
		print "Manufacturer: %s" % hid_device.get_manufacturer_string()
		print "Product: %s" % hid_device.get_product_string()
		print "Serial No: %s" % hid_device.get_serial_number_string()
		print ""
		
		# Set data sending object
		data_sending_object = hid_device
		data_receiving_object = hid_device
		
	sendHidPacket(data_sending_object, CMD_PING, 4, [0,1,2,3])
	if receiveHidPacket(data_receiving_object)[CMD_INDEX] == CMD_PING:
		print "Device responded to our ping"
	else:
		print "Bad answer to ping"
		sys.exit(0)
		
	keyboardTest(data_sending_object)
	
	if not using_pywinusb:
		# Close device
		data_sending_object.close()

