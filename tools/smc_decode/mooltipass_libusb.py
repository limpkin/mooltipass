from array import array
from time import sleep
import platform
import usb.core
import usb.util
import os.path
import random
import struct
import string
import copy
import time
import sys
import os

USB_VID					= 0x072F
USB_PID					= 0x90CC

def reverse_bit_order_in_byte_buffer(data):
	for i in range(0, len(data)):
		data[i] = int('{:08b}'.format(data[i])[::-1], 2)
	return data
	
def left_shift_byte_buffer_by_xbits(data, nb_bits):
	for i in range(0, len(data)-1):
		data[i] = (data[i] << nb_bits)%256 | (data[i+1] >> (8 - nb_bits))
	return data	
	
def right_shift_byte_buffer_by_xbits(data, nb_bits):
	for i in range(len(data)-2, -1, -1):
		data[i+1] = (data[i+1]>>nb_bits) | ((data[i] << (8 - nb_bits)) & 0x00FF)
	data[0] = data[0] >> nb_bits
	return data	

def receivePacketWithTimeout(epin, timeout_ms, debug):
	try :
		data = epin.read(epin.wMaxPacketSize, timeout=timeout_ms)
		print "Read", ''.join('0x{:02x} '.format(x) for x in data)
		return data
	except usb.core.USBError as e:
		if debug:
			print "Couldn't receive packet:", e
		return None

def checkInterruptChannel(epintin):
	recv = receivePacketWithTimeout(epintin, 100, False)
	if recv != None:
		print "Interrupt packet:", recv

def sendPacket(epout, data):
	# send data
	#for i in range(0, 64-len(data)):
	#	data.append(0)
	#print data
	print "Write", ''.join('0x{:02x} '.format(x) for x in data)
	epout.write(data)
	
def print_card_type(data):
	if data & 0x0001 != 0:
		print "- No card type"
	if data & 0x0002 != 0:
		print "- I2C <= 16kb card type"
	if data & 0x0004 != 0:
		print "- I2C > 16kb card type"
	if data & 0x0008 != 0:
		print "- AT88SC153 card type"
	if data & 0x0010 != 0:
		print "- AT88SC1608 card type"
	if data & 0x0020 != 0:
		print "- SLE4418/28 card type"
	if data & 0x0040 != 0:
		print "- SLE4432/42 card type"
	if data & 0x0080 != 0:
		print "- SLE4406/36 & SLE5536 card type"
	if data & 0x0100 != 0:
		print "- SLE4404 card type"
	if data & 0x0200 != 0:
		print "- AT88SC101/102/1003 card type"
	if data & 0x1000 != 0:
		print "- MCU T=0 card type"
	if data & 0x2000 != 0:
		print "- MCU T=1 card type"
	
def print_selected_card_type(data):
	if data == 0:
		print "No card type selected"
	elif data == 1:
		print "I2C <= 16kb card type selected"
	elif data == 2:
		print "I2C > 16kb card type selected"
	elif data == 3:
		print "AT88SC153 card type selected"
	elif data == 4:
		print "AT88SC1608 card type selected"
	elif data == 5:
		print "SLE4418/28 card type selected"
	elif data == 6:
		print "SLE4432/42 card type selected"
	elif data == 7:
		print "SLE4406/36 & SLE5536 card type selected"
	elif data == 8:
		print "SLE4404 card type selected"
	elif data == 9:
		print "AT88SC101/102/1003 card type selected"
	elif data == 0x0C:
		print "MCU T=0 card type selected"
	elif data == 0x0D:
		print "MCU T=1 card type selected"	
		
def response_analysis(bStatus, bError):
	if bStatus & 0xC0 == 0:
		print "Response OK"
	elif bStatus & 0xC0 == 0x40:
		print "Response problem, bError:", bError
	elif bStatus & 0xC0 == 0x80:
		print "Response problem, time extension is requested"
	else:
		print "Response problem, RFU"
	if bStatus & 0x03 == 0:
		print "ICC present and active"
	elif bStatus & 0x03 == 1:
		print "ICC present and inactive"
	elif bStatus & 0x03 == 2:
		print "No ICC present"

