#ifndef FILEIO_PNGIMAGE_H
#define FILEIO_PNGIMAGE_H

#include <vector>
#include <stdint.h>

void png_version_info(void);

std::vector<uint8_t> readPNG(const char *fname, int& width, int& height);
void writePNG(const char *iname, int width, int height, const void* data); 

#endif
