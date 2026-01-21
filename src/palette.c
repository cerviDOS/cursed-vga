#include <stdlib.h>
#include <stdio.h>
#include "palette.h"

void gp_color_cube(PALETTE* palette)
{
    PIXEL* data = palette->data;

    int curr_idx = 0;

    int red_val = 0;
    int red_incr = 0x5f;

    do {
        int green_val = 0;
        int green_incr = 0x5f;
        do {
            int blue_val = 0;
            int blue_incr = 0x5f;

            do {
                data[curr_idx++] = (PIXEL) {
                    .blue_val = blue_val,
                    .green_val = green_val,
                    .red_val = red_val
                };

                blue_val += blue_incr;
                blue_incr = 0x28;

            } while (blue_val < 256);

            green_val += green_incr;
            green_incr = 0x28;
        } while (green_val < 256);

        red_val += red_incr;
        red_incr = 0x28;
    } while (red_val < 256);
}

// TODO: move these utility functions to their own file
// also, modifying the amount bit shifted could be a
// TUI element in the future
int pixels_equal(PIXEL a, PIXEL b, int8_t bitshift)
{
    return a.red_val >> bitshift == b.red_val >> bitshift
    && a.green_val >> bitshift == b.green_val >> bitshift
    && a.blue_val >> bitshift == b.blue_val >> bitshift;
}

// returns -1 if not inserted
int put_if_absent(PALETTE* palette, PIXEL new_pixel, int num_elems)
{
    for (int index = 0; index < num_elems; index++) {
        if (pixels_equal(new_pixel, palette->data[index], 4)) {
            return -1;
        }
    }
    palette->data[num_elems+1] = new_pixel;
    return 0;
}

void gp_first_colors(PALETTE* palette, const IMAGE* image)
{
    int total_pixels = image->header.height * image->header.width;
    int num_elems = 0;
    for (int img_index = 0; img_index < total_pixels && num_elems < palette->size; img_index++) {
        PIXEL p = image->data[img_index];
        if (put_if_absent(palette, p, num_elems) != -1) {
            num_elems++;
        }
    }
}

typedef struct {
    uint16_t hex;
    int frequency;
} COLOR_FREQ;

int compare_color_frequency_descending(const void* a, const void* b)
{
    COLOR_FREQ* freq_a = (COLOR_FREQ*) a;
    COLOR_FREQ* freq_b = (COLOR_FREQ*) b;

    return freq_b->frequency - freq_a->frequency;
}

void gp_popularity(PALETTE* palette, const IMAGE* image)
{
    const int FREQ_SIZE = UINT16_MAX+1;
    COLOR_FREQ frequency[FREQ_SIZE];

    // Init frequency array
    for (int i = 0; i < FREQ_SIZE; i++) {
        frequency[i].hex = i; // Hex will align with index before the final sort
        frequency[i].frequency = 0;
    }

    const int TOTAL_PIXELS = image->header.height * image->header.width;
    for (int img_index = 0; img_index < TOTAL_PIXELS; img_index++) {
        PIXEL curr_pixel = image->data[img_index];

        // Bitshift right first to chop off the bottom 3
        // least significant bits of each channel
        uint16_t color_hex = (curr_pixel.red_val >> 3) << 10;
        color_hex |= (curr_pixel.green_val >> 3) << 5;
        color_hex |= (curr_pixel.blue_val >> 3);

        frequency[color_hex].frequency++;
    }

    qsort(&frequency, FREQ_SIZE, sizeof(COLOR_FREQ), compare_color_frequency_descending);

    const double COLOR_SCALE = 255.0 / 31.0;
    for (int index = 0; index < palette->size; index++) {
        // Multiply by COLOR_SCALE to expand range from 0-31 back to 0-255
        palette->data[index].red_val = ((frequency[index].hex & 0x7C00) >> 10) * COLOR_SCALE;
        palette->data[index].green_val = ((frequency[index].hex & 0x03E0) >> 5) * COLOR_SCALE;
        palette->data[index].blue_val = (frequency[index].hex & 0x001F) * COLOR_SCALE;
    }
}

void generate_palette(PALETTE* palette, const IMAGE* image, enum PALETTE_SIZE size, enum GENERATION_METHOD gen_method)
{
    palette->size = size;
    palette->data = malloc(size * sizeof(PIXEL));

    switch (gen_method) {
        case COLOR_CUBE:
            gp_color_cube(palette);
            break;
        case FIRST_COLORS_FOUND:
            gp_first_colors(palette, image);
            break;
        case POPULARITY:
            gp_popularity(palette, image);
            break;
        default:
            break;
    }
}
