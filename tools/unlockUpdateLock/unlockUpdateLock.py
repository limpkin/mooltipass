from intelhex import IntelHex
from array import array
from time import sleep
import serial, sys
import commands
import platform
import usb.core
import usb.util
import os.path
import random
import struct
import string
import pickle
import glob
import copy
import time
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

class loader:
	def __init__(self):
		name = 'aoBoot'
	def writePage(self):
		self.ser.write('m')
		ret_val = self.ser.read()
		if ret_val != '\r':
			print 'Page write failed, return char:', ret_val

	def getId(self):
		self.ser.write('S')
		ret_val = self.ser.read(7)
		print "Bootloader ID:", ret_val

	def getType(self):
		self.ser.write('p')
		ret_val = self.ser.read()
		print "Bootloader Type:", ret_val

	def getVersion(self):
		self.ser.write('V')
		ret_val = self.ser.read(2)
		print "BL Version:", ret_val[0] + "." + ret_val[1]		
			
	def setAddress(self,addr):
		self.ser.write('A')
		self.ser.write(chr((addr>>9) & 0xff))
		self.ser.write(chr((addr>>1) & 0xff))
		ret_val = self.ser.read()
		if ret_val != '\r':
			print 'Set address failed, return char:', ret_val
		
	def sendLowByte(self,data):
		#print hex(data)
		self.ser.write('c')
		self.ser.write(chr(data))
		ret_val = self.ser.read()
		if ret_val != '\r':
			print 'Low byte write failed, return char:', ret_val

	def sendHighByte(self,data):
		#print hex(data)
		self.ser.write('C')
		self.ser.write(chr(data))
		ret_val = self.ser.read()
		if ret_val != '\r':
			print 'High byte write failed, return char:', ret_val

	def enableRwwSection(self):
		self.ser.write('E')
		ret_val = self.ser.read()
		if ret_val != '\r':
			print 'enableRwwSection write failed, return char:', ret_val

	def eraseFlash(self):
		self.ser.write('e')
		ret_val = self.ser.read()
		if ret_val != '\r':
			print 'Flash erase failed, return char:', ret_val

	def resetTimeOut(self):
		# Here is actually a read mem command with erroneous parameters
		self.ser.write("gggg")
		ret_val = self.ser.read()
		if ret_val != '?':
			print "Couldn't reset timeout timer, return char:", ret_val
			
	def writeFlashPage(self,data):
		self.ser.write("B" + chr(0) + chr(128) + "F")
		for i in range(0, 128):
			self.ser.write(chr(data[i]))
		ret_val = self.ser.read()
		if ret_val != '\r':
			print 'Write flash page failed, return char:', ret_val

	def openSerial(self,port,baud):
		try:
			self.ser = serial.Serial(port,baud)
		except:
			print 'Failed to open COM port'

def serial_ports():
    """ Lists serial port names

        :raises EnvironmentError:
            On unsupported or unknown platforms
        :returns:
            A list of the serial ports available on the system
    """
    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result
		
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

def mooltipassInit(hid_device, intf, epin, epout):
	# Operation success state
	success_status = 1

	# Empty set password packet
	mooltipass_password = array('B')

	# We need 62 random bytes to set them as a password for the Mooltipass
	sys.stdout.write('Getting random bytes... ')
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
		sys.stdout.write('Uploading bundle... ')
		sys.stdout.flush()
		sendHidPacket(epout, CMD_IMPORT_MEDIA_START, 62, mooltipass_password)
		# Check that the import command worked
		if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
			# Open bundle file
			bundlefile = open('bundle_tutorial.img', 'rb')
			#bundlefile = open('bundle_tutorial_en_dv.img', 'rb')					
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
						success_status = 0
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

	# Send set password packet
	if success_status == 1:
		sys.stdout.write('Setting password... ')
		sys.stdout.flush()
		sendHidPacket(epout, CMD_SET_BOOTLOADER_PWD, 62, mooltipass_password)
		#print mooltipass_password
		if receiveHidPacket(epin)[DATA_INDEX] == 0x01:
			print "Done!"
			print ""
			print "Your new password is:", "".join(format(x, "02x") for x in mooltipass_password)
			print "PLEASE WRITE IT SOMEWHERE!"
			# Update Success status
			success_status = 1
		else:
			success_status = 0
			print "fail!!!"
			print "likely causes: mooltipass already setup"

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
	print ""
	print "Mooltipass USB update tool"
	print "Requirements: Debian based Linux, Python 2.7, script launched as root"
	print ""

	# Search for the mooltipass and read hid data
	hid_device, intf, epin, epout = findHIDDevice(USB_VID, USB_PID, True)
    
	if hid_device is None:
		sys.exit(0)
		
	# Get Mootipass status
	sendHidPacket(epout, CMD_MOOLTIPASS_STATUS, 0, None)
	status_data = receiveHidPacket(epin)
		
	# Only allow script to be run when a card is inserted
	if status_data[DATA_INDEX] != 5 and status_data[DATA_INDEX] != 9:
		print "Please insert a card in the mooltipass and restart this script!"
		sys.exit(0)
	
	print ""
	
	# Jump to bootloader command
	password = raw_input("Please enter your Mooltipass unique password: ")
	sendHidPacket(epout, CMD_JUMP_TO_BOOTLOADER, 62, array('B', password.decode("hex")))
	print "Sending jump to bootloader with password, PLEASE APPROVE REQUEST ON THE DEVICE"
	
	# Wait for the new serial interface to come up
	sys.stdout.write("Waiting for the serial interface to appear...")
	sys.stdout.flush()	
	orig_com_ports = serial_ports()
	new_com_ports = orig_com_ports
	counter = 0
	while new_com_ports == orig_com_ports and counter < 20:
		time.sleep(1)
		new_com_ports = serial_ports();
		sys.stdout.write(".")
		sys.stdout.flush()
		counter = counter + 1
	
	print ""
	# See if we found something...
	if counter >= 20:
		print "Couldn't find device!"
	else:
		print "Device found: " + list(set(new_com_ports) - set(orig_com_ports))[0] + ", starting programming process..."
		
		# Starting the programming process
		addr = 0
		page = 0
		prog = loader()
		firmware = IntelHex("Mooltipass.hex")
		port = list(set(new_com_ports) - set(orig_com_ports))[0]			
		prog.openSerial(port,57600)
		prog.getId()
		prog.getType()
		prog.getVersion()
		for i in range(0,len(firmware),128):
			prog.setAddress(addr)
			#print "Writing page", hex(addr)
			prog.writeFlashPage(firmware.tobinarray(start=addr, size=128))
			addr += 128	  
		prog.enableRwwSection()
		prog.ser.close()
		
		print "Programming done... reconnecting to device..."
		time.sleep(10)
		
		# Upload bundle, set password...
		hid_device = None
		while hid_device == None:
			hid_device, intf, epin, epout = findHIDDevice(USB_VID, USB_PID, False)
			time.sleep(1)		
			
		print "Connected to device, reuploading bundle and resetting device password..."
		mooltipassInit(hid_device, intf, epin, epout)			
		sendHidPacket(epout, CMD_VERSION, 0, None)
		data = receiveHidPacket(epin)

		print "Firmware updated to version", "".join(map(chr, data[DATA_INDEX+1:])).split(b"\x00")[0]
		print "PLEASE UNPLUG AND RE-PLUG YOUR DEVICE"
	
	
	
	