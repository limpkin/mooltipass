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
import md5
from optparse import OptionParser
from struct import *
from array import array

parser = OptionParser(usage = '''usage: %prog [options] bitmap1 bitmap2 font1 bitmap3 font2
    note: a filename that contains word "font" will be stored as a font
          other files are stored as bitmaps''')
parser.add_option('-o', '--output', help='name of output bundle file', dest='output', default='bundle.img')
parser.add_option('-i', '--input', help='name of input bundle file', dest='input', default='')
parser.add_option('-s', '--strings', help='include these strings at head of bundle', dest='strings', default=None)
parser.add_option('-t', '--test', help='On input: list contents of input bundle. On output: Don\'t actually write to disk', dest='test_bundle', action='store_true', default=False)
parser.add_option('-5', '--md5', help='Print md5sum of bundle and each file', dest='show_md5', action='store_true', default=False)

(options, args) = parser.parse_args()

if len(options.input) > 0 and options.strings:
    parser.error("Error: Can't add strings when expanding a bundle")

MEDIA_BITMAP = 1
MEDIA_FONT   = 2
RESERVED_IDS = 64

MEDIA_TYPE_NAMES = {
    MEDIA_BITMAP: 'bmap',
    MEDIA_FONT: 'font',
}

def imageTypeToString(imageType):
    if imageType in MEDIA_TYPE_NAMES:
        return MEDIA_TYPE_NAMES[imageType]
    else:
        return "unkn"

def buildBundle(bundlename, stringFile, files, test_bundle=False, show_md5=False):
    strings = []
    if stringFile:
        with open(stringFile) as fd:
            for line in fd:
                strings.append(line.strip())

    if len(strings) > RESERVED_IDS:
        print 'Error: {} strings is more than the {} supported'.format(len(strings), RESERVED_IDS)
        return

    data = []
    header = array('H')             		# unsigned short array (uint16_t)
    header.append(RESERVED_IDS + len(files))
    reserve = RESERVED_IDS*2 + 2*len(files) + 2      	# leave room for the header
    size = reserve
	
	#temp append while storing the string in flash
    for string,index in zip(strings,range(len(strings))):
        print '    0x{:04x}: size {:4} bytes, string[{}] = "{}"'.format(size,len(string)+1, index, string)
        header.append(size)
        size += len(string) + 1     # +1 for null terminator

    for i in range(len(strings),RESERVED_IDS):
        header.append(0)

    for filename,index in zip(files,range(len(files))):
        fd = open(filename, 'rb')
        image = fd.read()

        imageHash = ''
        if show_md5:
            m = md5.new()
            m.update(image)
            imageHash = m.hexdigest()

        imageType = array('H')
        if 'font' in filename:
            imageType.append(MEDIA_FONT)
            print '    0x{:04x}: size {:4} bytes, font[{}] {} {}'.format(size,len(image)+2, index, filename, imageHash)
        else:
            imageType.append(MEDIA_BITMAP)
            print '    0x{:04x}: size {:4} bytes, bmap[{}] {} {}'.format(size,len(image)+2, index, filename, imageHash)

        header.append(size)
        size += len(image) + 2      # 2 bytes for type prefix
        data.append((filename, imageType,image))
        fd.close()
    print 'total size: {}'.format(size-reserve)

    if not test_bundle:
        print 'Writing to {}'.format(bundlename)
        bfd = open(bundlename,  "wb")
        header.tofile(bfd)
        offset = 0

        # Strings first
        for string in strings:
            bfd.write(string)
            bfd.write(pack('B', 0)) # null terminated

        for filename,imageType,image in data:
            #print '    0x{:04x}: {} {}'.format(offset, imageType, image)
            imageType.tofile(bfd)
            bfd.write(image)
            offset += len(image)+2
        bfd.close()
        if show_md5:
            bfd = open(bundlename,  "rb")
            m = md5.new()
            data = bfd.read(512)
            while len(data) > 0:
                m.update(data)
                data = bfd.read(512)
            print "{} {}".format(bundlename, m.hexdigest())
            bfd.close()
        print 'wrote {} bytes to {}'.format(size-reserve, bundlename)

def expandBundle(bundlename, args, test_bundle=False, show_md5=False):
    bfd = open(bundlename, 'rb')

    if show_md5:
        m = md5.new()
        data = bfd.read(512)
        while len(data) > 0:
            m.update(data)
            data = bfd.read(512)
        print "{} {}".format(bundlename, m.hexdigest())
        bfd.seek(0)


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

        if show_md5:
            m = md5.new()
            m.update(imageData)
            imageHash = m.hexdigest()
        else:
            imageHash = ''

        print '    0x{:04x}: size {} bytes, {} {} {}'.format(imageBegin, imageLen, imageTypeToString(imageType), imageName, imageHash)
        if not test_bundle:
            fd = open(imageName, 'wb')
            fd.write(imageData)
            fd.close
        imageIndex += 1

    bfd.close()

def sortObjects(a,b):
    if '_' in a and '_' in b:
        ai = int(a.split('_')[0])
        bi = int(b.split('_')[0])
        if (ai < bi):
            return -1
        elif (ai == bi):
            return 0
        else:
            return 1
    else:
        return cmp(a,b)

def main():
    args.sort(cmp=sortObjects);
    if len(options.input) > 0:
        expandBundle(options.output, args, test_bundle=options.test_bundle, show_md5=options.show_md5)
    else:
        buildBundle(options.output, options.strings, args, test_bundle=options.test_bundle, show_md5=options.show_md5)

if __name__ == "__main__":
    main()
