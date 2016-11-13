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
#################################################################################################################################
#                                  COMMAND EXAMPLES                                                                             #
#                                                                                                                               #
# Upload new bundle to device: mooltipass_tool.py uploadBundle updatefile.img <password>                                        #
# Generate signed firmware: mooltipass_tool.py packAndSign bundleName firmwareName oldAesKey (newAesKey) updateFileName         #
# Generate and upload signed firmware: mooltipass_tool.py packSignUpload bundleName firmwareName oldAesKey (newAesKey) password #
# Initialize Mooltipass: mooltipass_tool.py init bundleName                                                                     #
# Launch security checks for mini: mooltipass_tool.py minicheck oldfirmware newfirmware bundlename                              #
#                                                                                                                               #
#                                                                                                                               #
#                                                                                                                               #
#                                                                                                                               #
#################################################################################################################################
from mooltipass_mass_prod_init_proc import *
from mooltipass_hid_device import *
from mooltipass_init_proc import *
import mooltipass_security_check 
import firmwareBundlePackAndSign
from datetime import datetime
from array import array
import platform
import usb.core
import usb.util
import random
import time
import sys
nonConnectionCommands = ["packAndSign", "decrypt_mini_prod"]

def main():
	skipConnection = False
	
	# If an arg is supplied and if the command doesn't require to connect to a device
	if len(sys.argv) > 1 and sys.argv[1] in nonConnectionCommands:
		skipConnection = True

	if not skipConnection:
		# HID device constructor
		mooltipass_device = mooltipass_hid_device()	
		
		# Connect to device
		if mooltipass_device.connect(True) == False:
			sys.exit(0)
			
		# Get Mooltipass Version
		version_data = mooltipass_device.getMooltipassVersionAndVariant()
		print "Mooltipass version: " + version_data[1] + ", variant: " + version_data[2] + ", " + str(version_data[0]) + "Mb of Flash"
			
		# Print Mooltipass status
		print "Mooltipass status:", mooltipass_device.getMooltipassStatus()
		print ""
	
	# See if args were passed
	if len(sys.argv) > 1:
		if sys.argv[1] == "uploadBundle":
			# extract args
			if len(sys.argv) > 2:
				filename = sys.argv[2]
			else:
				filename = None
			if len(sys.argv) > 3:
				password = sys.argv[3]
			else:
				password = None
			# start upload
			mooltipass_device.uploadBundle(password, filename, True)
			
		if sys.argv[1] == "packAndSign":
			if len(sys.argv) > 5:
				# Depending on number of args, set a new password or not
				if len(sys.argv) > 6:
					firmwareBundlePackAndSign.bundlePackAndSign(sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5], sys.argv[6], True)
				else:
					firmwareBundlePackAndSign.bundlePackAndSign(sys.argv[2], sys.argv[3], sys.argv[4], None, sys.argv[5], True)					
			else:
				print "packAndSign: not enough args!"
				
		if sys.argv[1] == "packSignUpload":
			if len(sys.argv) > 5:
				# Depending on number of args, set a new password or not
				if len(sys.argv) > 6:
					bundle_gen = firmwareBundlePackAndSign.bundlePackAndSign(sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5], "updatefile.img", True)
				else:
					bundle_gen = firmwareBundlePackAndSign.bundlePackAndSign(sys.argv[2], sys.argv[3], sys.argv[4], None, "updatefile.img", True)
				# Did we generate the bundle?
				if bundle_gen == True:
					if len(sys.argv) > 6:
						mooltipass_device.uploadBundle(sys.argv[6], "updatefile.img", True)
					else:
						mooltipass_device.uploadBundle(sys.argv[5], "updatefile.img", True)
			else:
				print "packAndSign: not enough args!"
				
		if sys.argv[1] == "minicheck":
			if len(sys.argv) > 7:
				mooltipass_security_check.mooltipassMiniSecCheck(mooltipass_device, sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5], sys.argv[6], sys.argv[7], sys.argv[8])
			else:
				print "minicheck: not enough args!"
		
		if sys.argv[1] == "initproc":
			if version_data[2] == "mini":
				mooltipassMiniInit(mooltipass_device)
			else:
				print "Device Not Supported"
				
		if sys.argv[1] == "massprodinit":
			if version_data[2] == "mini":
				mooltipassMiniMassProdInit(mooltipass_device)
			else:
				print "Device Not Supported"			
				
		if sys.argv[1] == "get_serial":
			if version_data[2] == "mini":
				serial_number = mooltipass_device.getMooltipassMiniSerial()
				print "Serial number in hex:", "".join(format(x, "02x") for x in serial_number)
				print "Serial number in decimal:", serial_number[0]*16777216 + serial_number[1]*65536 + serial_number[2]*256 + serial_number[3]*1
			else:
				print "Device Not Supported"		
				
		if sys.argv[1] == "decrypt_mini_prod":
			if len(sys.argv) > 2:
				decryptMiniProdFile(sys.argv[2])		
			else:
				print "decrypt_mini_prod: not enough args!"
				
		if sys.argv[1] == "get_uid":
			if len(sys.argv) > 2:
				uid = mooltipass_device.getUID(sys.argv[2])
				if uid != None:
					print "UID:", "".join(format(x, "02x") for x in uid)
			else:
				print "decrypt_mini_prod: not enough args!"
				
		if sys.argv[1] == "read_user_db_change_nb":
			mooltipass_device.getMooltipassUserDbChangeNumber()		
				
		if sys.argv[1] == "set_user_db_change_nb":
			if len(sys.argv) > 2:
				mooltipass_device.setMooltipassUserDbChangeNumber(int(sys.argv[2]))		
			else:
				print "set_user_db_change_nb: not enough args!"
				
		if sys.argv[1] == "get_free_user_slots":
			mooltipass_device.getFreeUserSlots()
			
		if sys.argv[1] == "change_login_description":
			mooltipass_device.changeLoginDescription()
			
		if sys.argv[1] == "add_credential":
			mooltipass_device.addCredential()
			
		if sys.argv[1] == "test_please_retry":
			mooltipass_device.testPleaseRetry()
			
		if sys.argv[1] == "get_card_cpz":
			mooltipass_device.getCardCpz()
			
		if sys.argv[1] == "lock":
			mooltipass_device.lock()
		
		
	#mooltipass_device.sendCustomPacket()
	#mooltipass_device.checkSecuritySettings()
	#if not skipConnection:
		#device.disconnect()

if __name__ == "__main__":
	main()
