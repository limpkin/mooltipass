from mooltipass_hid_device import *
from mooltipass_defines import *
from os.path import isfile, join
from array import array
from time import sleep
from os import listdir
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
if platform.system() == "Linux":
	# Barcode printing only implemented for linux
	from png_labels import create_label_type1, create_label_type2
	PRINTER_MODEL = "QL-700"
	import brother_ql	
	import logging
	logging.basicConfig(level='ERROR')


def create_raster_file(label_size, in_file, out_file, cut=True):
	qlr = brother_ql.BrotherQLRaster(PRINTER_MODEL)
	brother_ql.create_label(qlr, in_file, label_size, cut=cut)
	with open(out_file, 'wb') as f:
		f.write(qlr.data)
		
def getMpmColorForSerialNumber(serial_number):
	# To be implemented later
	return ["Purple", "PUR"]
	return ["Red", "RED"]
		
# Get a packet to send for a given command and payload
def mpmMassProdInitGetPacketForCommand(cmd, len, data):
	# data to send
	arraytosend = array('B')

	# if command copy it otherwise copy the data
	if cmd != 0:
		arraytosend.append(len)
		arraytosend.append(cmd)

	# add the data
	if data is not None:
		arraytosend.extend(data)
		
	return arraytosend
	
# Check print status
def checkPrintStatus():
	CHECK_MAX_SECONDS = 4
	data_received        = False
	print_successful     = False
	end_of_media_reached = False

	dev = os.open("/dev/usb/lp0", os.O_RDWR)
	try:
		start = time.time()
		while time.time() - start < CHECK_MAX_SECONDS:
			data = os.read(dev, 32)
			if len(data) == 32:
				data_received = True
				error_bytes = ord(data[8]), ord(data[9])
				if (error_bytes[0] & 0x2) or (error_bytes[1] & 0x40):
					end_of_media_reached = True
					break
				if ord(data[18]) == 0x06 and ord(data[19]) == 0x00:
					print_successful = True
					break
	except KeyboardInterrupt:
		pass
	finally:
		os.close(dev)
	
	# Check for print success
	if print_successful == True:
		print "Print successful"
	elif end_of_media_reached == True:
		print ""
		print "|!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!|"
		print "|---------------------------------------------------------|"
		print "|---------------------------------------------------------|"                  
		print "|                 CHANGE PRINTER LABEL ROLL!!!!           |"                     
		print "|---------------------------------------------------------|"
		print "|---------------------------------------------------------|"
		print "|!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!|"
		raw_input("Press enter once done:")
							
