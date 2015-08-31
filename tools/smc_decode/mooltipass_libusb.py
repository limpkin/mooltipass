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
USB_PID					= 0x9000

def receivePacketWithTimeout(epin, timeout_ms):
	try :
		data = epin.read(epin.wMaxPacketSize, timeout=timeout_ms)
		return data
	except usb.core.USBError as e:
		return None

def checkInterruptChannel(epintin):
	recv = receivePacketWithTimeout(epintin, 10)
	if recv != None:
		print "Interrupt packet:", recv

def sendPacket(epout, data):
	# send data
	#for i in range(0, 64-len(data)):
	#	data.append(0)
	#print data
	epout.write(data)
	
def print_card_type(data):
	if data & 0x0001 != 0:
		print "No card type"
	if data & 0x0002 != 0:
		print "I2C <= 16kb card type"
	if data & 0x0004 != 0:
		print "I2C > 16kb card type"
	if data & 0x0008 != 0:
		print "AT88SC153 card type"
	if data & 0x0010 != 0:
		print "AT88SC1608 card type"
	if data & 0x0020 != 0:
		print "SLE4418/28 card type"
	if data & 0x0040 != 0:
		print "SLE4432/42 card type"
	if data & 0x0080 != 0:
		print "SLE4406/36 & SLE5536 card type"
	if data & 0x0100 != 0:
		print "SLE4404 card type"
	if data & 0x0200 != 0:
		print "AT88SC101/102/1003 card type"
	if data & 0x1000 != 0:
		print "MCU T=0 card type"
	if data & 0x2000 != 0:
		print "MCU T=1 card type"
	
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
	
