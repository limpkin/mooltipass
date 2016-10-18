#!/usr/bin/env python2
#
# Copyright (c) 2016 Mathieu Stpehan
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
from struct import *
import random
import time
import sys

USB_VID					= 0x16D0
USB_PID					= 0x09A0

LEN_INDEX				= 0x00
CMD_INDEX				= 0x01
DATA_INDEX				= 0x02
PREV_ADDRESS_INDEX		= 0x02
NEXT_ADDRESS_INDEX		= 0x04
NEXT_CHILD_INDEX		= 0x06
SERVICE_INDEX			= 0x08
DESC_INDEX				= 6
LOGIN_INDEX				= 37
NODE_SIZE				= 132

CMD_PING				= 0xA1
CMD_MINI_FRAME_BUF_DATA = 0x9E

parser = OptionParser(usage = 'usage: %prog [options]')
parser.add_option('-i', '--input', help='image to send to the screen', dest='input', default=None)
parser.add_option('-r', '--reverse', help='set to inverse pixel data', action="store_true", dest='reverse', default=False)
(options, args) = parser.parse_args()

if options.input == None:
	parser.error('input option is required')
	
def main():
		
	# Open image and convert it to monochrome
	image = Image.open(options.input)
	image = image.convert(mode="RGB", colors=1)	
	
	# Get image specs
	img_format = image.format
	img_size = image.size
	img_mode = image.mode
	print "Format:", img_format, "size:", img_size, "mode:", img_mode
	
	# Check size
	if img_size[0] > 128 or img_size[1] > 32:
		print "Wrong dimensions or format!"
		sys.exit(0)
		
	# Compute size	
	dataSize = img_size[0] * ((img_size[1]+7) / 8)
	print "Total data size:", dataSize, "bytes"
		
	# Turn image left 90 degrees
	image_rot = image.transpose(Image.ROTATE_270)
	#image_rot.save("lapin.jpg", "JPEG")
	#print image_rot.size[0]
	#print image_rot.size[1]
	
	# Process the pixels
	pixel_data = 0
	bitCount = 7
	bitstream = []
	for y in range(0, image_rot.size[1]):
		for page in range(0, (image_rot.size[0]+7)/8):
			bitCount = 7
			pixel_data = 0
			for x in range(page*8, page*8 + 8):
				# Get RGB values
				if(x >= image_rot.size[0]):
					rgb = [0,0,0]
				else:
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
	
	# Open file to write
	fd = open(options.input.split('.')[0]+".img", 'wb');

	# Write header
	fd.write(pack('=HBBBH', img_size[0], img_size[1], 1, 0, dataSize))
	# Write data
	fd.write(pack('<{}B'.format(len(bitstream)), *bitstream))
	fd.close()
	
	print "File written"

if __name__ == "__main__":
	main()
