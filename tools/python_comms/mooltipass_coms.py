from array import array
from time import sleep
import platform
import usb.core
import usb.util
import random
import time
import sys
import os

USB_VID					= 0x16D0
USB_PID					= 0x09A0

LEN_INDEX				= 0x00
CMD_INDEX				= 0x01
DATA_INDEX				= 0x02
PREV_ADDRESS_INDEX		= 0x02
NEXT_ADDRESS_INDEX		= 0x04
NEXT_CHILD_INDEX		= 0x06
SERVICE_INDEX			= 0x08
DESC_INDEX				= 6
LOGIN_INDEX				= 37

CMD_DEBUG				= 0x01
CMD_PING				= 0x02
CMD_VERSION				= 0x03
CMD_CONTEXT				= 0x04
CMD_GET_LOGIN			= 0x05
CMD_GET_PASSWORD		= 0x06
CMD_SET_LOGIN			= 0x07
CMD_SET_PASSWORD		= 0x08
CMD_CHECK_PASSWORD		= 0x09
CMD_ADD_CONTEXT			= 0x0A
CMD_EXPORT_FLASH		= 0x30
CMD_EXPORT_FLASH_END	= 0x31
CMD_IMPORT_FLASH_BEGIN	= 0x32
CMD_IMPORT_FLASH		= 0x33
CMD_IMPORT_FLASH_END	= 0x34
CMD_EXPORT_EEPROM		= 0x35
CMD_EXPORT_EEPROM_END	= 0x36
CMD_IMPORT_EEPROM_BEGIN = 0x37
CMD_IMPORT_EEPROM		= 0x38
CMD_IMPORT_EEPROM_END	= 0x39
CMD_ERASE_EEPROM		= 0x40
CMD_ERASE_FLASH			= 0x41
CMD_ERASE_SMC			= 0x42
CMD_DRAW_BITMAP			= 0x43
CMD_SET_FONT			= 0x44
CMD_EXPORT_FLASH_START	= 0x45
CMD_EXPORT_EEPROM_START = 0x46
CMD_SET_BOOTLOADER_PWD	= 0x47
CMD_JUMP_TO_BOOTLOADER	= 0x48
CMD_CLONE_SMARTCARD		= 0x49
CMD_STACK_FREE			= 0x4A
CMD_GET_RANDOM_NUMBER	= 0x4B
CMD_START_MEMORYMGMT	= 0x50
CMD_END_MEMORYMGMT		= 0x51
CMD_IMPORT_MEDIA_START	= 0x52
CMD_IMPORT_MEDIA		= 0x53
CMD_IMPORT_MEDIA_END	= 0x54
CMD_READ_FLASH_NODE		= 0x55
CMD_WRITE_FLASH_NODE	= 0x56
CMD_SET_FAVORITE		= 0x57
CMD_SET_STARTINGPARENT	= 0x58
CMD_SET_CTRVALUE		= 0x59
CMD_ADD_CARD_CPZ_CTR	= 0x5A
CMD_GET_CARD_CPZ_CTR	= 0x5B
CMD_CARD_CPZ_CTR_PACKET = 0x5C
CMD_SET_MOOLTIPASS_PARM = 0x5D
CMD_GET_MOOLTIPASS_PARM = 0x5E
CMD_GET_FAVORITE		= 0x5F
CMD_RESET_CARD			= 0x60
CMD_READ_CARD_LOGIN		= 0x61
CMD_READ_CARD_PASS		= 0x62
CMD_SET_CARD_LOGIN		= 0x63
CMD_SET_CARD_PASS		= 0x64
CMD_GET_FREE_SLOT_ADDR	= 0x65
CMD_GET_STARTING_PARENT = 0x66

		
def receiveHidPacket(epin):
	try : 
		data = epin.read(epin.wMaxPacketSize, timeout=5000)
		return data
	except usb.core.USBError as e:
		print e
		if e.errno != 110: # 110 is a timeout.
			sys.exit("Mooltipass didn't send a packet")

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
		
