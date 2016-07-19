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
###############################################################################################################################
#                                  COMMAND EXAMPLES                                                                           #
#                                                                                                                             #
# Upload new bundle to device: mooltipass_tool.py uploadBundle updatefile.img <password>                                      #
# Generate signed firmware: mooltipass_tool.py packAndSign bundleName firmwareName oldAesKey newAesKey updateFileName         #
# Generate and upload signed firmware: mooltipass_tool.py packSignUpload bundleName firmwareName oldAesKey newAesKey password #
#                                                                                                                             #
#                                                                                                                             #
#                                                                                                                             #
#                                                                                                                             #
###############################################################################################################################
from mooltipass_hid_device import *
import firmwareBundlePackAndSign
from datetime import datetime
from array import array
import platform
import usb.core
import usb.util
import random
import time
import sys
nonConnectionCommands = ["packAndSign"]

def main():
	skipConnection = False
	
	# If an arg is supplied and if the command doesn't require to connect to a device
	if len(sys.argv) > 1 and sys.argv[1] in nonConnectionCommands:
		skipConnection = True

	if not skipConnection:
		# HID device constructor
		mooltipass_device = mooltipass_hid_device()	
		
		# Connect to device
		if mooltipass_device.connect() == False:
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
			custom_hid_packet.uploadBundle(device, password, filename)
		if sys.argv[1] == "packAndSign":
			if len(sys.argv) > 6:
				firmwareBundlePackAndSign.bundlePackAndSign(sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5], sys.argv[6])
			else:
				print "packAndSign: not enough args!"	
		if sys.argv[1] == "packSignUpload":
			if len(sys.argv) > 6:
				if firmwareBundlePackAndSign.bundlePackAndSign(sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5], "updatefile.img") == True:
					mooltipass_device.uploadBundle(sys.argv[6], "updatefile.img")
			else:
				print "packAndSign: not enough args!"
			
			
		
	#mooltipass_device.sendCustomPacket()
	#mooltipass_device.checkSecuritySettings()
	#if not skipConnection:
		#device.disconnect()

if __name__ == "__main__":
	main()
