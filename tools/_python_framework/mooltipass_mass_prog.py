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
try:
	import seccure
except ImportError:
	pass
import threading
import commands
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
CMD_PROG_DONE			= 0x82
CMD_PROG_FAILURE		= 0x83
CMD_DISPLAY_LINE1       = 0x84
CMD_DISPLAY_LINE2       = 0x85
CMD_DISPLAY_LINE3       = 0x86
# State machine
PROG_SOCKET_IDLE		= 0
PROG_SOCKET_PENDING		= 1
PROG_SOCKET_PROGRAMMING	= 2

# Lines currently displayed on the screen
displayed_texts = []


class FuncThread(threading.Thread):
	def __init__(self, target, *args):
		self._target = target
		self._args = args
		threading.Thread.__init__(self)

	def run(self):
		self._return = self._target(*self._args)
		
	def join(self):
		threading.Thread.join(self)
		return self._return

def pickle_write(data, outfile):
	f = open(outfile, "w+b")
	pickle.dump(data, f)
	f.close()
		
def pickle_read(filename):
	f = open(filename)
	data = pickle.load(f)
	f.close()
	return data
	
def add_line_on_screen(mooltipass_device, line):
	global displayed_texts
	displayed_texts.append(line)
	if len(displayed_texts) > 3:
		del(displayed_texts[0])
	for i in range(0, len(displayed_texts)):
		mooltipass_device.getInternalDevice().sendHidPacket(mooltipass_device.getPacketForCommand(CMD_DISPLAY_LINE1+i, len(displayed_texts[i])+1, mooltipass_device.textToByteArray(displayed_texts[i])))
		mooltipass_device.getInternalDevice().receiveHidPacketWithTimeout()
	
