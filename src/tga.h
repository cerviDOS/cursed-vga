#ifndef TGA_H
#define TGA_H

#include <stdint.h>

/*
 * Header of a TARGA file in order of each field
 */
typedef struct __attribute__((packed)) {
    uint8_t id_len;
    uint8_t color_map_type;
    uint8_t image_type;
    uint16_t color_map_origin;
    uint16_t color_map_len;
    uint8_t color_map_entry_size;
    uint16_t x_orig;
    uint16_t y_orig;
    uint16_t width;
    uint16_t height;
    uint8_t pixel_depth;
    uint8_t img_desc_byte;
} TARGA_HEADER;

/*
 * RGB values of a single pixel in BGR order to match
 * order and bits used by a TARGA file using 24-bit color.
 */
typedef struct __attribute__((packed)) {
    uint8_t blue_val;
    uint8_t green_val;
    uint8_t red_val;
} PIXEL;

/*
 * Complete image data of a TARGA file wrapped under
 * a single struct
 */
typedef struct {
    TARGA_HEADER* header;
    PIXEL* data;
} IMAGE;

/*
 * Enums for the two types of pixel packet present in a TARGA file
 */
enum PACKET_TYPE {
    RAW,
    RUNLEN
};

static const char* const TGA_SIGNATURE = "TRUEVISION-XFILE";

// Parses and returns an array of pixels from the bytestream.
//
// Requires that the bytestream be a byte-by-byte representation of a TGA image.
// Requires that the bytestream be positioned at the very start of the TGA image.
PIXEL* parse_tga(const void* bytestream);

// Parses and returns a pointer to the TGA header.
//
// Requires that the bytestream be a byte-by-byte representation of a TGA image.
// Requires that the bytestream be positioned at the very start of the TGA image.
TARGA_HEADER* parse_header(const void* bytestream);

// Frees the memory associated with the provided image
void destroy_tga(IMAGE* img);

#endif
