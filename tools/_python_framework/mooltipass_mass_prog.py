#!/usr/bin/env python2
#
# Copyright (c) 2014 Mathieu Stephan
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
from mooltipass_hid_device import *
from generate_prog_file import *
import firmwareBundlePackAndSign
from datetime import datetime
from array import array
import pickle
import time
import sys
import os

# TLV Field indexes
LEN_INDEX               = 0x00
CMD_INDEX               = 0x01
DATA_INDEX              = 0x02
# General defines
AES_KEY_LENGTH 			= 256/8
UID_REQUEST_KEY_LENGTH 	= 16
UID_KEY_LENGTH 			= 6
# Custom packets
CMD_BUTTON_PRESSED		= 0x80
CMD_GET_RNG_B_AVAIL		= 0x81
# State machine
PROG_SOCKET_IDLE		= 0
PROG_SOCKET_PENDING		= 1
PROG_SOCKET_PROGRAMMING	= 2


def pickle_write(data, outfile):
	f = open(outfile, "w+b")
	pickle.dump(data, f)
	f.close()
		
def pickle_read(filename):
	f = open(filename)
	data = pickle.load(f)
	f.close()
	return data

def main():
	print "Mooltipass Mass Programming Tool"
	
	# Temp vars
	prog_socket_states = [PROG_SOCKET_IDLE, PROG_SOCKET_IDLE, PROG_SOCKET_IDLE, PROG_SOCKET_IDLE, PROG_SOCKET_IDLE, PROG_SOCKET_IDLE, PROG_SOCKET_IDLE, PROG_SOCKET_IDLE, PROG_SOCKET_IDLE]
	temp_counter = 0
	
	# Random bytes buffer
	random_bytes_buffer = []
	
	# HID device constructor
	mooltipass_device = mooltipass_hid_device()	
	
	# Connect to device
	if mooltipass_device.connect(True) == False:
		sys.exit(0)
		
	# Change defaut time out to 300ms
	mooltipass_device.getInternalDevice().setReadTimeout(300)
	
	# Check for random numbers file presence
	if os.path.isfile("rng.bin"):
		random_bytes_buffer = pickle_read("rng.bin")
		
	# Main loop
	while True:
		# Try to receive data from the device
		received_data = mooltipass_device.getInternalDevice().receiveHidPacketWithTimeout()
		temp_counter = temp_counter + 1
		
		# Check if we received something
		if received_data != None:
			if received_data[CMD_INDEX] == CMD_BUTTON_PRESSED:
				# Pushed button extraction
				pushed_button_id = received_data[DATA_INDEX]
				
				# Programming button was pressed on the bench
				print "Button", pushed_button_id, "pressed"
				prog_socket_states[pushed_button_id] = PROG_SOCKET_PENDING
		
		# Get number of available random bytes
		mooltipass_device.getInternalDevice().sendHidPacket(mooltipass_device.getPacketForCommand(CMD_GET_RNG_B_AVAIL, 0, None))
		nb_random_bytes_available = mooltipass_device.getInternalDevice().receiveHidPacketWithTimeout()[DATA_INDEX]
		
		# If more than 32 bytes, fetch them
		if nb_random_bytes_available >= 32:
			mooltipass_device.getInternalDevice().sendHidPacket(mooltipass_device.getPacketForCommand(CMD_GET_RANDOM_NUMBER, 0, None))
			random_bytes = mooltipass_device.getInternalDevice().receiveHidPacketWithTimeout()[DATA_INDEX:DATA_INDEX+32]
			random_bytes_buffer.extend(random_bytes)
			
		# Store random bytes buffer every now and then
		if temp_counter % 30 == 0:
			pickle_write(random_bytes_buffer, "rng.bin")
			print "Random bytes buffer saved:", len(random_bytes_buffer), "bytes available"
			#print random_bytes_buffer
			
		# If we have enough random bytes and a button was pressed, program the MCU
		if len(random_bytes_buffer) >= AES_KEY_LENGTH+AES_KEY_LENGTH+UID_REQUEST_KEY_LENGTH+UID_KEY_LENGTH:
			# Check if a button was pressed
			for i in range(0, 9):
				if prog_socket_states[i] == PROG_SOCKET_PENDING:
					print "Starting programming for socket", i
					
					# Generate programming file
					
					#generateFlashAndEepromHex(sys.argv[2], sys.argv[3], 12345, [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0], [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0], [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0], [0,0,0,0,0,0], "newflash.hex", "neweeprom.hex", True)
					
					# Change state to programming
					prog_socket_states[i] = PROG_SOCKET_PROGRAMMING

if __name__ == "__main__":
	main()

	
	
	
	
	
	
	
	
	
	
	
	