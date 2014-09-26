from array import array
from time import sleep
import platform
import usb.core
import usb.util
import random
import sys
import os

LEN_INDEX				= 0x00
CMD_INDEX				= 0x01
DATA_INDEX				= 0x02
PREV_ADDRESS_INDEX		= 0x02
NEXT_ADDRESS_INDEX		= 0x04
NEXT_CHILD_INDEX		= 0x06
SERVICE_INDEX			= 0x08
DESC_INDEX				= 6
LOGIN_INDEX				= 37

CMD_DEBUG               = 0x01
CMD_PING                = 0x02
CMD_VERSION             = 0x03
CMD_CONTEXT             = 0x04
CMD_GET_LOGIN           = 0x05
CMD_GET_PASSWORD        = 0x06
CMD_SET_LOGIN           = 0x07
CMD_SET_PASSWORD        = 0x08
CMD_CHECK_PASSWORD      = 0x09
CMD_ADD_CONTEXT         = 0x0A
CMD_EXPORT_FLASH        = 0x30
CMD_EXPORT_FLASH_END    = 0x31
CMD_IMPORT_FLASH_BEGIN  = 0x32
CMD_IMPORT_FLASH        = 0x33
CMD_IMPORT_FLASH_END    = 0x34
CMD_EXPORT_EEPROM       = 0x35
CMD_EXPORT_EEPROM_END   = 0x36
CMD_IMPORT_EEPROM_BEGIN = 0x37
CMD_IMPORT_EEPROM       = 0x38
CMD_IMPORT_EEPROM_END   = 0x39
CMD_ERASE_EEPROM        = 0x40
CMD_ERASE_FLASH         = 0x41
CMD_ERASE_SMC           = 0x42
CMD_DRAW_BITMAP         = 0x43
CMD_SET_FONT            = 0x44
CMD_EXPORT_FLASH_START  = 0x45
CMD_EXPORT_EEPROM_START = 0x46
CMD_SET_BOOTLOADER_PWD  = 0x47
CMD_JUMP_TO_BOOTLOADER  = 0x48
CMD_CLONE_SMARTCARD     = 0x49
CMD_STACK_FREE          = 0x4A
CMD_GET_RANDOM_NUMBER   = 0x4B
CMD_GET_USERPROFILE     = 0x50
CMD_END_MEMORYMGMT      = 0x51
CMD_IMPORT_MEDIA_START  = 0x52
CMD_IMPORT_MEDIA        = 0x53
CMD_IMPORT_MEDIA_END    = 0x54
CMD_READ_FLASH_NODE     = 0x55
CMD_WRITE_FLASH_NODE    = 0x56
CMD_SET_FAVORITE        = 0x57
CMD_SET_STARTINGPARENT  = 0x58
CMD_SET_CTRVALUE        = 0x59
CMD_ADD_CARD_CPZ_CTR    = 0x5A
CMD_GET_CARD_CPZ_CTR    = 0x5B
CMD_CARD_CPZ_CTR_PACKET = 0x5C
CMD_SET_MOOLTIPASS_PARM = 0x5D
CMD_GET_MOOLTIPASS_PARM = 0x5E
CMD_GET_FAVORITE		= 0x5F
CMD_RESET_CARD			= 0x60

		
def receiveHidPacket(epin):
	try : 
		data = epin.read(epin.wMaxPacketSize, timeout=10000)
		return data
	except usb.core.USBError as e:
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
		
	# send data
	epout.write(arraytosend)	
	
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
	
def favoritePrint(epin, epout):
	favoriteArg = array('B')
	favoriteArg.append(0)
	
	# get user profile
	sendHidPacket(epout, CMD_GET_USERPROFILE, 0, None)
	print "Please accept memory management mode on the MP"
	data = receiveHidPacket(epin)
	while data[LEN_INDEX] == 1:
		print "Please accept memory management mode on the MP"
		sendHidPacket(epout, CMD_GET_USERPROFILE, 0, None)
		data = receiveHidPacket(epin)
	
	# receive other packet containing CTR
	receiveHidPacket(epin)
	
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
	sendHidPacket(epout, CMD_GET_USERPROFILE, 0, None)
	print "Please accept memory management mode on the MP"
	data = receiveHidPacket(epin)
	while data[LEN_INDEX] == 1:
		print "Please accept memory management mode on the MP"
		sendHidPacket(epout, CMD_GET_USERPROFILE, 0, None)
		data = receiveHidPacket(epin)
	
	# receive other packet containing CTR
	receiveHidPacket(epin)
	
	# print starting node
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
 
def findHIDDevice(vendor_id, product_id):	
	# find our device
	hid_device = usb.core.find(idVendor=vendor_id, idProduct=product_id)

	# was it found?
	if hid_device is None:
		raise ValueError('Device not found')
	else:
		print "Mooltipass found"

		if platform.system() == "Linux":
			#Need to do things differently
			try:
				hid_device.detach_kernel_driver(0)
				hid_device.reset()
			except Exception, e:
				pass #Probably already detached
		else:
			# set the active configuration. With no arguments, the first configuration will be the active one
			try:
				hid_device.set_configuration()
			except usb.core.USBError as e:
				sys.exit("Cannot set configuration the device: %s" % str(e))

	# get an endpoint instance
	cfg = hid_device.get_active_configuration()
	intf = cfg[(0,0)]

	# match the first OUT endpoint
	epout = usb.util.find_descriptor(intf, custom_match = lambda e: usb.util.endpoint_direction(e.bEndpointAddress) == usb.util.ENDPOINT_OUT)
	assert epout is not None
	
	# match the first IN endpoint
	epin = usb.util.find_descriptor(intf, custom_match = lambda e: usb.util.endpoint_direction(e.bEndpointAddress) == usb.util.ENDPOINT_IN)
	assert epin is not None
	
	# try a ping packet	
	sendHidPacket(epout, CMD_PING, 0, None)
	while receiveHidPacket(epin)[CMD_INDEX] != CMD_PING:
		print "cleaning remaining input packets"
	print "Mooltipass replied to our ping message"
	
	choice = 1
	while choice != 0:
		# print use
		print ""
		print "0) Quit"
		print "1) Add a favorite"
		print "2) See current favorites (only v0.5)"
		print "3) Erase unknown smartcard (only v0.5)"
		choice = input("Make your choice: ")
		
		if choice == 1:
			favoriteSelectionScreen(epin, epout)
		elif choice == 2:
			favoritePrint(epin, epout)
		elif choice == 3:
			unlockSmartcard(epin, epout)
	
	hid_device.reset()

if __name__ == '__main__':
	# Main function
	print ""
	print "Mooltipass USB client" 
	# Search for the mooltipass and read hid data
	findHIDDevice(0x16D0, 0x09A0)