def mooltipassMiniMassProdInit(mooltipass_device):		
	# Check for update bundle
	if not os.path.isfile("updatefile.img"):
		print "Couldn't find data file!"
		return		

	# Loop
	try:
		temp_bool = 0
		while temp_bool == 0:
			# Operation success state
			success_status = True
			
			# Get serial number
			mooltipass_device.getInternalDevice().sendHidPacket([0, CMD_GET_MINI_SERIAL])
			serial_number = mooltipass_device.getInternalDevice().receiveHidPacketWithTimeout()
			if serial_number != None:
				serial_number = serial_number[DATA_INDEX+0]*16777216 + serial_number[DATA_INDEX+1]*65536 + serial_number[DATA_INDEX+2]*256 + serial_number[DATA_INDEX+3]*1
				sys.stdout.write("MPM-"+str(serial_number)+" found... ")
				sys.stdout.flush()
			else:
				success_status = False

			# Send our bundle
			if success_status == True:
				sys.stdout.write('Uploading graphics... ')
				sys.stdout.flush()
				
				# Upload bundle, password is not used in that context
				success_status = mooltipass_device.uploadBundle("00000000000000000000000000000000", "updatefile.img", False)
				
				# For the mini version this procedure doesn't check the last return packet because in normal mode the device reboots
				if success_status == True:
					if mooltipass_device.getInternalDevice().receiveHidPacketWithTimeout()[DATA_INDEX] == 0x01:
						success_status = True
					else:
						success_status = False
						print "last packet fail!!!"
						print "likely causes: problem with external flash"
				else:
					success_status = False
					print "fail!!!"
					print "likely causes: problem with external flash"

			# Inform the Mooltipass that the bundle is sent so it can start functional test
			if success_status == True:
				magic_key = array('B')
				magic_key.append(0)
				magic_key.append(187)
				mooltipass_device.getInternalDevice().sendHidPacket(mpmMassProdInitGetPacketForCommand(CMD_SET_MOOLTIPASS_PARM, 2, magic_key))
				if mooltipass_device.getInternalDevice().receiveHidPacket()[DATA_INDEX] == 0x01:
					success_status = True
					print ""
				else:
					success_status = False
					print "fail!!!"
					print "likely causes: none"

			# Wait for the mooltipass to inform the script that the test was successfull
			if success_status == True:
				temp_bool2 = False
				sys.stdout.write('Please follow the instructions on the mooltipass screen... ')
				sys.stdout.flush()
				while temp_bool2 != True:
					test_result = mooltipass_device.getInternalDevice().receiveHidPacketWithTimeout()
					if test_result == None:
						sys.stdout.write('.')
						sys.stdout.flush()
					else:
						if test_result[CMD_INDEX] == CMD_FUNCTIONAL_TEST_RES and test_result[DATA_INDEX] == 0:
							success_status = True
							print " ok!"
						else:
							success_status = False
							print " fail!!!"
							print "Please look at the screen to know the cause"
						temp_bool2 = True

			if success_status == True:
				if platform.system() == "Linux":
					sys.stdout.write('printing label 1... ')
					sys.stdout.flush()
					
					# 17*87mm label size
					label_size = "17x87"
					# Bar code value: MPM - Color - Serial
					barcode_value = "MPM-"+getMpmColorForSerialNumber(serial_number)[1]+"-"+str(serial_number).zfill(5)
					# Text: Mooltipass Mini / Color: XXX / Serial number: XXXX
					line1, line2, line3 = "Mooltipass Mini", "Color: "+getMpmColorForSerialNumber(serial_number)[0], "Serial Number: "+str(serial_number).zfill(5)
					out_file = "label_number_1.bin"
					# Create label with content
					im = create_label_type1(label_size, barcode_value, line1, line2, line3)
					create_raster_file(label_size, im, out_file, cut=False)
					# Use cat to print label
					os.system("cat "+out_file+" > /dev/usb/lp0")
					
					# Check print status
					checkPrintStatus()
					
					sys.stdout.write('printing label 2... ')
					sys.stdout.flush()
					# 17*87mm label size, text value: MPM - v1 - 8Mb - Color		
					label_size, text = "17x87", "MPM-v1-8Mb-"+getMpmColorForSerialNumber(serial_number)[1]
					out_file = "label_number_2.bin"
					# Create label with content
					im = create_label_type2(label_size, text, font_size=16)
					create_raster_file(label_size, im, out_file, cut=True)
					# Use cat to print label
					os.system("cat "+out_file+" > /dev/usb/lp0")
					
					# Check print status
					checkPrintStatus()
				
				# Let the user know it is done
				print "Setting up Mooltipass MPM-"+str(serial_number).zfill(4)+" DONE"
			else:
				print ""
				print "|!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!|"
				print "|---------------------------------------------------------|"
				print "|---------------------------------------------------------|"
				print "| Setting up Mooltipass MPM-"+str(serial_number).zfill(4)+" FAILED                  |"
				print "|                                                         |"                     
				print "|           PLEASE PUT AWAY THIS MOOLTIPASS!!!!           |"                     
				print "|---------------------------------------------------------|"
				print "|---------------------------------------------------------|"
				print "|!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!|"

			# Disconnect this device
			print "\r\nPlease disconnect this Mooltipass"

			# Wait for no answer to ping
			temp_bool2 = 0
			while temp_bool2 == 0:
				try :
					# Send ping packet
					mooltipass_device.pingMooltipass()
				except usb.core.USBError as e:
					#print e
					temp_bool2 = 1
				time.sleep(.5)

			# Connect another device
			print "Connect other Mooltipass"

			# Wait for findHidDevice to return something
			temp_bool2 = False;
			while temp_bool2 == False:
				temp_bool2 = mooltipass_device.connect(False)
				time.sleep(.5)

			# Delay
			time.sleep(1)

			# New Mooltipass detected
			print "New Mooltipass detected"
			print ""
	except KeyboardInterrupt:
		print "File written, everything ok"