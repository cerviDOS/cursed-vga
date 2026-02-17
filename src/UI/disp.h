#ifndef IMAGE_DISPLAY_H
#define IMAGE_DISPLAY_H

#include <SDL3/SDL.h>
#include <ncurses.h>

#include "../tga.h"
#include "../palette.h"

void initialize_palette(PALETTE* color_palette);

void draw_image_ncurses(WINDOW* canvas, IMAGE* image);
void draw_image_SDL(SDL_Window* canvas, IMAGE* image);

#endif
