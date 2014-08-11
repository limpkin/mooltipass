#Creating bitmap headers for Mooltipass

The bitmap format supported by the OLEDMP library in the Mooltipass project supports bitmaps up to 256x64 pixels in size and 1, 2, 3, or 4 bits per pixel (greyscale).

To format a bitmap for use in the mooltipass project, you can use the <a href="http://www.gimp.org">GIMP GNU Image Manipulation Program</a> (available for Linux, OSX, Windows, etc).  

1. Scale or crop the image so it fits within the 256x64 size limit or to the size you want it to be.
2. Convert the image to grayscale (image->mode->grayscale)
3. Convert the image to an indexed image (image->mode->indexed) and select either 2, 4, 8, or 16 colour levels (1-bit pixels, 2-bit, 3-bit, or 4-bit pixels).  
4. Export it as a C header file (file->export, then select the file type as a C header file).

This header file is then used as the input to bitmap.py to produce the final header file for use in the Mooltipass project.

```
./bitmap.py --help
Usage: bitmap.py [options]

Options:
  -h, --help            show this help message and exit
  -n NAME, --name=NAME  name for bitmap
  -o OUTPUT, --output=OUTPUT
                        name of output file (.img produces binary blob, .h
                        produces optimized header)
  -i INPUT, --input=INPUT
                        input header file
  -c, --compress        compress output
  -b BITDEPTH, --bitdepth=BITDEPTH
                        number of bits per pixel (default: 4)
```

###Example conversion of a 256x64 image with 16 colours per pixel (4-bit pixels)
```
./bitmap.py -i 0_HaD_Mooltipass.h -o 0_HaD_Mooltipass_4bit.h -n 0_HaD_Mooltipass
Parsed header: 256x64
Image size: 4096 words
Wrote 0_HaD_Mooltipass_4bit.h
```

The output file is 0_HaD_Mooltipass_4bit.h, and the bitmap data structure in the file is called 0_HaD_Mooltipass (from the --name option).

###Example with run-length compression:

Use gimp to convert this:
![alt text](https://github.com/limpkin/mooltipass/raw/master/bitmaps/HaD_Mooltipass.png "Hackaday Mooltipass Logo") into [HaD_Mooltipass.h](https://github.com/limpkin/mooltipass/blob/master/bitmaps/HaD_Mooltipass.h).

Then use the [bitmap.py](https://github.com/limpkin/mooltipass/blob/master/bitmaps/bitmap.py) python script to convert [HaD_Mooltipass.h](https://github.com/limpkin/mooltipass/blob/master/bitmaps/HaD_Mooltipass.h) into [had_mooltipass.h](https://github.com/limpkin/mooltipass/blob/master/source_code/src/had_mooltipass.h)
```
./bitmap.py -i 0_HaD_Mooltipass.h -o 0_HaD_Mooltipass_4bit.h -n 0_HaD_Mooltipass -c
Parsed header: 256x64
Compressed image: 4096 -> 3124 words
Wrote 0_HaD_Mooltipass_4bit.h
```

In this case the count is the number of bytes used by the compressed bit image; 3.8KB. The amount of compression is very dependent on the image and how many horizontal runs of the same pixel are in the image. The best possible case would be a 8 to 1 compression. Compressed bitmaps also have the benefit of taking less time to display on the screen.

###Example with run-length compression and outputing to binary blob
```
./bitmap.py -i 0_HaD_Mooltipass.h -o 0_HaD_Mooltipass.img -n 0_HaD_Mooltipass -c
Parsed header: 256x64
Compressed image: 4096 -> 3124 words
Wrote 0_HaD_Mooltipass.img
```

In this case, the output file is a binary blob suitable for passing to bundle.py

#Creating a bundle of bitmaps and fonts to use on the Mooltipass

The OLEDMP library supports bitmaps and fonts stored in the Mooltipass's SPI FLASH.  There is about 32KB of storage available for this purpose.

The bundle.py python script takes a list of bitmap and font image filenames on its command line and generates a single image that
can be loaded onto the mooltipass using the Chrome plugin GUI.

```
./bundle --help
Usage: bundle.py [options] bitmap1 bitmap2 font1 bitmap3 font2
    note: a filename that contains word "font" will be stored as a font
          other files are stored as bitmaps

Options:
  -h, --help            show this help message and exit
  -o OUTPUT, --output=OUTPUT
                        name of output file
```

If the filename of the input image has the word "font" in it then it will be treated as a font, otherwise the input will be treated as a bitmap.

##Example

Using wildcards to specify all of the image files starting with the digits 0 through 8:

```
./bundle.py -o bundle.img [0-8]*.img
    0x0448: size 3849 bytes, bmap 0_HaD_Mooltipass.img
    0x1351: size 1553 bytes, bmap 1_login_select.img
    0x1962: size 333 bytes, bmap 2_left.img
    0x1aaf: size 333 bytes, bmap 3_right.img
    0x1bfc: size 83 bytes, bmap 4_tick.img
    0x1c4f: size 149 bytes, bmap 5_cross.img
    0x1ce4: size 149 bytes, bmap 6_cross.img
    0x1d79: size 2380 bytes, font 7_font_profont_10.img
    0x26c5: size 3324 bytes, font 8_font_checkbook_14.img
total size: 12153
Writing to bundle.img
wrote 12153 bytes to bundle.img
```

Combining two images and a font into a bundle:

```
./bundle.py -o mybundle.img 0_HaD_Mooltipass.img 7_font_profont_10.img 5_cross.img 
    0x0430: size 3849 bytes, bmap 0_HaD_Mooltipass.img
    0x1339: size 2380 bytes, font 7_font_profont_10.img
    0x1c85: size 149 bytes, bmap 5_cross.img
total size: 6378
Writing to mybundle.img
wrote 6378 bytes to mybundle.img
```

