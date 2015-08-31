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
PACKET_EXPORT_SIZE		= 62

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
	
def writeNodeToFlash(epout, pageAddr, nodeAddr, data):
	# Maximum number of bytes per packet we can send
	writeNodePayloadSize = (PACKET_EXPORT_SIZE-3)
	# Packet we will send
	data_packet = array('B')
	data_packet.append((nodeAddr + (pageAddr << 3)) & 0x00FF)
	data_packet.append((pageAddr >> 5) & 0x00FF)
	data_packet.append(0)
	for i in range(0, 3):
		# Keep node address, change packet number
		data_packet = data_packet[0:3]
		data_packet[2] = i
		data_packet.extend(data[i*writeNodePayloadSize:i*writeNodePayloadSize+writeNodePayloadSize])
		sendHidPacket(epout, CMD_WRITE_FLASH_NODE, len(data_packet), data_packet)
		if receiveHidPacket(epin)[DATA_INDEX] != 0x01:
			print "Error in writing"
			raw_input("confirm")	

def getEmptyFlashNode():	
	empty_node = array('B')
	for i in range(NODE_SIZE):
		empty_node.append(255)
	return empty_node

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
	
def eraseDataNodes(epin, epout):
	# Get Mootipass status
	sendHidPacket(epout, CMD_MOOLTIPASS_STATUS, 0, None)
	while receiveHidPacket(epin)[DATA_INDEX] != 5:
		print "Please enter your card and unlock your Mooltipass"
		sendHidPacket(epout, CMD_MOOLTIPASS_STATUS, 0, None)
		sleep(1)
		
	# End memory management mode in case we're still in it
	sendHidPacket(epout, CMD_END_MEMORYMGMT, 0, None)
	receiveHidPacket(epin)

	# Start memory management
	sendHidPacket(epout, CMD_START_MEMORYMGMT, 0, None)
	print "Please accept memory management mode on the MP"
	while receiveHidPacket(epin)[DATA_INDEX] != 1:
		print "Please accept memory management mode on the MP"
		sendHidPacket(epout, CMD_START_MEMORYMGMT, 0, None)
		
	# find Mootipass version
	sendHidPacket(epout, CMD_VERSION, 0, None)
	data = receiveHidPacket(epin)
	nbMBits = data[DATA_INDEX]
	
	# print Mooltipass version, compute number of available pages and nodes per page
	print "Mooltipass has " + repr(nbMBits) + "Mb of Flash"
	if nbMBits == 1:
		storage_start_page = 128
		number_of_pages = 128 * 4
	elif nbMBits == 2:
		storage_start_page = 128
		number_of_pages = 128 * 8
	elif nbMBits == 4:
		storage_start_page = 256
		number_of_pages = 256 * 8
	elif nbMBits == 8:
		storage_start_page = 256
		number_of_pages = 256 * 16
	elif nbMBits == 16:
		storage_start_page = 256
		number_of_pages = 256 * 16
	elif nbMBits == 32:
		storage_start_page = 128
		number_of_pages = 128 * 64

	# How many nodes per page (aka bytes per page)
	if nbMBits >= 16:
		nodes_per_page = 4
	else:
		nodes_per_page = 2
		
	# reset starting parent
	correct_starting_parent = array('B')
	correct_starting_parent.append(0)
	correct_starting_parent.append(0)
	sendHidPacket(epout, CMD_SET_STARTING_PARENT, 2, correct_starting_parent)
	receiveHidPacket(epin)	
	print "Starting parent reset"
	
	# start looping through the slots
	completion_percentage = 0
	for pagei in range(storage_start_page, number_of_pages):
		current_percentage = int(float(float(pagei-storage_start_page) / (float(number_of_pages) - storage_start_page)) * 100)
		if current_percentage != completion_percentage:
			completion_percentage = current_percentage
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
					if node_data[DATA_INDEX+1] & 0xC0 == 0x80:
						print "Found parent data node at", format(next_node_addr[0] + next_node_addr[1]*256, '#04X'), "- service name:", "".join(map(chr, node_data[DATA_INDEX+SERVICE_INDEX:])).split(b"\x00")[0], "- ctr:", format(node_data[DATA_INDEX+129], '#02X'), format(node_data[DATA_INDEX+130], '#02X'), format(node_data[DATA_INDEX+131], '#02X')
						writeNodeToFlash(epout, pagei, nodei, getEmptyFlashNode())
					elif node_data[DATA_INDEX+1] & 0xC0 == 0xC0:
						print "Found data node at", format(next_node_addr[0] + next_node_addr[1]*256, '#04X')
						writeNodeToFlash(epout, pagei, nodei, getEmptyFlashNode())
		
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
		print "1) Erase all data nodes in memory"
		choice = input("Make your choice: ")
		print ""

		if choice == 1:
			eraseDataNodes(epin, epout)

	hid_device.reset()

