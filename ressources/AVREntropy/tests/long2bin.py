#! /usr/bin/python

import sys
import struct

try:
    infilename = sys.argv[1]
    outfilename = sys.argv[2]
    maxsize = int(sys.argv[3])
except:
    print "ERROR! ./long2bin.py infile (txt) outfile (binary) maximum_size"

infp = open(infilename,"r")
outfp = open(outfilename,"wb")
count = 0
while (count < maxsize):
    line = infp.readline()
    if not line:
        break
    value = int(line)
    count += 1
    outfp.write(struct.pack('=L',value))
outfp.close()
infp.close()

