#!/usr/bin/env python
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
from custom_hid_device import *
import custom_hid_packet
from datetime import datetime
from array import array
import platform
import usb.core
import usb.util
import random
import time
import sys

def main():
	# HID device constructor
	device = custom_hid_device()	
	
	# Connect to device
	if device.connect(True, custom_hid_packet.createPingPacket(), custom_hid_packet.checkPingAnswerPacket) == False:
		sys.exit(0)
		
	# Get Mooltipass Version
	version_data = custom_hid_packet.getMooltipassVersionAndVariant(device)
	print "Mooltipass version: " + version_data[1] + ", variant: " + version_data[2] + ", " + str(version_data[0]) + "Mb of Flash"
		
	# Print Mooltipass status
	print "Mooltipass status:", custom_hid_packet.getMooltipassStatus(device)
	print ""
	
	# See if args were passed
	if len(sys.argv) > 1:
		if sys.argv[1] == "uploadBundle":
			custom_hid_packet.uploadBundle(device, None)
			
	custom_hid_packet.checkSecuritySettings(device)
	#device.disconnect()

if __name__ == "__main__":
	main()
