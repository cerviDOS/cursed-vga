#ifndef UI_NEW_H
#define UI_NEW_H

#include "../palette.h"

typedef struct {
    char req_filepath[512]; // BUG: trace trap if too small, corrupts memory if too big
    enum GENERATION_METHOD req_gen_method;
    enum PALETTE_SIZE req_palette_size;
} UI_RETURN_DATA;

/*
// TODO: Temporarily exposing pair numbers, encapsulate this better later
#define ACTIVE_UI_ELEM_COLOR_PAIR 1
#define INVALID_UI_ELEM_COLOR_PAIR 2
#define INACTIVE_UI_ELEM_COLOR_PAIR 3
#define INACTIVE_INVALID_UI_ELEM_COLOR_PAIR 4

#define NUM_RESERVED_COLORS 16
#define NUM_RESERVED_PAIRS 4
*/

void initialize_ui();
void destroy_ui();

UI_RETURN_DATA navigate_ui();

void display_image(IMAGE* image, PALETTE* palette);

#endif
