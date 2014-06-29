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

import sys
from optparse import OptionParser
from struct import *
from array import array

parser = OptionParser(usage = 'usage: %prog [options] bitmap1 bitmap2 bitmap3')
parser.add_option('-o', '--output', help='name of output file', dest='output', default='bundle.img')
(options, args) = parser.parse_args()

def buildBundle(bundlename, files):

    data = []
    header = array('I')
    header.append(len(files))
    size = 4*len(files) + 4     # leave room for the header

    for filename in files:
        fd = open(filename, 'rb')
        image = fd.read()
        header.append(size)
        size += len(image)
        data.append(image)
        fd.close()
    print 'total size: {}'.format(size)

    bfd = open(bundlename,  "wb")
    header.tofile(bfd)
    for image in data:
        bfd.write(image)
    bfd.close()
    print 'wrote {} bytes to {}'.format(size, bundlename)

def main():

    buildBundle(options.output, args)

if __name__ == "__main__":
    main()
