#!/usr/bin/env python
#
# Copyright (c) 2016 Mathieu Stephan
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
from datetime import datetime
from array import array
import platform
import usb.core
import usb.util
import random
import signal
import time
import sys

# Set to true to get advanced debugging information
HID_DEVICE_DEBUG	= False

# Generic HID device class
class generic_hid_device:

	# Device constructor
	def __init__(self):
		# Install SIGINT catcher
		signal.signal(signal.SIGINT, self.signal_handler)
		self.connected = False
		
	# Catch CTRL-C interrupt
	def signal_handler(self, signal, frame):
		print "Keyboard Interrupt"
		self.disconnect()
		sys.exit(0)
		
	# Send HID packet to device
	def sendHidPacket(self, data):
		# check that we're actually connected to a device
		if self.connected == False:
			print "Not connected to device"
			return
		
		# data to send
		arraytosend = array('B')

		# add the data
		if data is not None:
			arraytosend.extend(data)
		else:
			print "Data is None"
			return

		# debug: print sent data
		if HID_DEVICE_DEBUG:
			print "TX DBG data:", ' '.join(hex(x) for x in arraytosend)

		# send data
		self.epout.write(arraytosend)
	
	# Receive HID packet, crash when nothing is sent
	def receiveHidPacket(self):
		# check that we're actually connected to a device
		if self.connected == False:
			print "Not connected to device"
			return None

		# read from endpoint
		try :
			data = self.epin.read(self.epin.wMaxPacketSize, timeout=self.read_timeout)
			if HID_DEVICE_DEBUG:
				print "RX DBG data:", ' '.join(hex(x) for x in data)
			return data
		except usb.core.USBError as e:
			if HID_DEVICE_DEBUG:
				print e
			sys.exit("Device didn't send a packet")
		
	# Receive HID packet, return None when nothing is sent	
	def receiveHidPacketWithTimeout(self):
		try :
			data = self.epin.read(self.epin.wMaxPacketSize, timeout=self.read_timeout)
			if HID_DEVICE_DEBUG:
				print "RX DBG data:", ' '.join(hex(x) for x in data)
			return data
		except usb.core.USBError as e:
			return None
		
	# Try to connect to HID device. 
	# ping_packet: an array containing a ping packet to send to the device over HID
	# pong_check_func: a function which takes a ping packet and it's reply as parameters, replies True if OK
	def connect(self, print_debug, device_vid, device_pid, read_timeout, ping_packet, pong_check_func):
		# Find our device
		self.hid_device = usb.core.find(idVendor=device_vid, idProduct=device_pid)
		self.read_timeout = read_timeout

		# Was it found?
		if self.hid_device is None:
			if print_debug:
				print "Device not found"
			return False

		# Device found
		if print_debug:
			print "USB device found"

		# Different init codes depending on the platform
		if platform.system() == "Linux":
			# Need to do things differently
			try:
				self.hid_device.detach_kernel_driver(0)
				self.hid_device.reset()
			except Exception, e:
				pass # Probably already detached
		else:
			# Set the active configuration. With no arguments, the first configuration will be the active one
			try:
				self.hid_device.set_configuration()
			except Exception, e:
				if print_debug:
					print "Cannot set configuration the device:" , str(e)
				return False

		if HID_DEVICE_DEBUG:
			for cfg in self.hid_device:
				print "configuration val:", str(cfg.bConfigurationValue)
				for intf in cfg:
					print "int num:", str(intf.bInterfaceNumber), ", int alt:", str(intf.bAlternateSetting)
					for ep in intf:
						print "endpoint addr:", str(ep.bEndpointAddress)

		# Get an endpoint instance
		cfg = self.hid_device.get_active_configuration()
		intf = cfg[(0,0)]

		# Match the first OUT endpoint
		self.epout = usb.util.find_descriptor(intf, custom_match = lambda e: usb.util.endpoint_direction(e.bEndpointAddress) == usb.util.ENDPOINT_OUT)
		if self.epout is None:
			self.hid_device.reset()
			return False
			
		if HID_DEVICE_DEBUG:
			print "Selected OUT endpoint:", self.epout.bEndpointAddress

		# Match the first IN endpoint
		self.epin = usb.util.find_descriptor(intf, custom_match = lambda e: usb.util.endpoint_direction(e.bEndpointAddress) == usb.util.ENDPOINT_IN)
		if self.epin is None:
			self.hid_device.reset()
			return False
			
		if HID_DEVICE_DEBUG:
			print "Selected IN endpoint:", self.epin.bEndpointAddress

		time.sleep(0.5)
		try:
			# try to send ping packet
			self.epout.write(ping_packet)
			if HID_DEVICE_DEBUG:
				print "TX DBG data:", ' '.join(hex(x) for x in ping_packet)
			# try to receive one answer
			temp_bool = True
			while temp_bool:
				try :
					# try to receive answer
					data = self.epin.read(self.epin.wMaxPacketSize, timeout=2000)
					if HID_DEVICE_DEBUG:
						print "RX DBG data:", ' '.join(hex(x) for x in data)
					# check that the received data is correct
					if pong_check_func(ping_packet, data) == True :
						temp_bool = False
						if print_debug:
							print "Device replied to our ping message"
					else:
						if print_debug:
							print "Cleaning remaining input packets"
					time.sleep(.5)
				except usb.core.USBError as e:
					if print_debug:
						print e
					return False
		except usb.core.USBError as e:
			if print_debug:
				print e
			return False

		# Set connected var, return success
		self.connected = True
		return True
		
	# Disconnect from HID device
	def disconnect(self):
		# check that we're actually connected to a device
		if self.connected == False:
			print "Not connected to device"
			return
		else:
			print "Disconnecting from device..."
		
		# reset device
		self.hid_device.reset()
		
	# Benchmark ping pong speed	
	def benchmarkPingPongSpeed(self, ping_packet):	
		# check that we're actually connected to a device
		if self.connected == False:
			print "Not connected to device"
			return

		# start ping ponging
		current_second = datetime.now().second
		data_counter = 0
		while True:
			self.sendHidPacket(ping_packet)
			self.receiveHidPacket()
			data_counter += 64
			
			# Print out performance
			if current_second != datetime.now().second:
				current_second = datetime.now().second
				print "Ping pong transfer speed (unidirectional):", data_counter , "B/s"
				data_counter = 0
