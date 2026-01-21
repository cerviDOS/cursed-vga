#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "UI.h"
#include "palette.h"
#include "tga.h"

// TODO: move these functions into a util file
int is_whitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\12';
}

char* ltrim(char* str)
{
    while (*str && is_whitespace(*str)) str++;
    return str;
}

char* rtrim(char* str)
{
    char* end = str + strlen(str);
    while(*str && is_whitespace(*--end));
    *(end+1) = '\0';
    return str;
}

char* trim(char* str)
{
    return rtrim(ltrim(str));
}

IMAGE* try_read_image(const char* filepath)
{
    if (access(filepath, F_OK) != 0) {
        return NULL;
    }

    FILE* file = fopen(filepath, "r");

    fseek(file, 0, SEEK_END);
    int filesize = ftell(file);
    rewind(file);

    uint8_t* bytestream = malloc(filesize);
    fread(bytestream, sizeof(uint8_t), filesize, file);
    fclose(file);

    TARGA_HEADER* header = parse_header(bytestream);

    uint64_t total_pixels = header->height * header->width;
    PIXEL* pixel_data = parse_tga(bytestream);
    free(bytestream);

    // bundle everything together
    IMAGE* image_data = malloc(sizeof(IMAGE));
    *image_data = (IMAGE) {
        header,
        pixel_data
    };

    return image_data;
}

int main(int argc, char *argv[])
{
    initialize_UI();

    IMAGE* image_data = NULL;
    PALETTE* color_palette = NULL;
    uint8_t return_bitflags;

    // TODO: clean up where mallocs occur, place them in a consistent, predictable place
    // BUG: possible double free, malloc(): invalid next size (unsorted)
    //      occurs when switching between COLOR_CUBE and FIRST_COLORS_FOUND
    while ((return_bitflags = navigate_UI()) != EXIT_REQUESTED) {
        if (return_bitflags & NO_UPDATE) {
            continue;
        }

        // TODO: this logic might be leading to the seg fault,
        // investigate
        if (return_bitflags & FILEPATH_UPDATE) {
            free(image_data);
            free(color_palette);

            image_data = try_read_image(trim(get_filepath()));
            color_palette = generate_palette(image_data, COMPRESSED_216, get_palette_gen_method());
            initialize_palette(color_palette);
        }

        if (return_bitflags & PALETTE_GEN_UPDATE && image_data != NULL) {
            free(color_palette);
            color_palette = generate_palette(image_data, COMPRESSED_216, get_palette_gen_method());
            initialize_palette(color_palette);
        }

        if (image_data == NULL || color_palette == NULL) {
            continue;
        }

        display_image(*image_data->header, image_data->data);
    }

    end_UI();
}