def print_get_reader_info(epout, epin, sequence_number):
	# Get Reader Info
	print ""
	Get_Reader_Info_Packet = array('B')
	Get_Reader_Info_Packet.append(0x6F)  					# bMessageType
	Get_Reader_Info_Packet.append(0x00)  					# dwLength
	Get_Reader_Info_Packet.append(0x00)  					# dwLength
	Get_Reader_Info_Packet.append(0x00)  					# dwLength
	Get_Reader_Info_Packet.append(0x05)  					# dwLength
	Get_Reader_Info_Packet.append(0x00)  					# bSlot
	Get_Reader_Info_Packet.append(sequence_number)  		# bSeq
	Get_Reader_Info_Packet.append(0xFF)  					# bBWI
	Get_Reader_Info_Packet.append(0x00)  					# wLevelParameter
	Get_Reader_Info_Packet.append(0x00)  					# wLevelParameter	
	Get_Reader_Info_Packet.append(0xFF)  					# CLA
	Get_Reader_Info_Packet.append(0x09)  					# INS
	Get_Reader_Info_Packet.append(0x00)  					# P1
	Get_Reader_Info_Packet.append(0x00)  					# P2
	Get_Reader_Info_Packet.append(0x10)  					# Lc
	sendPacket(epout, Get_Reader_Info_Packet)
	sequence_number += 1
	response = receivePacketWithTimeout(epin, 5000, True)
	if response == None or response[0] != 0x80:
		print "Reader Problem!"
	elif len(response) > 10:
		print "Get Reader Info Packet"
		print "Firmware:", response[10:20]
		print "Max number of command data bytes:", response[20]
		print "Max number of data bytes that can be requested to be transmitted in a response:", response[21]
		print "Supported card types :"	
		print_card_type(int(response[22])*256 + int(response[23]))
		print_selected_card_type(response[24])		
		if response[25] == 0:
			print "No card inserted"
		elif response[25] == 1:
			print "Card inserted, not powered up"
		elif response[25] == 3:
			print "Card inserted, powered up"
	return sequence_number + 1
	
def verify_security_code(epout, epin, pin_code, sequence_number):
	# Verify Security code
	Verify_Sec_Code_Packet = array('B')
	Verify_Sec_Code_Packet.append(0x6F)  					# bMessageType
	Verify_Sec_Code_Packet.append(0x00)  					# dwLength
	Verify_Sec_Code_Packet.append(0x00)  					# dwLength
	Verify_Sec_Code_Packet.append(0x00)  					# dwLength
	Verify_Sec_Code_Packet.append(0x07)  					# dwLength
	Verify_Sec_Code_Packet.append(0x00)  					# bSlot
	Verify_Sec_Code_Packet.append(sequence_number)  		# bSeq
	Verify_Sec_Code_Packet.append(0xFF)  					# bBWI
	Verify_Sec_Code_Packet.append(0x00)  					# wLevelParameter
	Verify_Sec_Code_Packet.append(0x00)  					# wLevelParameter	
	Verify_Sec_Code_Packet.append(0xFF)  					# CLA
	Verify_Sec_Code_Packet.append(0x20)  					# INS
	Verify_Sec_Code_Packet.append(0x08)  					# Error Counter LEN
	Verify_Sec_Code_Packet.append(0x0A)  					# Byte Address
	Verify_Sec_Code_Packet.append(0x02)  					# MEM_L
	Verify_Sec_Code_Packet.append(pin_code / 256)  			# Code byte 1
	Verify_Sec_Code_Packet.append(pin_code & 0x00FF) 		# Code byte 2
	#print ''.join('0x{:02x} '.format(x) for x in Verify_Sec_Code_Packet)
	sendPacket(epout, Verify_Sec_Code_Packet)
	sequence_number += 1
	response = receivePacketWithTimeout(epin, 5000, True)
	if response == None or response[0] != 0x80 or len(response) == 10:
		print "Reader Problem!"
		reader_device.reset()
		sys.exit(0)
	else:
		if response[10] == 0x90 and response[11] == 0x00:
			return sequence_number, False
		elif response[10] == 0x63 and response[11] == 0x00:
			return sequence_number, True
			
	return sequence_number, True
	
