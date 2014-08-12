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
#/

import sys
import png, math
import xml.etree.ElementTree as ET
import numpy as np
from optparse import OptionParser
from struct import *

parser = OptionParser(usage = 'usage: %prog [options]')
parser.add_option('-n', '--name', help='name for font', dest='name', default=None)
parser.add_option('-p', '--png', help='png file for font', dest='png', default=None)
parser.add_option('-x', '--xml', help='xml file for font', dest='xml', default=None)
parser.add_option('-o', '--output', help='name of output file', dest='output', default='font')
parser.add_option('-d', '--debug', help='enable debug output', action='store_true', dest='debug', default=False)
(options, args) = parser.parse_args()

CHAR_EURO = 0x20ac      # Euro currency sign, not yet supported

if options.name == None or options.png == None or options.xml == None:
    parser.error('name, png, and xml options are required')

extendedChars = {
    0xB0: 'Degree Sign',
    0xE0: '`a - Latin Small Letter A with Grave',
    0xE1: '\'a - Latin Small Letter A with Acute',
    0xE2: '^a - Latin Small Letter A with Circumflex',
    0xE3: '~a - Latin Small Letter A with Tilde',
    0xE4: '"a - Latin Small Letter A with Diaeresis',
    0xE5: ' a - Latin Small Letter A with Ring Above',
    0xE6: 'ae - Latin Small Letter Ae',
    0xE7: ' c - Latin Small Letter c with Cedilla',
    0xE8: '`e - Latin Small Letter E with Grave',
    0xE9: '\'e - Latin Small Letter E with Acute',
    0xEA: '^e - Latin Small Letter E with Circumflex',
    0xEB: '"e - Latin Small Letter E with Diaeresis',
    0x20AC: ' E - Euro Sign'
}