def print_acr_stat(epout):
	# This damn device uses FW1.10 commands
	Get_Acr_Stat = array('B')
	Get_Acr_Stat.append(0x01)  				# Header
	Get_Acr_Stat.append(0x01)  				# Instruction
	Get_Acr_Stat.append(0x00)  				# Data Length
	Get_Acr_Stat.append(0x00)  				# Data Length
	epout.write(Get_Acr_Stat)
	acr = receivePacketWithTimeout(epin, 10000)
	
	print "ACR reader found"
	print "Max number of command data bytes:", acr[14]
	print "Max number of data bytes that can be requested to be transmitted in a response:", acr[15]
	print "Supported card types :"	
	print_card_type(int(acr[16])*256 + int(acr[17]))
	print_selected_card_type(acr[18])		
	if acr[19] == 0:
		print "No card inserted"
	elif acr[19] == 1:
		print "Card inserted, not powered up"
	elif acr[19] == 3:
		print "Card inserted, powered up"

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

	# Search for the reader 
	reader_device, intf, epin, epout, epintin = findReaderDevice(USB_VID, USB_PID, True)

	if reader_device is None:
		sys.exit(0) 
	
	# Power Off Packet
	Power_Off_Packet = array('B')
	Power_Off_Packet.append(0x01)  					# Header
	Power_Off_Packet.append(0x81)  					# Instruction
	Power_Off_Packet.append(0x00)  					# Data Length
	Power_Off_Packet.append(0x00)  					# Data Length
	epout.write(Power_Off_Packet)
	print "Power Off Packet", receivePacketWithTimeout(epin, 1000)
	checkInterruptChannel(epintin)
	epout.write(Power_Off_Packet)
	print "Power Off Packet", receivePacketWithTimeout(epin, 1000)
	checkInterruptChannel(epintin)
	
	# Select card type
	Select_Card_Packet = array('B')
	Select_Card_Packet.append(0x01)  				# Header
	Select_Card_Packet.append(0x02)  				# Instruction
	Select_Card_Packet.append(0x00)  				# Data Length
	Select_Card_Packet.append(0x01)  				# Data Length
	Select_Card_Packet.append(0x09)  				# AT88SC101/102/1003 mode
	epout.write(Select_Card_Packet)
	print "Select Card Packet", receivePacketWithTimeout(epin, 1000)
	checkInterruptChannel(epintin)
	
	# Set Acr options
	Acr_Options_Packet = array('B')
	Acr_Options_Packet.append(0x01)  				# Header
	Acr_Options_Packet.append(0x07)  				# Instruction
	Acr_Options_Packet.append(0x00)  				# Data Length
	Acr_Options_Packet.append(0x01)  				# Data Length
	Acr_Options_Packet.append(0x30)  				# EMV & Memory card mode
	epout.write(Acr_Options_Packet)
	print "Set Acr Options Packet", receivePacketWithTimeout(epin, 1000)
	checkInterruptChannel(epintin)
	epout.write(Acr_Options_Packet)
	print "Set Acr Options Packet", receivePacketWithTimeout(epin, 1000)
	checkInterruptChannel(epintin)
	
	# Reset with 5 volts
	Reset_With_5V_Packet = array('B')
	Reset_With_5V_Packet.append(0x01)  				# Header
	Reset_With_5V_Packet.append(0x80)  				# Instruction
	Reset_With_5V_Packet.append(0x00)  				# Data Length
	Reset_With_5V_Packet.append(0x01)  				# Data Length
	Reset_With_5V_Packet.append(0x01)  				# 5V card
	epout.write(Reset_With_5V_Packet)
	print "Reset with 5 volts Packet", receivePacketWithTimeout(epin, 1000)
	checkInterruptChannel(epintin)
	
	#print_acr_stat(epout)
	
	# Set Acr options
	Acr_Options_Packet = array('B')
	Acr_Options_Packet.append(0x01)  				# Header
	Acr_Options_Packet.append(0x07)  				# Instruction
	Acr_Options_Packet.append(0x00)  				# Data Length
	Acr_Options_Packet.append(0x01)  				# Data Length
	Acr_Options_Packet.append(0x20)  				# Memory card mode
	epout.write(Acr_Options_Packet)
	print "Set Acr Options Packet", receivePacketWithTimeout(epin, 1000)
	checkInterruptChannel(epintin)
	
	# Select card type (adpu style)
	Select_Card_Packet = array('B')
	Select_Card_Packet.append(0x01)  				# Header
	Select_Card_Packet.append(0xA0)  				# Instruction
	Select_Card_Packet.append(0x00)  				# Data Length
	Select_Card_Packet.append(0x06)  				# Data Length
	Select_Card_Packet.append(0xFF)  				# CLA
	Select_Card_Packet.append(0xA4)  				# INS
	Select_Card_Packet.append(0x00)  				# P1
	Select_Card_Packet.append(0x00)  				# P2
	Select_Card_Packet.append(0x01)  				# Lc
	Select_Card_Packet.append(0x09)  				# Card type
	epout.write(Select_Card_Packet)
	print "Select Card Packet", receivePacketWithTimeout(epin, 1000)
	checkInterruptChannel(epintin)
	epout.write(Select_Card_Packet)
	print "Select Card Packet", receivePacketWithTimeout(epin, 1000)
	checkInterruptChannel(epintin)
	
	sys.exit(0) 
	
	# Print Acr stat
	print_acr_stat(epout);	
		
	# Power on packet
	PC_to_RDR_IccPowerOn_Packet = array('B')
	PC_to_RDR_IccPowerOn_Packet.append(0x62)  # Command ID
	PC_to_RDR_IccPowerOn_Packet.append(0x00)  # DwLength
	PC_to_RDR_IccPowerOn_Packet.append(0x00)  # DwLength
	PC_to_RDR_IccPowerOn_Packet.append(0x00)  # DwLength
	PC_to_RDR_IccPowerOn_Packet.append(0x00)  # DwLength
	PC_to_RDR_IccPowerOn_Packet.append(0x00)  # bSlot
	PC_to_RDR_IccPowerOn_Packet.append(sequence_number)  # bSeq
	PC_to_RDR_IccPowerOn_Packet.append(0x01)  # Power Select
	PC_to_RDR_IccPowerOn_Packet.append(0x00)  # abRFU
	PC_to_RDR_IccPowerOn_Packet.append(0x00)  # abRFU
	sendPacket(epout, PC_to_RDR_IccPowerOn_Packet)
	sequence_number+=1
	print "epin"
	print receivePacketWithTimeout(epin)
	
	reader_device.reset()
	sys.exit(0)
	
	# Get Reader Info
	Get_Reader_Info_Packet = array('B')
	Get_Reader_Info_Packet.append(0x6F)  # Command ID
	Get_Reader_Info_Packet.append(0x00)  # DwLength
	Get_Reader_Info_Packet.append(0x00)  # DwLength
	Get_Reader_Info_Packet.append(0x00)  # DwLength
	Get_Reader_Info_Packet.append(0x05)  # DwLength
	Get_Reader_Info_Packet.append(0x00)  # bSlot
	Get_Reader_Info_Packet.append(0x00)  # bSeq
	Get_Reader_Info_Packet.append(0x00)  # bBWI
	Get_Reader_Info_Packet.append(0x00)  # wLevelParameter 
	Get_Reader_Info_Packet.append(0x00)  # wLevelParameter 	
	Get_Reader_Info_Packet.append(0xFF)  # CLA
	Get_Reader_Info_Packet.append(0x09)  # INS
	Get_Reader_Info_Packet.append(0x00)  # P1
	Get_Reader_Info_Packet.append(0x00)  # P2
	Get_Reader_Info_Packet.append(0x10)  # Lc
	sendPacket(epout, Get_Reader_Info_Packet)
	print "epin"
	print receivePacketWithTimeout(epin)
	print "epintin"
	print receivePacketWithTimeout(epintin)
	print receivePacketWithTimeout(epin)
	
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
	
	# Get current status of the slot
	setCardTypePacket = array('B')
	setCardTypePacket.append(0xFF)
	setCardTypePacket.append(0xA4)
	setCardTypePacket.append(0x00)
	setCardTypePacket.append(0x00)
	setCardTypePacket.append(0x01)
	setCardTypePacket.append(0)
	sendPacket(epout, setCardTypePacket)
	print "epin"
	print receivePacketWithTimeout(epin)
	print "epintin"
	print receivePacketWithTimeout(epintin)
	
	# Get current status of the slot
	getSlotSatusReq = array('B')
	getSlotSatusReq.append(0x65)
	getSlotSatusReq.append(0x00)
	getSlotSatusReq.append(0x00)
	getSlotSatusReq.append(0x00)
	getSlotSatusReq.append(0x00)
	getSlotSatusReq.append(0)
	getSlotSatusReq.append(0)
	getSlotSatusReq.append(0x00)
	getSlotSatusReq.append(0x00)
	getSlotSatusReq.append(0x00)
	sendPacket(epout, getSlotSatusReq)
	print "epin"
	print receivePacketWithTimeout(epin)
	print "epintin"
	print receivePacketWithTimeout(epintin)
		
	reader_device.reset()
	sys.exit(0)

	

	# Abort request through control endpoint
	requestType=0x21	# From CCID spec
	request=0x01		# ABORT request
	value=0x0000		# bseq high byte bslot low byte
	index=0x0000		# interface number
	print reader_device.ctrl_transfer(requestType,request,value,index,None,1000)
