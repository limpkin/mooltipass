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
                          name of output file
    -i INPUT, --input=INPUT
                          input header file
    -c, --compress        compress output
    -b BITDEPTH, --bitdepth=BITDEPTH
                          number of bits per pixel (default: 4)
```

###Example conversion of a 256x64 image with 16 colours per pixel (4-bit pixels)
```
./bitmap.py --input HaD_Mooltipass.h --output bitmap.h --name had_mooltipass --bitdepth 4
count:  4096
```

The count: 4096 is the number of 16-bit words used by the uncompressed bitmap image; 8KB in this case. The output file is bitmap.h, and the bitmap data structure in the file is called image_had_mooltipass (from the --name option).

###Example with run-length compression:

Use gimp to convert this:
![alt text](https://github.com/harlequin-tech/mooltipass/raw/master/bitmaps/HaD_Mooltipass.png "Hackaday Mooltipass Logo") into [HaD_Mooltipass.h](https://github.com/harlequin-tech/mooltipass/blob/master/bitmaps/HaD_Mooltipass.h).

Then use the [bitmap.py](https://github.com/harlequin-tech/mooltipass/blob/master/bitmaps/bitmap.py) python script to convert [HaD_Mooltipass.h](https://github.com/harlequin-tech/mooltipass/blob/master/bitmaps/HaD_Mooltipass.h) into [had_mooltipass.h](https://github.com/harlequin-tech/mooltipass/blob/master/source_code/src/had_mooltipass.h)
```
./bitmap.py --input HaD_Mooltipass.h --output had_mooltipass.h --name HaD_Mooltipass --bitdepth 4 --compress
count:  3840
```


In this case the count is the number of bytes used by the compressed bit image; 3.8KB. The amount of compression is very dependent on the image and how many horizontal runs of the same pixel are in the image. The best possible case would be a 8 to 1 compression. Compressed bitmaps also have the benefit of taking less time to display on the screen.
