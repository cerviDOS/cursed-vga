#ifndef IMAGE_DISPLAY_H
#define IMAGE_DISPLAY_H

#include <SDL3/SDL.h>
#include <ncurses.h>

#include "../tga.h"
#include "../palette.h"

// Sets the color palette to be used when drawing an
// image with the functions below
void initialize_palette(PALETTE* color_palette);

// Draws an image next to the TUI,
// best suited for images < 100 pixels
void draw_image_ncurses(WINDOW* canvas, IMAGE* image);

// Opens a new window to display an image,
// intended for larger images that cannot be viewed
// within the terminal.
void draw_image_SDL(SDL_Window* canvas, IMAGE* image);

#endif
