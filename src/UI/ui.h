#ifndef UI_NEW_H
#define UI_NEW_H

#include "../palette.h"

typedef struct {
    char req_filepath[512];
    enum GENERATION_METHOD req_gen_method;
    enum PALETTE_SIZE req_palette_size;
} UI_RETURN_DATA;

void initialize_ui();
void destroy_ui();

UI_RETURN_DATA navigate_ui();

void display_image(IMAGE* image, PALETTE* palette);

#endif
