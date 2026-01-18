#ifndef UI_H
#define UI_H

#include "tga.h"
#include "palette.h"

enum STATUS_FLAG {
    NO_UPDATE = 0,
    FILEPATH_UPDATE = 1,
    PALETTE_GEN_UPDATE = 2,
    EXIT_REQUESTED = 16
};

void initialize_UI();
void end_UI();

// returns bitflag byte aligning with STATUS_FLAGS enum
uint8_t navigate_UI();
char* get_filepath();
enum GENERATION_METHOD get_palette_gen_method();

void initialize_palette(PALETTE* color_palette);
void display_image(TARGA_HEADER header, PIXEL* pixel_data);

#endif
