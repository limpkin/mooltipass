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

import os
import sys
import png, math
import numpy as np
from optparse import OptionParser
from struct import *

parser = OptionParser(usage = 'usage: %prog [options]')
parser.add_option('-o', '--output', help='name of output file', dest='output', default=None)
parser.add_option('-i', '--input', help='input header file', dest='input', default=None)
(options, args) = parser.parse_args()

if options.output == None or options.input == None:
    parser.error('input and output options are required')

def compressImage(image):
    data = image['data']
    width = image['width']
    height = image['height']
    
    # change array format to 2D array, rotate it 90degrees as we scanning starts from the bot left to top left, etc...
    #print data
    data = np.array(data).reshape(height,width)
    #print ""
    #print data
    data = np.rot90(data, 3)
    #print ""
    #print data

    depth = data.max()+1
    if (depth > 2):
        print "Wrong bit depth!"

    pixel_data = 0
    bitCount = 7
    output = []
    for line in data:
        bitCount = 7
        pixel_data = 0
        for pix in line:
            pixel_data |= int(pix) << bitCount
            bitCount-=1
            if bitCount < 0:
                output.append(pixel_data)
                pixel_data = 0
                bitCount = 7
        if bitCount != 7:
            output.append(pixel_data)
    
    #print ""
    #print output

    image['data'] = pack('<{}B'.format(len(output)), *output) # little endian, byte packed uint8_t
    image['flags'] = 0      # uncompressed
    image['depth'] = 1

    return image

def parseGimpHeader(filename):
    ''' Parse GIMP H file image into array
    '''
    if (filename == "-"):
        fd = sys.stdin
    else:
        fd = open(filename, 'r');

    # locate width and height
    # then import all pixel data
    init = True
    init2 = True
    data = []
    mapping = []
    for line in fd:
        if init:
            if 'width' in line:
                width = int(line.split('=')[-1].strip()[:-1])
            elif 'height' in line:
                height = int(line.split('=')[-1].strip()[:-1])
            elif 'header_data_cmap[256][3]' in line:
                print "found cmap header"
                init = False
        elif init2:
            if 'header_data[]' in line:
                init2 = False
            elif '};' in line:
                continue
            else:
                mapping.append(int(line.replace("{", "").replace("}", "").strip().split(',')[0]))
        else:
            if '};' in line:
                break
            data.extend(line.split(','))

    data = [int(round(float(mapping[int(x.strip())])/255*1)) for x in data if x.strip().isdigit()]
    #print data
    if len(data) == 0:
        print 'Failed to extract pixel data from {}'.format(imageFile)
        sys.exit(1)

    dataSizeBytes = len(data)

    image = {
        'data': data,
        'dataSizeBytes': dataSizeBytes,
        'width': width,
        'height': height,
        }

    return image


def writeImage(filename, image):
    flags = image['flags']
    data = image['data']

    dataSize = len(data)

    if (filename == "-"):
        fd = sys.stdout
    else:
        fd = open(filename, 'wb');

    # Write header
    fd.write(pack('=HBBBH', image['width'], image['height'], image['depth'], flags, dataSize))
    # Write data
    fd.write(data)
    fd.close()


def main():
    if (options.input == options.output):
        raise Exception("input and output must not be the same")

    image = parseGimpHeader(options.input)
    origSize = image['dataSizeBytes']
    print "Parsed header: {}x{}".format(image['width'], image['height'])
    
    image = compressImage(image)
    print "Compressed image: {} -{}bit-> {} bytes".format(origSize, 1, len(image['data']))

    writeImage(options.output, image)
    print "Wrote {}".format(options.output)


if __name__ == "__main__":
    main()
