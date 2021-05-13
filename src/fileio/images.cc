#include "images.h"
#include "bitmap.h"
#include "pngimage.h"
#include <string>
#if defined(_MSC_VER)
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#else
#include <strings.h>
#endif
#include <iostream>

using std::string;

namespace {
bool cicmp(const string& lhs, const string& rhs)
{
	return strcasecmp(lhs.c_str(), rhs.c_str()) == 0;
}

struct Backend {
	const char* ext;
	std::vector<uint8_t> (*reader)(const char *fname, int& width, int& height);
	void (*writer)(const char *iname, int width, int height, const void *data);
};

Backend backends[] = {
	{".bmp", readBMP, writeBMP},
	{".png", readPNG, writePNG},
};

const Backend* bmp_handler = &backends[0];

const Backend* find_handler(const char* fname)
{
	string filename(fname);
	int start = (int) filename.find_last_of('.');
	int end = (int) filename.size() - 1;
	if (start < 0 || start >= end)
		return NULL;
	string ext = filename.substr(start, end);
	for (size_t i = 0; i < sizeof(backends)/sizeof(backends[0]); i++) {
		if (cicmp(ext, backends[i].ext))
			return &backends[i];
	}
	return NULL;
}

};

std::vector<uint8_t> readImage(const char *fname, int& width, int& height)
{
	auto handler = find_handler(fname);
	if (!handler)
		return std::vector<uint8_t>();
	return handler->reader(fname, width, height);
}

void writeImage(const char *fname, int width, int height, const void* data)
{
	auto handler = find_handler(fname);
	if (!handler) {
		std::cerr << "Unrecognized extension for file " << fname
			<< ", writing bmp format" << std::endl;
		handler = bmp_handler;
	}
	handler->writer(fname, width, height, data);
}
