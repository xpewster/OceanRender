#ifndef FILEIO_IMAGES_H
#define FILEIO_IMAGES_H

#include <vector>
#include <stdint.h>

/*
 * Improved readBMP/writeBMP.
 * Automatically detects extensions and read/write the data.
 * Currently supports: bmp, png
 * 
 */
extern std::vector<uint8_t> readImage(const char *fname, int& width, int& height);
extern void writeImage(const char *iname, int width, int height, const void *data); 

#endif