def generateHeader(fontName, pngFilename, xmlFilename):
    ''' Output C code tables for the specified font
    '''
    font = png.Reader(pngFilename)
    xml = ET.parse(xmlFilename)

    chars = xml.findall('Char')
    root = xml.getroot()
    glyphd = {}
    maxWidth = 0
    for char in chars:
        glyphd[ord(char.attrib['code'])] = char.attrib
        if int(char.attrib['width']) > maxWidth:
            maxWidth = int(char.attrib['width'])

    width, height, pixels, meta = font.asDirect()

    outfd = open('{}.h'.format(options.output), 'w')
    print >> outfd, '/*'
    print >> outfd, ' * Font {}'.format(fontName)
    print >> outfd, ' */'
    print >> outfd, ''
    print >> outfd, '#define {}_HEIGHT {}'.format(fontName.upper(),root.attrib['height'])
    print >> outfd, ''

    yind = 0
    line = {}
    for item in pixels:
        line[yind] = []
        for xind in xrange(0,len(item),4):
            #line[yind].append(np.reshape(item, (128,4)))
            line[yind].append(dict(r=item[xind+0], g=item[xind+1], b=item[xind+2], a=item[xind+3]))
        yind += 1

    asciiPixel = [' ', '.', '*', '*',
                  '*', '*', '*', '*',
                  '*', '*', '*', '*',
                  '*', '*', '*', '*']
    count = 0
    glyphData = {}
    for ch in sorted(glyphd.keys()):
        if ch == ord(' '):
            # skip space
            continue
        glyph = glyphd[ch]
        glyphData[ch] = []
        rect = [int(x) for x in glyph['rect'].split()]
        offset = [int(x) for x in glyph['offset'].split()]
        print >> outfd, 'const uint8_t {}_{:#x}[] __attribute__((__progmem__)) = {{'.format(fontName,ord(glyph['code'])),
        try:
            print >> outfd, "  /* '{0}' width: {1} */".format(glyph['code'],glyph['width'])
        except:
            print >> outfd, "  /* '?' width: {} */".format(glyph['width'])
        x = 0
        for y in range(rect[1],rect[1]+rect[3]):
            patt = ''
            print >> outfd, '    ',
            lineWidth = 1
            pixels = 0
            pixCount = 0
            for x in range(rect[0],rect[0]+rect[2]-1):
                count += 1
                # map 255 shades to 4
                pix = line[y][x]['a']/64
                pixels = pixels << 2 | pix
                pixCount += 1
                if pixCount >= 4:
                    lineWidth += 1
                    print >> outfd, '0x{:02x}, '.format(pixels),
                    glyphData[ch].append(pixels & 0xFF)
                    #glyphData[ch].append((pixels >> 8) & 0xFF)
                    pixels = 0
                    pixCount = 0

                patt += asciiPixel[pix]

            count += 1
            pix = line[y][x+1]['a']/64
            patt += asciiPixel[pix]
            pixels = pixels << 2 | pix
            pixCount += 1
            if pixCount < 4:
                pixels = pixels << (4-pixCount)*2
            print >> outfd, '0x{:02x}, '.format(pixels),
            glyphData[ch].append(pixels & 0xFF)
            #glyphData[ch].append((pixels >> 8) & 0xFF)
            if lineWidth < (maxWidth+3)/4:
                for ind in range(lineWidth, (maxWidth+3)/4):
                    print >> outfd, '      ',
            print >> outfd, ' /* [{}] */'.format(patt)
        print >> outfd, '};\n'
    print >> outfd, ''

    # font header:
    #
    #    uint8_t height;         //*< height of font
    #    uint8_t fixedWidth;     //*< width of font, 0 = proportional font
    #    uint8_t depth;          //*< Number of bits per pixel
    #    const uint8_t *map;     //*< ASCII to font map
    #    union
    #    {
    #        const glyph_t *glyphs;   //*< variable width font data
    #        const uint8_t *bitmaps;  //*< fixed width font data
    #    } fontData;

    bfd = open('{}.img'.format(options.output), "wb")

    #
    # binary header
    #
    fixedWidth = 0
    depth = 2

    glyphCount = len(glyphData)
    if ord(' ') in glyphd:
        glyphCount += 1   # add 1 for space character

    # Can't handle 2 byte characters yet, so skip them
    for char in glyphData.keys():
        if char > 254:
            print 'skipping extended character 0x{:x}'.format(char)
            glyphCount -= 1

    print '{} glyphs'.format(glyphCount)
    header = pack('=BBBB', int(root.attrib['height']), fixedWidth, depth, glyphCount)
    bfd.write(header)

    if options.debug:
        print 'XXX header: {}'.format(['0x{:02x}'.format(item) for item in bytearray(header)])

    # map
    index = 0
    chMap = []
    glyphHeader = {}
    glyphOffset = 0
    glyphHeaderStr = ''
    for ch in range(ord(' '), 255):
        if glyphd.has_key(ch):
            chMap.append(index)
            index += 1
            glyph = glyphd[ch]
            rect = [int(x) for x in glyph['rect'].split()]
            offset = [int(x) for x in glyph['offset'].split()]
            if ch == ord(' '):
                glyphHeaderStr += "    {{ {:>2}, {:>2}, {:>2}, {:>2}, {:>2}, -1 }}, /* '{}' */\n".format(glyph['width'], 
                    rect[2], rect[3], offset[0], offset[1], glyph['code'])
                glyphHeader[ch] = pack('=BBBbbH', int(glyph['width']), rect[2], rect[3], offset[0], offset[1], 0xFFFF)
            else:
                if ch > 127:
                    if extendedChars.has_key(ch):
                        char = extendedChars[ch]
                    else:
                        char = '~'
                else:
                    char = chr(ch)
                glyphHeaderStr += "    {{ {:>2}, {:>2}, {:>2}, {:>2}, {:>2}, {}_{:#x} }}, /* '{}' */\n".format(glyph['width'], 
                    rect[2], rect[3], offset[0], offset[1], fontName, ch, char)
                glyphHeader[ch] = pack('=BBBbbH', int(glyph['width']), rect[2], rect[3], offset[0], offset[1], glyphOffset)
                glyphOffset += len(glyphData[ch])
        else:
            chMap.append(None)

    glyphMap = []

    print >> outfd, '/* Mapping from ASCII codes to font characters, from space (0x20) to del (0x7f) */'
    print >> outfd, 'const uint8_t {}_asciimap[{}] __attribute__((__progmem__)) = {{ '.format(fontName,len(chMap)),
    index = 0
    for item in chMap:
        if index == 0:
            print >> outfd, '\n    ',
            index = 16
        if item != None:
            glyphMap.append(item)
            print >> outfd, '{:>3}, '.format(item),
        else:
            glyphMap.append(255)
            print >> outfd, '255, ',
        index -= 1
    print >> outfd, '};\n'

    if len(chMap) < 256:
        glyphMap.extend([255 for i in range(0, 256-len(chMap))])


    #
    # binary ASCII map table
    #
    binaryData = bytearray(glyphMap)
    bfd.write(binaryData)
    if options.debug:
        print "XXX glyphMap: {}".format(['0x{:02x}'.format(item) for item in bytearray(glyphMap)])

    #
    # binary glyph header data
    #
    index = 0
    for ch in range(ord(' '), 255):
        if glyphd.has_key(ch):
            bfd.write(glyphHeader[ch])
            if options.debug:
                if ch < 127:
                    print "XXX '{}' hdr[{}]: {}".format(chr(ch), index, ['0x{:02x}'.format(item) for item in bytearray(glyphHeader[ch])])
                else:
                    print "XXX {} hdr[{}]: {}".format(ch, index, ['0x{:02x}'.format(item) for item in bytearray(glyphHeader[ch])])
            index += 1

    # glyph_t

    print >> outfd, 'const glyph_t {}[] __attribute__((__progmem__)) = {{'.format(fontName)

    print >> outfd, glyphHeaderStr,

    print >> outfd, '};\n'

    #
    # binary glyph data
    #
    offset = 0
    for ch in sorted(glyphd.keys()):
        if ch == ord(' '):
            # skip space
            continue
        # convert glyph data to uint16_t packed data
        binaryData = bytearray(glyphData[ch])
        bfd.write(binaryData)
        if options.debug:
            if ch < 127:
                print "XXX 0x{:04x} '{}' data: {}".format(offset,chr(ch),['0x{:02x}'.format(item) for item in bytearray(glyphData[ch])])
            else:
                print "XXX 0x{:04x} {} data: {}".format(offset,ch,['0x{:02x}'.format(item) for item in bytearray(glyphData[ch])])
        offset += len(glyphData[ch])

    bfd.close()
    outfd.close()
    print >> sys.stderr, 'wrote header font to {}.h'.format(options.output)
    print >> sys.stderr, 'wrote binary font to {}.img'.format(options.output)
    return count

def main():
    generateHeader(options.name, options.png, options.xml)

if __name__ == "__main__":
    main()