def read_scac_value(epout, epin, sequence_number):
	# Read SCAC
	Read_Memory_Card_Packet = array('B')
	Read_Memory_Card_Packet.append(0x6F)  					# bMessageType
	Read_Memory_Card_Packet.append(0x00)  					# dwLength
	Read_Memory_Card_Packet.append(0x00)  					# dwLength
	Read_Memory_Card_Packet.append(0x00)  					# dwLength
	Read_Memory_Card_Packet.append(0x05)  					# dwLength
	Read_Memory_Card_Packet.append(0x00)  					# bSlot
	Read_Memory_Card_Packet.append(sequence_number)  		# bSeq
	Read_Memory_Card_Packet.append(0xFF)  					# bBWI
	Read_Memory_Card_Packet.append(0x00)  					# wLevelParameter
	Read_Memory_Card_Packet.append(0x00)  					# wLevelParameter	
	Read_Memory_Card_Packet.append(0xFF)  					# CLA
	Read_Memory_Card_Packet.append(0xB0)  					# INS
	Read_Memory_Card_Packet.append(0x00)  					# P1
	Read_Memory_Card_Packet.append(0x00)  					# Byte Address
	Read_Memory_Card_Packet.append(40)  					# MEM_L
	sendPacket(epout, Read_Memory_Card_Packet)
	sequence_number += 1
	response = receivePacketWithTimeout(epin, 5000, True)
	if response == None or response[0] != 0x80:
		print "Reader Problem!"
		reader_device.reset()
		sys.exit(0)
	else:
		response[10:] = reverse_bit_order_in_byte_buffer(response[10:])
		card_scac = response[22:24]
		number_of_tries_left = bin((card_scac[0] >> 4)&0x0F).count("1")
			
	return sequence_number, number_of_tries_left
	
def read_memory_val(epout, epin, sequence_number, addr, size):
	# Read Memory Card
	Read_Memory_Card_Packet = array('B')
	Read_Memory_Card_Packet.append(0x6F)  					# bMessageType
	Read_Memory_Card_Packet.append(0x00)  					# dwLength
	Read_Memory_Card_Packet.append(0x00)  					# dwLength
	Read_Memory_Card_Packet.append(0x00)  					# dwLength
	Read_Memory_Card_Packet.append(0x05)  					# dwLength
	Read_Memory_Card_Packet.append(0x00)  					# bSlot
	Read_Memory_Card_Packet.append(sequence_number)  		# bSeq
	Read_Memory_Card_Packet.append(0xFF)  					# bBWI
	Read_Memory_Card_Packet.append(0x00)  					# wLevelParameter
	Read_Memory_Card_Packet.append(0x00)  					# wLevelParameter	
	Read_Memory_Card_Packet.append(0xFF)  					# CLA
	Read_Memory_Card_Packet.append(0xB0)  					# INS
	Read_Memory_Card_Packet.append(0x00)  					# P1
	Read_Memory_Card_Packet.append(addr)  					# Byte Address
	Read_Memory_Card_Packet.append(size)  					# MEM_L
	sendPacket(epout, Read_Memory_Card_Packet)
	sequence_number += 1
	response = receivePacketWithTimeout(epin, 5000, True)
	if response == None or response[0] != 0x80:
		print "Reader Problem!"
		reader_device.reset()
		sys.exit(0)
	else:
		response[10:] = reverse_bit_order_in_byte_buffer(response[10:])
		print ''.join('0x{:02x} '.format(x) for x in response[10:])
			
	return sequence_number

