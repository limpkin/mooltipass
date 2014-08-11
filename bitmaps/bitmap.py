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
parser.add_option('-n', '--name', help='name for bitmap', dest='name', default=None)
parser.add_option('-o', '--output', help='name of output file (.img produces binary blob, .h produces optimized header)', dest='output', default=None)
parser.add_option('-i', '--input', help='input header file', dest='input', default=None)
parser.add_option('-c', '--compress', help='compress output', action='store_true', dest='compress', default=False)
(options, args) = parser.parse_args()

if options.name == None or options.input == None:
    parser.error('name and input options are required')

def compressImage(image):
    ''' Takes unpacked indexed data and produces 4-bit packed data
    '''
    data = image['data']
    width = image['width']
    height = image['height']
    pixel = 0

    # arange in lines
    data = np.array(data).reshape(height,width)

    depth = data.max()+1
    if (depth > 16):
        print "Warning: Image has more colors than bit depth can accurately encode"
    scale = 16 / float(depth)

    output = []
    count = 0
    for line in data:
        line = line * scale
        for pix in line:
            if count & 0x01 == 0:
                pixel = (int(pix) << 4) | 0xF0
            else:
                pixel |= (int(pix) & 0x0F)
                output.append(pixel);
            count += 1

    if len(output) & 0x01 != 0:
        output.append(0)

    image['data'] = output
    image['dataSizeBytes'] = len(output)
    image['flags'] = 0

    return image

def compressImageRLE(image):
    ''' Takes unpacked indexed data and produces 4-bit RLE compressed data
    '''
    count = 0
    pixels = 0
    runCount = 0
    runPixel = 0
    data = image['data']
    width = image['width']
    height = image['height']

    # arange in lines
    data = np.array(data).reshape(height,width)

    depth = data.max()+1
    if (depth > 16):
        print "Warning: Image has more colors than bit depth can accurately encode"
    scale = 16 / float(depth)

    output = []
    for line in data:
        line = line * scale
        ind = 0
        for pix in line:
            if runCount == 0:
                runPixel = pix
                runCount = 1
                continue
            if pix != runPixel:
                output.append((runCount-1) << 4 | int(runPixel))
                runPixel = pix
                runCount = 1
                count += 1
            else:
                runCount += 1
                if runCount > 16:
                    output.append(0xF0 | int(runPixel))
                    runCount = 1
                    count += 1
    if runCount != 0:
        output.append((runCount-1) << 4 | int(runPixel))
        runCount = 0
        count += 1

    if len(output) & 0x01 != 0:
        output.append(0)

    image['data'] = output
    image['dataSizeBytes'] = count
    image['flags'] = 1

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
    data = []
    for line in fd:
        if init:
            if 'width' in line:
                width = int(line.split('=')[-1].strip()[:-1])
            elif 'height' in line:
                height = int(line.split('=')[-1].strip()[:-1])
            elif 'header_data[]' in line:
                init = False
        else:
            if '};' in line:
                break
            data.extend(line.split(','))

    data = [int(x.strip()) for x in data if x.strip().isdigit()]
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

    dataSize = image['dataSizeBytes']
    if flags == 0:
        # When just bitpacked, dataSize is in words
        if dataSize & 0x01 != 0:
            raise Exception("dataSize is not word aligned");
        dataSize /= 2
    # When RLE compressed, dataSize is in bytes

    if (filename == "-"):
        fd = sys.stdout
    else:
        fd = open(filename, 'wb');

    # Write header
    fd.write(pack('=HBBBH', image['width'], image['height'], 4, flags, dataSize))
    # Write data
    fd.write(bytearray(data))
    fd.close()


def writeMooltipassHeader(filename, imageName, image):
    width = image['width']
    height = image['height']
    data = image['data']
    flags = image['flags']
    imageNameUpper = imageName.upper()

    dataSize = image['dataSizeBytes']
    dataSizeWords = dataSize / 2
    if flags == 0:
        # When just bitpacked, dataSize is in words
        dataSize = dataSizeWords
    # When RLE compressed, dataSize is in bytes

    if dataSize & 0x01 != 0:
        raise Exception("dataSize is not word aligned");

    if (filename == "-"):
        fd = sys.stdout
    else:
        fd = open(filename, 'w');

    print >> fd, '/*'
    print >> fd, ' * bitmap {}'.format(imageName)
    print >> fd, ' */'
    print >> fd
    print >> fd, '#define {}_WIDTH {}'.format(imageNameUpper, width)
    print >> fd, '#define {}_HEIGHT {}'.format(imageNameUpper, height)
    print >> fd, ''
    print >> fd, 'const struct {'
    print >> fd, '    uint16_t width;'
    print >> fd, '    uint8_t height;'
    print >> fd, '    uint8_t depth;'
    print >> fd, '    uint8_t flags;'
    print >> fd, '    uint16_t dataSize;'
    print >> fd, '    uint16_t data[{}];'.format(dataSizeWords)
    print >> fd, '}} image_{} __attribute__((__progmem__)) = {{'.format(imageName)
    print >> fd, '    {0}_WIDTH, {0}_HEIGHT, {1}, {2}, {3},'.format(imageNameUpper, 4, flags, dataSize)
    print >> fd, '    {',
    for ind in xrange(0,len(data)/2):
        if ind % 8 == 0:
            print >> fd, ''
            print >> fd, '    ',
        if data[ind*2] > 255:
            print 'data[{}] {} > 255'.format(ind*2, data[ind*2])
        if data[ind*2+1] > 255:
            print 'data[{}] {} > 255'.format(ind*2+1, data[ind*2+1])
        print >> fd, '0x{:04x}, '.format(data[ind*2] | data[ind*2+1]<<8),
    print >> fd, '    }'
    print >> fd, '};'


def main():
    if (options.input == options.output):
        raise Exception("input and output must not be the same")

    image = parseGimpHeader(options.input)
    origSize = image['dataSizeBytes']
    print "Parsed header: {}x{}".format(image['width'], image['height'])


    if options.compress:
        image = compressImageRLE(image)
        compSize = image['dataSizeBytes']
        print "Compressed image: {} -RLE-> {} bytes".format(
            origSize,
            compSize)
    else:
        image = compressImage(image)
        compSize = image['dataSizeBytes']
        print "Compressed image: {} -4bit-> {} bytes".format(
            origSize,
            compSize)

    unused, outputExtension = os.path.splitext(options.output)
    if outputExtension == ".img":
        writeImage(options.output, image)
    elif outputExtension == ".h":
        writeMooltipassHeader(options.output, options.name, image)
    print "Wrote {}".format(options.output)


if __name__ == "__main__":
    main()
