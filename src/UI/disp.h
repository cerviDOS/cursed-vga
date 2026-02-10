#ifndef IMAGE_DISPLAY_H
#define IMAGE_DISPLAY_H

#include <ncurses.h>

#include "../tga.h"
#include "../palette.h"

void initialize_palette(PALETTE* color_palette);
void draw_image(WINDOW* canvas, IMAGE* image);

#endif