def mooltipassInit(hid_device, intf, epin, epout):
	# Ask for Mooltipass ID
	try :
		mp_id = int(raw_input("Enter Mooltipass ID: "))
		print ""
	except ValueError :
		mp_id = 0
		print ""
	
	# Create text file
	f = open(time.strftime("%Y-%m-%d-%H-%M-%S-Mooltipass IDs.txt"), 'wb')
	
	# Loop
	try:
		temp_bool = 0
		while temp_bool == 0:
			# Set password empty packet
			packet = array('B')
			
			# We need 62 random bytes to set them as a password for the Mooltipass
			print "Getting first random half"
			sendHidPacket(epout, CMD_GET_RANDOM_NUMBER, 0, None)
			data = receiveHidPacket(epin)
			packet.extend(data[DATA_INDEX:DATA_INDEX+32])
			print "Getting second random half"
			sendHidPacket(epout, CMD_GET_RANDOM_NUMBER, 0, None)
			data2 = receiveHidPacket(epin)
			packet.extend(data2[DATA_INDEX:DATA_INDEX+30])
			
			# Send set password packet
			#sendHidPacket(epout, CMD_SET_BOOTLOADER_PWD, 62, packet)	
			#if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
			if True:
				# Write Mooltipass ID in file together with random bytes, flush write
				f.write(str(mp_id))
				f.write('|')
				f.write(''.join(format(x, '02x') for x in data[DATA_INDEX:DATA_INDEX+32]))
				f.write(''.join(format(x, '02x') for x in data2[DATA_INDEX:DATA_INDEX+30]))
				f.write('\r\n')
				f.flush()	
				# Let the user know it is done
				print ""
				print "Setting up Mooltipass #", mp_id, "..... DONE" 
			else:
				print ""
				print "---------------------------------------------------------"
				print "---------------------------------------------------------"
				print "Setting up Mooltipass #", mp_id, "..... FAIL!!!!!!!!!!!!!!!!!!" 
				print "Please put away this Mooltipass!!!!!!!!!"
				print "---------------------------------------------------------"
				print "---------------------------------------------------------"
				
			# Disconnect this device
			print "Please disconnect this Mooltipass"
			
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
						data = epin.read(epin.wMaxPacketSize, timeout=5000)
					except usb.core.USBError as e:
						temp_bool2 = 1
				except usb.core.USBError as e:
					temp_bool2 = 1	
				time.sleep(.5)
			
			try:
				hid_device.reset()
			except Exception, e:
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
					
			# Delay and increment
			time.sleep(2)
			mp_id = mp_id + 1
	except KeyboardInterrupt:
		f.close()
		print "File written, everything ok"

def setCurrentTimeout(epin, epout):
	packetToSend = array('B')
	packetToSend.append(1)
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
	packetToSend.append(0)
	print "0) EN_EN"
	print "1) FR_FR"
	print "2) ES_ES"
	choice = input("Make your choice: ")
	print ""
	packetToSend.append(64+19+choice)
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
	for count in range(0, 15):
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
			data_parent.extend(receiveHidPacket(epin))
			data_parent.extend(receiveHidPacket(epin))
			# read child node
			sendHidPacket(epout, CMD_READ_FLASH_NODE, 2, fav_data[DATA_INDEX+2:DATA_INDEX+4])
			# read it
			data_child = receiveHidPacket(epin)
			data_child.extend(receiveHidPacket(epin))
			data_child.extend(receiveHidPacket(epin))
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
		data_parent.extend(receiveHidPacket(epin))
		data_parent.extend(receiveHidPacket(epin))
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
			data_child.extend(receiveHidPacket(epin))
			data_child.extend(receiveHidPacket(epin))
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
	while fav_slot_id > 15:
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
	ping_packet = array('B')
	ping_packet.append(0)
	ping_packet.append(CMD_PING)
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
				if data[CMD_INDEX] == CMD_PING:
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
	
	choice = 1
	while choice != 0:
		# print use
		print ""
		print "0) Quit"
		print "1) Add a favorite"
		print "2) See current favorites (only v0.5)"
		print "3) Erase unknown smartcard (only v0.5)"
		print "4) Store 1M random bytes (only v0.6)"
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
		print "15) Mooltipass initialization process"
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
			sys.exit(0)
	
	hid_device.reset()