def start_programming(socket_id, mooltipass_id, flashFile, EepromFile, encryptedKeysFile):
	print "Programming socket", socket_id, "with flash file", flashFile, "and eeprom file", EepromFile
	
	# Socket ID to programmer serial number Look Up Table
	socket_id_to_programmer_serial = ["1230", "1231", "1232", "1233", "1234", "1235", "1236", "1237", "1238"]
	
	# If needed: test mode
	testMode = False
	if testMode:	
		time.sleep(10)	
		success_state = False		
		if mooltipass_id % 2 == 0:		
			success_state = True
		return [success_state, mooltipass_id, flashFile, EepromFile, "test mode", encryptedKeysFile]
	
	# Read fuses using avrdude
	avrdude_command = "avrdude -c avrisp2 -p m32u4 -B 10 -P usb:"+socket_id_to_programmer_serial[socket_id]+" -U lfuse:r:/tmp/low_fuse_val_"+str(socket_id)+".hex:r -U hfuse:r:/tmp/high_fuse_val_"+str(socket_id)+".hex:r -U efuse:r:/tmp/extended_fuse_val_"+str(socket_id)+".hex:r -U lock:r:/tmp/lock_fuse_val_"+str(socket_id)+".hex:r"
	output_avrdude = commands.getstatusoutput(avrdude_command)
	
	# Check for generated files
	if not os.path.isfile("/tmp/low_fuse_val_"+str(socket_id)+".hex"):
		print "Couldn't read fuse files"
		print "Avrdude command", avrdude_command
		print "Output:", output_avrdude
		return [False, mooltipass_id, flashFile, EepromFile, "avrdude error!", encryptedKeysFile]
		
	# Read generated files
	file = open("/tmp/low_fuse_val_"+str(socket_id)+".hex", 'rb')
	low_fuse = struct.unpack('B', file.read(1))[0]
	file.close()
	file = open("/tmp/high_fuse_val_"+str(socket_id)+".hex", 'rb')
	high_fuse = struct.unpack('B', file.read(1))[0]
	file.close()
	file = open("/tmp/extended_fuse_val_"+str(socket_id)+".hex", 'rb')
	extended_fuse = struct.unpack('B', file.read(1))[0]
	file.close()
	file = open("/tmp/lock_fuse_val_"+str(socket_id)+".hex", 'rb')
	lock_fuse = struct.unpack('B', file.read(1))[0]
	file.close()
	
	# Print values and delete temporary values
	#print "Low fuse:", hex(low_fuse)
	#print "High fuse:", hex(high_fuse)
	#print "Extended fuse:", hex(extended_fuse)
	#print "Lock fuse:", hex(lock_fuse)
	os.remove("/tmp/low_fuse_val_"+str(socket_id)+".hex")
	os.remove("/tmp/high_fuse_val_"+str(socket_id)+".hex")
	os.remove("/tmp/extended_fuse_val_"+str(socket_id)+".hex")
	os.remove("/tmp/lock_fuse_val_"+str(socket_id)+".hex")
	
	# Check if it wasn't already programmed
	if low_fuse == 0xFF and high_fuse == 0xD8 and extended_fuse == 0xC8 and lock_fuse == 0x3C and False:
		return [False, mooltipass_id, flashFile, EepromFile, "already programmed!", encryptedKeysFile]
		
	# Erase device
	commands.getstatusoutput("avrdude -c avrisp2 -p m32u4 -B 10 -P usb:"+socket_id_to_programmer_serial[socket_id]+" -e")
	
	# Program all fuses except lock fuse
	avrdude_command = "avrdude -c avrisp2 -p m32u4 -B 10 -P usb:"+socket_id_to_programmer_serial[socket_id]+" -U lfuse:w:0xFF:m -U hfuse:w:0xD8:m -U efuse:w:0xC8:m"
	output_avrdude_prog_fuse = commands.getstatusoutput(avrdude_command)
	
	# Program flash & eeprom
	avrdude_command = "avrdude -c avrisp2 -p m32u4 -B 1 -P usb:"+socket_id_to_programmer_serial[socket_id]+" -U flash:w:"+flashFile+":i -U eeprom:w:"+EepromFile+":i"
	output_avrdude_flash = commands.getstatusoutput(avrdude_command)
	
	# Program lock fuse
	avrdude_command = "avrdude -c avrisp2 -p m32u4 -B 1 -P usb:"+socket_id_to_programmer_serial[socket_id]+" -U lock:w:0x3C:m"
	output_avrdude_prog_lock = commands.getstatusoutput(avrdude_command)	
	
	# Return success state
	if "failed" in output_avrdude_prog_fuse or "failed" in output_avrdude_flash or "failed" in output_avrdude_prog_lock:
		return [False, mooltipass_id, flashFile, EepromFile, "couldn't program!", encryptedKeysFile]
	else:
		return [True, mooltipass_id, flashFile, EepromFile, "programmed", encryptedKeysFile]

