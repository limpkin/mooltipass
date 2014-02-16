#! /usr/bin/python

import sys
import struct

infilename = sys.argv[1]
outfilename = sys.argv[2]

infp = open(infilename, "rb")
outfp = open(outfilename,"w")

for data in infp:
    bad = 0
    for c in data:
        tst = ord(c)
        if tst == 10:
            if bad == 0:
                outfp.write(data)
                bad = 0
        if tst < 48 or tst > 57:
            bad = 1
outfp.close()

