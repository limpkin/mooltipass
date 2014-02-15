#ifndef BITMAP_H_
#define BITMAP_H_

typedef struct bitmap_s {
    uint8_t width;	//*< width of image in pixels
    uint8_t height;	//*< height of image in pixels
    uint8_t depth;	//*< Number of bits per pixel
    uint16_t dataSize;  //*< number of words in data
    uint16_t data[];	//*< pointer to the image data
} bitmap_t;


#endif
