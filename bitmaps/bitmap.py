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

import sys
import png, math
import numpy as np
from optparse import OptionParser
from struct import *

parser = OptionParser(usage = 'usage: %prog [options]')
parser.add_option('-n', '--name', help='name for bitmap', dest='name', default=None)
parser.add_option('-o', '--output', help='name of output file', dest='output', default=None)
parser.add_option('-i', '--input', help='input header file', dest='input', default=None)
parser.add_option('-c', '--compress', help='compress output', action='store_true', dest='compress', default=False)
parser.add_option('-b', '--bitdepth', help='number of bits per pixel (default: 4)', type='int', dest='bitdepth', default=4)
(options, args) = parser.parse_args()

if options.name == None or options.input == None:
    parser.error('name and input options are required')

def generateCompressedHeader(output, imageName, imageFile, bitDepth=4, wordSize=16):
    ''' Output C code tables for the specified image
        Use RLE to reduce the size of the bitmap
    '''

    fd = open(imageFile, 'r')

    if output:
        outfd = open(output, 'w')
    else:
        outfd = sys.stdout

    flags = 1

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

    # arange in lines
    data = np.array(data).reshape(height,width)

    depth = data.max()+1
    scale = 2**bitDepth / float(depth)

    lineSize = (width * bitDepth) / wordSize
    rem = (width * bitDepth) - (lineSize * wordSize)

    # see if the data fits fully in the number of words
    if (rem > 0): lineSize += 1

    dataSize = int(math.ceil(width * bitDepth * height / float(wordSize)))
    pixPerWord = float(wordSize) / bitDepth

    print >> outfd, '/*'
    print >> outfd, ' * bitmap {}'.format(imageName)
    print >> outfd, ' */'
    print >> outfd
    print >> outfd, '#define {}_WIDTH {}'.format(imageName.upper(), width)
    print >> outfd, '#define {}_HEIGHT {}'.format(imageName.upper(), height)
    print >> outfd, ''
    print >> outfd, 'const struct {'
    print >> outfd, '    uint16_t width;'
    print >> outfd, '    uint8_t height;'
    print >> outfd, '    uint8_t depth;'
    print >> outfd, '    uint8_t flags;'
    print >> outfd, '    uint16_t dataSize;'
    count = 0
    pixels = 0
    bitCount = wordSize
    runCount = 0
    runPixel = 0

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

    print >> outfd, '    uint16_t data[{}];'.format(len(output)/2)
    print >> outfd, '}} image_{} __attribute__((__progmem__)) = {{'.format(imageName)
    print >> outfd, '    {0}_WIDTH, {0}_HEIGHT, {1}, {2}, {3},'.format(imageName.upper(), bitDepth, flags, count)
    print >> outfd, '    {',
    for ind in xrange(0,len(output)/2):
        if ind % 8 == 0:
            print >> outfd, ''
            print >> outfd, '    ',
        if output[ind*2] > 255:
            print 'output[{}] {} > 255'.format(ind*2, output[ind*2])
        if output[ind*2+1] > 255:
            print 'output[{}] {} > 255'.format(ind*2+1, output[ind*2+1])
        print >> outfd, '0x{:04x}, '.format(output[ind*2] | output[ind*2+1]<<8),
    print >> outfd, '    }'
    print >> outfd, '};'

    print 'count: ',count

    bfd = open('{}.img'.format(options.output), "wb")
    header = pack('=HBBBH', width, height, bitDepth, flags, count)
    bfd.write(header)
    binaryData = bytearray(output)
    bfd.write(binaryData)
    bfd.close()
    print 'write {}.img'.format(options.output)

def generateHeader(output, imageName, imageFile, bitDepth=4, wordSize=16):
    ''' Output C code tables for the specified image
    '''

    fd = open(imageFile, 'r')

    if output:
        outfd = open(output, 'w')
    else:
        outfd = sys.stdout

    flags = 0

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

    # arange in lines
    data = np.array(data).reshape(height,width)

    depth = data.max()+1
    scale = 2**bitDepth / float(depth)

    lineSize = (width * bitDepth) / wordSize
    rem = (width * bitDepth) - (lineSize * wordSize)

    # see if the data fits fully in the number of words
    if (rem > 0): lineSize += 1

    dataSize = int(math.ceil(width * bitDepth * height / float(wordSize)))
    pixPerWord = float(wordSize) / bitDepth

    print >> outfd, '/*'
    print >> outfd, ' * bitmap {}'.format(imageName)
    print >> outfd, ' */'
    print >> outfd
    print >> outfd, '#define {}_WIDTH {}'.format(imageName.upper(), width)
    print >> outfd, '#define {}_HEIGHT {}'.format(imageName.upper(), height)
    print >> outfd, ''
    print >> outfd, 'const struct {'
    print >> outfd, '    uint16_t width;'
    print >> outfd, '    uint8_t height;'
    print >> outfd, '    uint8_t depth;'
    print >> outfd, '    uint8_t flags;'
    print >> outfd, '    uint16_t dataSize;'
    print >> outfd, '    uint16_t data[{}];'.format(dataSize)
    print >> outfd, '}} image_{} __attribute__((__progmem__)) = {{'.format(imageName)
    print >> outfd, '    {0}_WIDTH, {0}_HEIGHT, {1}, {2}, {3},'.format(imageName.upper(), bitDepth, flags, dataSize)
    print >> outfd, '    {'
    count = 0
    pixels = 0
    bitCount = wordSize
    lineNum = 0
    for line in data:
        line = line * scale
        print >> outfd, '   /* {} */'.format(lineNum),
        lineNum += 1
        ind = 0
        for pix in line:
            if bitCount < bitDepth:
                lastPixels = pixels
                pixels |= int(pix) >> (bitDepth - bitCount)
                print >> outfd, '0x{:04x}, '.format(int(pixels)),
                count += 1
                ind += 1
                if ind >= 8:
                    print >> outfd, '\n   ',
                    ind = 0
                bitCount = wordSize - (bitDepth - bitCount)
                if int(pix) > 15:
                    print >> sys.stderr, 'ERROR: pix = {}'.format(pix)
                pixels = (int(pix) << bitCount) & 0xFFFF
            else:
                bitCount -= bitDepth
                pixels |= int(pix) << bitCount
                if bitCount == 0:
                    bitCount = wordSize
                    print >> outfd, '0x{:04x}, '.format(int(pixels)),
                    count += 1
                    pixels = 0
                    ind += 1
                    if ind >= 8:
                        print >> outfd, '\n   ',
                        ind = 0
        print >> outfd
    if bitCount != 16:
        print 'bitCount {} pixels {:04x}'.format(bitCount, pixels)
        pixels = pixels << bitCount
        print >> outfd, '0x{:04x}, '.format(int(pixels)),
        count += 1
    print >> outfd, '    }'
    print >> outfd, '};'

    print 'count: ',count

    if output:
        outfd.close()

def main():

    if options.compress:
        generateCompressedHeader(options.output, options.name, options.input, options.bitdepth)
    else:
        generateHeader(options.output, options.name, options.input, options.bitdepth)

if __name__ == "__main__":
    main()
