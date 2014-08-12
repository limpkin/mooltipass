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


#
# Bundle a collection of bitmaps into a flat image with indices that can
# be imported into the SPI flash media region on the mooltipass
#
# Note: fonts are expected to have the word "font" in their filename
#

import os
import sys
from optparse import OptionParser
from struct import *
from array import array

parser = OptionParser(usage = '''usage: %prog [options] bitmap1 bitmap2 font1 bitmap3 font2
    note: a filename that contains word "font" will be stored as a font
          other files are stored as bitmaps''')
parser.add_option('-o', '--output', help='name of output bundle file', dest='output', default='bundle.img')
parser.add_option('-i', '--input', help='name of input bundle file', dest='input', default='')
parser.add_option('-t', '--test', help='On input: list contents of input bundle. On output: Don\'t actually write to disk', dest='test_bundle', action='store_true', default=False)

(options, args) = parser.parse_args()

MEDIA_BITMAP = 1
MEDIA_FONT   = 2

def imageTypeToString(imageType):
    if imageType == MEDIA_BITMAP:
        return "bmap"
    elif imageType == MEDIA_FONT:
        return "font"
    else:
        return "unkn"

def buildBundle(bundlename, files):
    data = []
    header = array('H')             # unsigned short array (uint16_t)
    header.append(len(files))
    reserve = 2*len(files) + 2      # leave room for the header
    size = reserve

    for filename in files:
        fd = open(filename, 'rb')
        image = fd.read()

        imageType = array('H')
        if 'font' in filename:
            imageType.append(MEDIA_FONT)
            print '    0x{:04x}: size {} bytes, font {}'.format(size,len(image)+2, filename)
        else:
            imageType.append(MEDIA_BITMAP)
            print '    0x{:04x}: size {} bytes, bmap {}'.format(size,len(image)+2, filename)

        header.append(size)
        size += len(image) + 2      # 2 bytes for type prefix
        data.append((filename, imageType,image))
        fd.close()
    print 'total size: {}'.format(size-reserve)

    print 'Writing to {}'.format(bundlename)
    bfd = open(bundlename,  "wb")
    header.tofile(bfd)
    offset = 0
    for filename,imageType,image in data:
        #print '    0x{:04x}: {} {}'.format(offset, imageType, image)
        imageType.tofile(bfd)
        bfd.write(image)
        offset += len(image)+2
    bfd.close()
    print 'wrote {} bytes to {}'.format(size-reserve, bundlename)

def expandBundle(bundlename, args, test_bundle=False):
    bfd = open(bundlename, 'rb')

    file_count = unpack('H', bfd.read(2))[0]
    file_offsets = array('H')
    file_offsets.fromfile(bfd, file_count)
    bundle_len = os.path.getsize(bundlename)
    file_offsets.append(bundle_len)

    imageIndex = 0
    for o in range(len(file_offsets)-1):
        imageBegin = file_offsets[o]
        imageEnd = file_offsets[o+1]
        imageLen = imageEnd - imageBegin

        bfd.seek(file_offsets[o])
        imageType = unpack('H', bfd.read(2))[0]
        imageData = bfd.read(imageLen - 2)
        imageName = '{}_{}.img'.format(imageIndex, imageTypeToString(imageType))

        print '    0x{:04x}: size {} bytes, {} {}'.format(imageBegin, imageLen, imageTypeToString(imageType), imageName)
        if not test_bundle:
            fd = open(imageName, 'wb')
            fd.write(imageData)
            fd.close
        imageIndex += 1

    bfd.close()

def main():
    if len(options.input) > 0:
        expandBundle(options.output, args, test_bundle=options.test_bundle)
    else:
        buildBundle(options.output, args)

if __name__ == "__main__":
    main()
