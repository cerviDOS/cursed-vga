#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "strutil.h"

#include "palette.h"
#include "tga.h"

#include "UI/ui.h"


// TODO: encapsulate this logic within a dedicated image struct
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

// TODO: clean up where mallocs occur, place them in a consistent, predictable place
// BUG: Switching between palette generation methods
// cases a malloc(): corrupted top size. MacOS unaffected.
// BUG: freeze if directory is chosen.

int main(int argc, char *argv[])
{
    initialize_ui();

    IMAGE* image_data = NULL;
    PALETTE* color_palette = NULL;

    UI_RETURN_DATA ret;

    while (1) {
        ret = navigate_ui();

        if (image_data != NULL) {
            destroy_tga(image_data);
        }

        image_data = try_read_image(trim(ret.req_filepath));

        if (image_data == NULL) {
            continue;
        }

        if (color_palette != NULL) {
            destroy_palette(color_palette);
        }

        color_palette = generate_new_palette(image_data,
                                             ret.req_palette_size,
                                             ret.req_gen_method);

        display_image(image_data, color_palette);
    }

    getchar();
    destroy_ui();
}
