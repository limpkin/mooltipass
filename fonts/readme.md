Converting TTF to PNG & XML
===========================

Pre-reqs:
 * qt4

Build font builder:
```sh
$ git submodule update
$ cd fonts/fontbuilder
$ qmake
$ make
$ ./bin/FontBuilder
```

Font tab:
* Set "Fonts directory" to fonts/ttf
* Set "Family", "Style" and "Size"
* Set "DPI" to 72
* Choose "Smoothing" or not depending on preference/size/use

Characters tab:
* Click "Import from file ..." and choose fonts/characters.txt

Output tab:
* Set output directory to fonts/build
* Set format to "png" (not "PNG")
* Set format to "Divo compatible - xml"
* Click "Write font"

Converting PNG & XML to .h and .img
===================================

Pre-reqs:
 * python2-pypng
 * python2-numpy

Invoke font.py:
```sh
$ python2 ./font.py -n <font name> -o <font file name> -p build/<font file name>.png -x build/<font file name>.xml -d1 
```