def findReaderDevice(vendor_id, product_id, print_debug):
	# Find our device
	reader_device = usb.core.find(idVendor=vendor_id, idProduct=product_id)

	# Was it found?
	if reader_device is None:
		if print_debug:
			print "Device not found"
		return None, None, None, None, None

	# Device found
	print "Reader found"
	#print reader_device
	#_name = usb.util.get_string(reader_device,256,0)	#This is where I'm having trouble
	#print "device name=",_name
	#print "Device:", reader_device.filename
	#print "	 Device class:",reader_device.deviceClass
	#print "	 Device sub class:",dev.deviceSubClass
	#print "	 Device protocol:",dev.deviceProtocol
	#print "	 Max packet size:",dev.maxPacketSize
	#print "	 idVendor:",hex(dev.idVendor)
	#print "	 idProduct:",hex(dev.idProduct)
	#print "	 Device Version:",dev.deviceVersion

	# Different init codes depending on the platform
	if platform.system() == "Linux":
		# Need to do things differently
		try:
			reader_device.detach_kernel_driver(0)
			reader_device.reset()
		except Exception, e:
			pass # Probably already detached
	else:
		# Set the active configuration. With no arguments, the first configuration will be the active one
		try:
			reader_device.set_configuration()
		except Exception, e:
			if print_debug:
				print "Cannot set configuration the device:" , str(e)
			return None, None, None, None, None

	#for cfg in reader_device:
	#	print "configuration val:", str(cfg.bConfigurationValue)
	#	for intf in cfg:
	#		print "int num:", str(intf.bInterfaceNumber), ", int alt:", str(intf.bAlternateSetting)
	#		for ep in intf:				
	#			print ep
	#			if usb.util.endpoint_direction(ep.bEndpointAddress) == usb.util.ENDPOINT_OUT:
	#				print "OUT endpoint addr:", str(ep.bEndpointAddress)
	#			else:
	#				print "IN endpoint addr:", str(ep.bEndpointAddress)

	# Get an endpoint instance
	cfg = reader_device.get_active_configuration()
	intf = cfg[(0,0)]

	# Match the first OUT endpoint
	epout = usb.util.find_descriptor(intf, custom_match = lambda e: usb.util.endpoint_direction(e.bEndpointAddress) == usb.util.ENDPOINT_OUT and e.bmAttributes == 0x02)
	if epout is None:
		reader_device.reset()
		return None, None, None, None, None
	#print "Selected OUT endpoint:", epout.bEndpointAddress

	# Match the first IN endpoint
	epin = usb.util.find_descriptor(intf, custom_match = lambda e: usb.util.endpoint_direction(e.bEndpointAddress) == usb.util.ENDPOINT_IN and e.bmAttributes == 0x02)
	if epin is None:
		reader_device.reset()
		return None, None, None, None, None
	#print "Selected IN endpoint:", epin.bEndpointAddress

	# Match the first IN endpoint
	epintin = usb.util.find_descriptor(intf, custom_match = lambda e: usb.util.endpoint_direction(e.bEndpointAddress) == usb.util.ENDPOINT_IN and e.bmAttributes == 0x03)
	if epin is None:
		reader_device.reset()
		return None, None, None, None, None
	#print "Selected IN INT endpoint:", epintin.bEndpointAddress

	# Return device & endpoints
	return reader_device, intf, epin, epout, epintin