def main():
	print "Mooltipass Mass Programming Tool"
	
	# Check for public key
	if not os.path.isfile("publickey.bin"):
		print "Couldn't find public key!"
		return
	
	# Check for firmware file presence
	if not os.path.isfile("Mooltipass.hex"):
		print "Couldn't find Mooltipass.hex"
		sys.exit(0)
	
	# Check for bootloader file presence
	if not os.path.isfile("bootloader_mini.hex"):
		print "Couldn't find bootloader_mini.hex"
		sys.exit(0)
	
	# Temp vars
	prog_socket_states = [PROG_SOCKET_IDLE, PROG_SOCKET_IDLE, PROG_SOCKET_IDLE, PROG_SOCKET_IDLE, PROG_SOCKET_IDLE, PROG_SOCKET_IDLE, PROG_SOCKET_IDLE, PROG_SOCKET_IDLE, PROG_SOCKET_IDLE]
	programming_threads = [0, 0, 0, 0, 0, 0, 0, 0, 0]
	next_available_mooltipass_id = 1
	mooltipass_ids_pending = []
	mooltipass_ids_to_take = []
	global displayed_texts
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
	
	# Generate a blank firmware to see if we actually can generate it and then print the hash
	return_gen = generateFlashAndEepromHex("Mooltipass.hex", "bootloader_mini.hex", 12345, [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0], [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0], [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0], [0,0,0,0,0,0], "/tmp/test_flash.hex", "/tmp/test_eeprom.hex", True)
	os.remove("/tmp/test_flash.hex")
	os.remove("/tmp/test_eeprom.hex")
	
	# Check the success status
	if return_gen[0] == False:
		print "Couldn't generate a template flash hex!"
		sys.exit(0)
	
	# Check for random numbers file presence
	if os.path.isfile("rng.bin"):
		random_bytes_buffer = pickle_read("rng.bin")
		
	# Set to true to export the random bytes
	if False:		
		f = open('randombytes.bin', 'wb')
		random_bytes_buffer.tofile(f)
		f.flush()
		f.close()
	
	# Check for next mooltipass id file
	if os.path.isfile("mooltipass_id.bin"):
		next_available_mooltipass_id = pickle_read("mooltipass_id.bin")
		
	# Check for available mooltipass ids file
	if os.path.isfile("mooltipass_av_ids.bin"):
		mooltipass_ids_to_take = pickle_read("mooltipass_av_ids.bin")
		
	# Check for available mooltipass ids file
	if os.path.isfile("mooltipass_pending_ids.bin"):
		mooltipass_ids_pending = pickle_read("mooltipass_pending_ids.bin")
		if len(mooltipass_ids_pending) > 0:
			mooltipass_ids_to_take.extend(mooltipass_ids_pending)
			print "Adding previously pending ids:", mooltipass_ids_pending
			
	# Read public key
	public_key = pickle_read("publickey.bin")
	
	# Display text on screen
	add_line_on_screen(mooltipass_device, "Python Script Started")
	add_line_on_screen(mooltipass_device, return_gen[1][0:20])
	add_line_on_screen(mooltipass_device, return_gen[1][20:])
		
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
			for socket_id in range(0, 9):
				if prog_socket_states[socket_id] == PROG_SOCKET_PENDING:
					print "Starting programming for socket", socket_id
					
					# Generate new mooltipass ID
					if len(mooltipass_ids_to_take) > 0:
						mooltipass_id = mooltipass_ids_to_take[0]
						del(mooltipass_ids_to_take[0])
					else:
						# No ids to take, take the next available one
						mooltipass_id = next_available_mooltipass_id
						next_available_mooltipass_id = next_available_mooltipass_id + 1
						# Store the new id in file
						pickle_write(next_available_mooltipass_id, "mooltipass_id.bin")
					
					# Generate keys from the random bytes buffer
					aes_key1 = random_bytes_buffer[0:AES_KEY_LENGTH]
					aes_key2 = random_bytes_buffer[AES_KEY_LENGTH:AES_KEY_LENGTH+AES_KEY_LENGTH]
					uid_key = random_bytes_buffer[AES_KEY_LENGTH+AES_KEY_LENGTH:AES_KEY_LENGTH+AES_KEY_LENGTH+UID_REQUEST_KEY_LENGTH]
					uid = random_bytes_buffer[AES_KEY_LENGTH+AES_KEY_LENGTH+UID_REQUEST_KEY_LENGTH:AES_KEY_LENGTH+AES_KEY_LENGTH+UID_REQUEST_KEY_LENGTH+UID_KEY_LENGTH]
					del(random_bytes_buffer[0:AES_KEY_LENGTH+AES_KEY_LENGTH+UID_REQUEST_KEY_LENGTH+UID_KEY_LENGTH])
					
					# Write in file: Mooltipass ID | aes key 1 | aes key 2 | request ID key | UID, flush write					
					aes_key1_text =  "".join(format(x, "02x") for x in aes_key1)
					aes_key2_text =  "".join(format(x, "02x") for x in aes_key2)
					uid_key_text = "".join(format(x, "02x") for x in uid_key)
					uid_text = "".join(format(x, "02x") for x in uid)					
					string_export = str(mooltipass_id)+"|"+ aes_key1_text +"|"+ aes_key2_text +"|"+ uid_key_text +"|"+ uid_text+"\r\n"
					#print string_export
					try:
						pickle_file_name = time.strftime("export/%Y-%m-%d-%H-%M-%S-Mooltipass-")+str(mooltipass_id)+".txt"
						pickle_write(seccure.encrypt(string_export, public_key, curve='secp521r1/nistp521'), pickle_file_name)	
					except NameError:
						pickle_file_name = "test"
					
					# Generate programming file					
					generateFlashAndEepromHex("Mooltipass.hex", "bootloader_mini.hex", mooltipass_id, aes_key1, aes_key2, uid_key, uid, "/tmp/flash_"+str(mooltipass_id)+".hex", "/tmp/eeprom_"+str(mooltipass_id)+".hex", True)
					
					# Change state to programming
					prog_socket_states[socket_id] = PROG_SOCKET_PROGRAMMING
					
					# Display info on display
					add_line_on_screen(mooltipass_device, "#"+str(socket_id)+": programming id "+str(mooltipass_id))
					
					# Launch a programming thread
					mooltipass_ids_pending.append(mooltipass_id)
					pickle_write(mooltipass_ids_pending, "mooltipass_pending_ids.bin")
					programming_threads[socket_id] = FuncThread(start_programming, socket_id, mooltipass_id, "/tmp/flash_"+str(mooltipass_id)+".hex", "/tmp/eeprom_"+str(mooltipass_id)+".hex", pickle_file_name)
					programming_threads[socket_id].start()
					
					
		# Check for thread end
		for socket_id in range(0, 9):
			# Check if we're currently programming
			if prog_socket_states[socket_id] == PROG_SOCKET_PROGRAMMING:
				# Check if the thread ended
				if not programming_threads[socket_id].is_alive():
					print "Thread for socket", socket_id, "ended"
					
					# Fetch the return data
					return_data = programming_threads[socket_id].join()
					
					# Delete the temporary programming files
					os.remove(return_data[2])
					os.remove(return_data[3])
					
					# Check success state
					if return_data[0] :
						print "Programming for socket", socket_id, "succeeded (mooltipass id:", str(return_data[1]) + ")"
						
						# Save our mooltipass pool in case it was an id to take
						pickle_write(mooltipass_ids_to_take, "mooltipass_av_ids.bin")
						
						# Inform programming platform of success state
						mooltipass_device.getInternalDevice().sendHidPacket(mooltipass_device.getPacketForCommand(CMD_PROG_DONE, 1, [socket_id]))
						mooltipass_device.getInternalDevice().receiveHidPacketWithTimeout()
						add_line_on_screen(mooltipass_device, "#"+str(socket_id)+": "+return_data[4])
					else:
						print "Programming for socket", socket_id, "failed (mooltipass id:", str(return_data[1]) + ")"
						
						# Put the mooltipass id back in the pool
						mooltipass_ids_to_take.append(return_data[1])
						pickle_write(mooltipass_ids_to_take, "mooltipass_av_ids.bin")
						
						# Delete encrypted keys file
						os.remove(return_data[5])
						
						# Inform programming platform of success state
						mooltipass_device.getInternalDevice().sendHidPacket(mooltipass_device.getPacketForCommand(CMD_PROG_FAILURE, 1, [socket_id]))
						mooltipass_device.getInternalDevice().receiveHidPacketWithTimeout()
						add_line_on_screen(mooltipass_device, "#"+str(socket_id)+": "+return_data[4])
						
					# Remove id from pending ones					
					mooltipass_ids_pending.remove(return_data[1])
					pickle_write(mooltipass_ids_pending, "mooltipass_pending_ids.bin")
					
					# Reset prog state
					prog_socket_states[socket_id] = PROG_SOCKET_IDLE

if __name__ == "__main__":
	main()

	
	
	
	
	
	
	
	
	
	
	
	