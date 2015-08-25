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

def receivePacketWithTimeout(epin):
	try :
		data = epin.read(epin.wMaxPacketSize, timeout=10000)
		return data
	except usb.core.USBError as e:
		return None


def sendPacket(epout, data):
	# send data
	#for i in range(0, 64-len(data)):
	#	data.append(0)
	#print data
	print epout.write(data)

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

	# Search for the reader 
	reader_device, intf, epin, epout, epintin = findReaderDevice(USB_VID, USB_PID, True)

	if reader_device is None:
		sys.exit(0) 
	
	# Select card type packet
	Select_Card_Type_Packet = array('B')
	Select_Card_Type_Packet.append(0x6F)  # Command ID
	Select_Card_Type_Packet.append(0x06)  # DwLength
	Select_Card_Type_Packet.append(0x00)  # DwLength
	Select_Card_Type_Packet.append(0x00)  # DwLength
	Select_Card_Type_Packet.append(0x00)  # DwLength
	Select_Card_Type_Packet.append(0x00)  # bSlot
	Select_Card_Type_Packet.append(0x00)  # bSeq
	Select_Card_Type_Packet.append(0x00)  # bBWI
	Select_Card_Type_Packet.append(0x00)  # wLevelParameter 
	Select_Card_Type_Packet.append(0x00)  # wLevelParameter 	
	Select_Card_Type_Packet.append(0xFF)  # CLA
	Select_Card_Type_Packet.append(0xA4)  # INS
	Select_Card_Type_Packet.append(0x00)  # P1
	Select_Card_Type_Packet.append(0x00)  # P2
	Select_Card_Type_Packet.append(0x01)  # Lc
	Select_Card_Type_Packet.append(0x06)  # Card type
	epout.write(Select_Card_Type_Packet)
	sendPacket(epout, Select_Card_Type_Packet)
	print "epin"
	print receivePacketWithTimeout(epin)
	print "epintin"
	print receivePacketWithTimeout(epintin)
	
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
	
	# Abort request through control endpoint
	requestType=0x21	# From CCID spec
	request=0x01		# ABORT request
	value=0x0000		# bseq high byte bslot low byte
	index=0x0000		# interface number
	print reader_device.ctrl_transfer(requestType,request,value,index,None,1000)
	
	# Automatically follow with a PC_to_RDR_Abort
	PC_to_RDR_Abort_Packet = array('B')
	PC_to_RDR_Abort_Packet.append(0x72)  # Command ID
	PC_to_RDR_Abort_Packet.append(0x00)  # DwLength
	PC_to_RDR_Abort_Packet.append(0x00)  # DwLength
	PC_to_RDR_Abort_Packet.append(0x00)  # DwLength
	PC_to_RDR_Abort_Packet.append(0x00)  # DwLength
	PC_to_RDR_Abort_Packet.append(0x00)  # bSlot
	PC_to_RDR_Abort_Packet.append(0x00)  # bSeq
	PC_to_RDR_Abort_Packet.append(0x00)  # abRFU
	PC_to_RDR_Abort_Packet.append(0x00)  # abRFU
	PC_to_RDR_Abort_Packet.append(0x00)  # abRFU
	sendPacket(epout, PC_to_RDR_Abort_Packet)
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