if __name__ == '__main__':
	# Main function
	print ""
	print "Mooltipass Card Reader"
	sequence_number = 0
	user_card_ok = True
	cur_scac_val = 666

	# Search for the reader 
	reader_device, intf, epin, epout, epintin = findReaderDevice(USB_VID, USB_PID, True)

	if reader_device is None:
		sys.exit(0) 

	# print reader info
	sequence_number = print_get_reader_info(epout, epin, sequence_number)		
		
	# Power Off Packet
	print ""
	Power_Off_Packet = array('B')
	Power_Off_Packet.append(0x63)  					# bMessageType
	Power_Off_Packet.append(0x00)  					# dwLength
	Power_Off_Packet.append(0x00)  					# dwLength
	Power_Off_Packet.append(0x00)  					# dwLength
	Power_Off_Packet.append(0x00)  					# dwLength
	Power_Off_Packet.append(0x00)  					# bSlot
	Power_Off_Packet.append(sequence_number)  		# bSeq
	Power_Off_Packet.append(0x00)  					# abRFU
	Power_Off_Packet.append(0x00)  					# abRFU
	Power_Off_Packet.append(0x00)  					# abRFU
	sendPacket(epout, Power_Off_Packet)
	sequence_number += 1
	response = receivePacketWithTimeout(epin, 5000, True)
	if response == None or response[0] != 0x81:
		print "Reader Problem!"
		reader_device.reset()
		sys.exit(0)
	else:
		print "Power Off Packet"
		response_analysis(response[7], response[8])
		if response[9] == 0:
			print "Clock running"
		elif response[9] == 1:
			print "Clock stopped in state L"
		elif response[9] == 2:
			print "Clock stopped in state H"
		elif response[9] == 3:
			print "Clock stopped in an unknown state"
		
	checkInterruptChannel(epintin)		
		
	# Power On Packet
	print ""
	Power_On_Packet = array('B')
	Power_On_Packet.append(0x62)  					# bMessageType
	Power_On_Packet.append(0x00)  					# dwLength
	Power_On_Packet.append(0x00)  					# dwLength
	Power_On_Packet.append(0x00)  					# dwLength
	Power_On_Packet.append(0x00)  					# dwLength
	Power_On_Packet.append(0x00)  					# bSlot
	Power_On_Packet.append(sequence_number)  		# bSeq
	Power_On_Packet.append(0x01)  					# abRFU
	Power_On_Packet.append(0x00)  					# abRFU
	Power_On_Packet.append(0x00)  					# abRFU
	sendPacket(epout, Power_On_Packet)
	sequence_number += 1
	response = receivePacketWithTimeout(epin, 5000, True)
	if response == None or response[0] != 0x80:
		print "Reader Problem!"
		reader_device.reset()
		sys.exit(0)
	elif response[0] != 0x80:
		print "Wrong Answer to Power On!"
		print "Return code:", response[0]
		reader_device.reset()
		sys.exit(0)
	else:
		print "Power On Packet"
		print "abData:", ''.join('0x{:02x} '.format(x) for x in response[10:])
		
	checkInterruptChannel(epintin)	
	
	# Select card type
	print ""
	Select_Card_Packet = array('B')
	Select_Card_Packet.append(0x6F)  					# bMessageType
	Select_Card_Packet.append(0x00)  					# dwLength
	Select_Card_Packet.append(0x00)  					# dwLength
	Select_Card_Packet.append(0x00)  					# dwLength
	Select_Card_Packet.append(0x06)  					# dwLength
	Select_Card_Packet.append(0x00)  					# bSlot
	Select_Card_Packet.append(sequence_number)  		# bSeq
	Select_Card_Packet.append(0xFF)  					# bBWI
	Select_Card_Packet.append(0x00)  					# wLevelParameter
	Select_Card_Packet.append(0x00)  					# wLevelParameter	
	Select_Card_Packet.append(0xFF)  					# CLA
	Select_Card_Packet.append(0xA4)  					# INS
	Select_Card_Packet.append(0x00)  					# P1
	Select_Card_Packet.append(0x00)  					# P2
	Select_Card_Packet.append(0x01)  					# Lc
	Select_Card_Packet.append(0x09)  					# Card type
	sendPacket(epout, Select_Card_Packet)
	sequence_number += 1
	response = receivePacketWithTimeout(epin, 5000, True)
	if response == None or response[0] != 0x80:
		print "Reader Problem!"
		reader_device.reset()
		sys.exit(0)
	else:
		print "Select Card Type Packet"
		response_analysis(response[7], response[8])
		if len(response) > 10 and response[10] == 0x90 and response[11] == 00:
			print "Correctly changed card type to AT88SC102"
		else:
			print "Problem setting the card type to AT88SC102"
			
	# print reader info
	#sequence_number = print_get_reader_info(epout, epin, sequence_number)
	
	# Read Memory Card
	print ""
	Read_Memory_Card_Packet = array('B')
	Read_Memory_Card_Packet.append(0x6F)  					# bMessageType
	Read_Memory_Card_Packet.append(0x00)  					# dwLength
	Read_Memory_Card_Packet.append(0x00)  					# dwLength
	Read_Memory_Card_Packet.append(0x00)  					# dwLength
	Read_Memory_Card_Packet.append(0x05)  					# dwLength
	Read_Memory_Card_Packet.append(0x00)  					# bSlot
	Read_Memory_Card_Packet.append(sequence_number)  		# bSeq
	Read_Memory_Card_Packet.append(0xFF)  					# bBWI
	Read_Memory_Card_Packet.append(0x00)  					# wLevelParameter
	Read_Memory_Card_Packet.append(0x00)  					# wLevelParameter	
	Read_Memory_Card_Packet.append(0xFF)  					# CLA
	Read_Memory_Card_Packet.append(0xB0)  					# INS
	Read_Memory_Card_Packet.append(0x00)  					# P1
	Read_Memory_Card_Packet.append(0x00)  					# Byte Address
	Read_Memory_Card_Packet.append(22)  					# MEM_L
	sendPacket(epout, Read_Memory_Card_Packet)
	sequence_number += 1
	response = receivePacketWithTimeout(epin, 5000, True)
	if response == None or response[0] != 0x80:
		print "Reader Problem!"
		reader_device.reset()
		sys.exit(0)
	else:
		print "Read Memory Card Packet"
		response_analysis(response[7], response[8])
		response[10:] = reverse_bit_order_in_byte_buffer(response[10:])
		card_fz = response[10:12]
		card_iz = response[12:20]
		card_iz.append(0)
		card_iz_string = "".join(map(chr, card_iz))
		card_sc = response[20:22]
		card_scac = response[22:24]
		card_cpz = response[24:32]
		#print "FZ:", ''.join('0x{:02x} '.format(x) for x in response[10:12])
		#print "IZ:", ''.join('0x{:02x} '.format(x) for x in response[12:20])
		#print "IZ:", "".join(map(chr, response[10:20]))
		#print "SC:", ''.join('0x{:02x} '.format(x) for x in response[20:22])
		#print "SCAC:", ''.join('0x{:02x} '.format(x) for x in response[22:24])
		#print "CPZ:", ''.join('0x{:02x} '.format(x) for x in response[24:32])
		#print ''.join('0x{:02x} '.format(x) for x in response[10:])
		#print "".join(map(chr, response[10:]))
		#print response[10:]
		if card_fz[0] == 0x0F and card_fz[1] == 0x0F:
			print "Correct AT88SC102 card inserted"
		else:
			print "Error with AT88SC102 card"
			reader_device.reset()
			sys.exit(0)
		if str(card_iz_string == "hackaday") or str(card_iz_string) == "limpkin":
			print "Card Initialized by Mooltipass"
		else:
			print "Card Not Initialized by Mooltipass"
			user_card_ok = False
		number_of_tries_left = bin((card_scac[0] >> 4)&0x0F).count("1")
		cur_scac_val = number_of_tries_left
		if number_of_tries_left == 0:
			print "Card Blocked!!!"
			user_card_ok = False
		else:
			print "Number of tries left:", number_of_tries_left	
		if card_cpz[0] == 0xFF and card_cpz[1] == 0xFF and card_cpz[2] == 0xFF and card_cpz[3] == 0xFF and card_cpz[4] == 0xFF and card_cpz[5] == 0xFF and card_cpz[6] == 0xFF and card_cpz[7] == 0xFF:
			print "Empty Card"
			user_card_ok = False
		else:
			print "User Card"
	
	# If it is not a user card, return
	if user_card_ok == False:
		reader_device.reset()
		sys.exit(0)
		
	#sequence_number = read_memory_val(epout, epin, sequence_number, 0, 22)
	#sequence_number = read_memory_val(epout, epin, sequence_number, 22, 22)
	
	print ""
	pin_code = raw_input("Please Enter Your PIN: ")
	pin_code = int(pin_code, 16)
	
	card_blocked = False
	sequence_number, card_blocked = verify_security_code(epout, epin, pin_code, sequence_number)
	sequence_number, new_scac_val = read_scac_value(epout, epin, sequence_number)
	if card_blocked == True:
		print "Card Blocked"
	else:
		if new_scac_val == 4:
			print "Correct PIN"	
		else:
			print "Wrong PIN,", new_scac_val, "tries left"
			reader_device.reset()
			sys.exit(0)
	
	reader_device.reset()
	sys.exit(0)
	
	
	# Automatically follow with a PC_to_RDR_Abort
	PC_to_RDR_IccPowerOn_Packet = array('B')
	PC_to_RDR_IccPowerOn_Packet.append(0x62)  # Command ID
	PC_to_RDR_IccPowerOn_Packet.append(0x00)  # DwLength
	PC_to_RDR_IccPowerOn_Packet.append(0x00)  # DwLength
	PC_to_RDR_IccPowerOn_Packet.append(0x00)  # DwLength
	PC_to_RDR_IccPowerOn_Packet.append(0x00)  # DwLength
	PC_to_RDR_IccPowerOn_Packet.append(0x00)  # bSlot
	PC_to_RDR_IccPowerOn_Packet.append(0x00)  # bSeq
	PC_to_RDR_IccPowerOn_Packet.append(0x00)  # abRFU
	PC_to_RDR_IccPowerOn_Packet.append(0x00)  # abRFU
	PC_to_RDR_IccPowerOn_Packet.append(0x00)  # abRFU
	sendPacket(epout, PC_to_RDR_IccPowerOn_Packet)
	print "epin"
	print receivePacketWithTimeout(epin)
	print "epintin"
	print receivePacketWithTimeout(epintin)
	print receivePacketWithTimeout(epin)
	

	# Abort request through control endpoint
	requestType=0x21	# From CCID spec
	request=0x01		# ABORT request
	value=0x0000		# bseq high byte bslot low byte
	index=0x0000		# interface number
	print reader_device.ctrl_transfer(requestType,request,value,index,None,1000)
