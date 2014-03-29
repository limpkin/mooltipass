#!/usr/bin/python
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


import qrcode, sys
from qrcode.image.pure import PymagingImage
from optparse import OptionParser

factories = {
    'pil': 'qrcode.image.pil.PilImage',
    'pymaging': 'qrcode.image.pure.PymagingImage',
    'svg': 'qrcode.image.svg.SvgImage',
    'svg-fragment': 'qrcode.image.svg.SvgFragmentImage',
    'svg-path': 'qrcode.image.svg.SvgPathImage',
}

# (version, size, bits, numerics, alhpanumerics, binary)
qrVersion = [
    ( 1,  21,   152,   41,  25,    17),
    ( 2,  25,   272,   77,  47,    32),
    ( 3,  28,   440,  127,  77,    53),
    ( 4,  33,   640,  187,  114,   78),
    ( 5,  37,   864,  255,  154,  106),
    ( 6,  41,  1088,  322,  195,  134),
    ( 7,  45,  1248,  370,  224,  154),
    ( 8,  49,  1552,  461,  279,  192),
    ( 9,  53,  1856,  552,  335,  230),
    (10,  57,  2192,  652,  395,  271),
    (11,  61,  2592,  772,  468,  321),
    (12,  65,  2960,  883,  535,  367),
    (13,  69,  3424, 1022,  619,  425),
    (14,  73,  3688, 1101,  667,  458),
    (15,  77,  4184, 1250,  758,  520),
    (16,  81,  4712, 1408,  854,  586),
    (17,  85,  5176, 1548,  938,  644),
    (18,  89,  5768, 1725, 1046,  718),
    (19,  93,  6360, 1903, 1153,  792),
    (20,  97,  6888, 2061, 1249,  858),
    (21, 101,  7456, 2232, 1352,  929),
    (22, 105,  8048, 2409, 1460, 1003),
    (23, 109,  8752, 2620, 1588, 1091),
    (24, 113,  9392, 2812, 1704, 1171),
    (25, 117, 10208, 3057, 1853, 1273),
    (26, 121, 10960, 3283, 1990, 1367),
    (27, 125, 11744, 3514, 2132, 1465),
    (28, 129, 12248, 3669, 2223, 1528),
    (29, 133, 13048, 3909, 2369, 1628),
    (30, 137, 13880, 4158, 2520, 1732),
    (31, 141, 14744, 4417, 2677, 1840),
    (32, 145, 15640, 4686, 2840, 1952),
    (33, 149, 16568, 4965, 3009, 2068),
    (34, 153, 17528, 5253, 3183, 2188),
    (35, 157, 18448, 5529, 3351, 2303),
    (36, 161, 19472, 5836, 3537, 2431),
    (37, 165, 20528, 6153, 3729, 2563),
    (38, 169, 21616, 6479, 3927, 2699),
    (39, 173, 22496, 6743, 4087, 2809),
    (40, 177, 23648, 7089, 4296, 2953)]

parser = OptionParser(usage = 'usage: %prog [options]')
parser.add_option('-v', '--version', help='QR version 1-{} (default 2: 25x25)'.format(len(qrVersion)),
                  dest='version', default=2)
parser.add_option('', '--boxsize', help='width data box in pixels (default: 2)', type='int', dest='boxsize', default=2)
parser.add_option('-b', '--border', help='width of border in boxes (default: 3)', type='int', dest='border', default=2)
parser.add_option('-o', '--output', help='output png filename (default: qr.png)', type='string', dest='output', default='qr.png')
(options, args) = parser.parse_args()

if options.version < 1 or options.version > len(qrVersion):
    parser.error('version must be a value from 1 to {}'.format(len(qrVersion)))

def main():
    # Version 2 with L error correction supports
    #    - 272 bits of data, 77 numeric, or 47 alphanumeric
    qr = qrcode.QRCode( version=options.version,
        error_correction=qrcode.constants.ERROR_CORRECT_L,
        box_size=options.boxsize, border=options.border)

    data = ' '.join(args)

    ver,size,bits,numerals,alphanumerals,binary = qrVersion[options.version-1]

    if data.isdigit():
        print 'data is numeric'
        if len(data) > numerals:
            parser.error('numeric string size {} is greater than version {} limit of {}'.format(len(data),ver,numerals))
        limit = numerals
    else:
        print 'data is alphanumeric'
        if len(data) > alphanumerals:
            parser.error('alphanumeric string size {} is greater than version {} limit of {}'.format(len(data),ver,alphanumerals))
        limit = alphanumerals

    print '"{}"'.format(data)
    print 'length {}, limit is {} characters'.format(len(data),limit)

    qr.add_data(data)
    qr.make(fit=True)

    module = factories.get('pymaging', 'pymaging')
    if '.' not in module:
        parser.error("The image factory is not a full python path")
    module, name = module.rsplit('.', 1)
    imp = __import__(module, {}, [], [name])
    factory = getattr(imp, name)

    img = qr.make_image(image_factory=factory)
    fd = open(options.output, 'w')
    img.save(fd)
    fd.close()

if __name__ == "__main__":
    main()
