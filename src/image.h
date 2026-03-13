#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>

// Currently unimplemented new interface for TGA parsing

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} PIXEL_DATA;

typedef struct {
    int height;
    int width;
    PIXEL_DATA* data;
} IMAGE;

enum {
    TGA = 0
} IMAGE_FORMAT;

IMAGE* new_image_tga(const void* bytestream);
IMAGE* new_image_from_file(char* filename);

void destroy_image(IMAGE* image);

#endif

