#!/usr/bin/env python
#
# Copyright (c) 2014 Darran Hunt (darran [at] hunt dot net dot nz)
# All rights reserved.
#
# CDDL HEADER START
#
# The contents of this file are subject to the terms of the
# Common Development and Distribution License (the "License").
# You may not use this file except in compliance with the License.
#
# You can obtain a copy of the license at src/license_cddl-1.0.txt
# or http://www.opensolaris.org/os/licensing.
# See the License for the specific language governing permissions
# and limitations under the License.
#
# When distributing Covered Code, include this CDDL HEADER in each
# file and include the License file at src/license_cddl-1.0.txt
# If applicable, add the following below this CDDL HEADER, with the
# fields enclosed by brackets "[]" replaced with your own identifying
# information: Portions Copyright [yyyy] [name of copyright owner]
#
# CDDL HEADER END
from optparse import OptionParser
from array import array
from PIL import Image
import platform
import usb.core
import usb.util
import random
import time
import sys

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

CMD_PING                = 0xA1
CMD_MINI_FRAME_BUF_DATA = 0x9E

parser = OptionParser(usage = 'usage: %prog [options]')
parser.add_option('-i', '--input', help='image to send to the screen', dest='input', default=None)
parser.add_option('-r', '--reverse', help='set to inverse pixel data', action="store_true", dest='reverse', default=False)
(options, args) = parser.parse_args()

if options.input == None:
	parser.error('input option is required')
	
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

def main():
	# Search for the mooltipass and read hid data
	hid_device, intf, epin, epout = findHIDDevice(USB_VID, USB_PID, True)
	if hid_device is None:
		sys.exit(0)
		
	# Open image
	image = Image.open(options.input)
	
	# If PNG, convert to RGB
	if image.mode == "P":
		image = image.convert('RGBA')		
	
	# Get image specs
	img_format = image.format
	img_size = image.size
	img_mode = image.mode
	print "Format:", img_format, "size:", img_size, "mode:", img_mode
	
	# Check size
	if img_size[0] != 128 or img_size[1] != 32:
		print "Wrong dimensions or format!"
		sys.exit(0)
		
	# Turn image left 90 degrees
	image_rot = image.rotate(270)
	#image_rot.show()
	
	# Process the pixels
	pixel_data = 0
	bitCount = 7
	bitstream = []
	for page in reversed(range(0, image_rot.size[0]/8)):
		for y in range(0, image_rot.size[1]):
			bitCount = 7
			pixel_data = 0
			for x in range(page*8, page*8 + 8):
				# Get RGB values
				rgb = image_rot.getpixel((x, y))
				# Update pixel data
				if options.reverse:
					if rgb[0] < 128:
						pixel_data |= 1 << bitCount
				else:
					if rgb[0] > 128:
						pixel_data |= 1 << bitCount
				bitCount-=1
				if bitCount < 0:
					bitstream.append(pixel_data)
					pixel_data = 0
					bitCount = 7
			if bitCount != 7:
				bitstream.append(pixel_data)
			
	#print bitstream
	
	# Send pixel data, to update it when length isn't a multiple of 32....
	for i in range(0, len(bitstream)/32):
		packet = array('B')
		packet.append((i*32)&0x0FF)
		packet.append((i*32)>>8)
		packet.extend(bitstream[i*32: i*32 + 32])
		sendHidPacket(epout, CMD_MINI_FRAME_BUF_DATA, 34, packet)
		
	#hid_device.reset()

if __name__ == "__main__":
	main()
