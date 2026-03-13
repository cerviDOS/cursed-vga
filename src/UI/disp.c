#define _XOPEN_SOURCE_EXTENDED

#include <SDL3/SDL.h>
#include <ncurses.h>

#include "../tga.h"
#include "../palette.h"
#include "ui_defs.h"

/*** Color Palette Management ***/

// Color palette used for mapping an image to a reduced color space
static PALETTE* curr_color_palette;

void initialize_palette(PALETTE* color_palette)
{
    const double color_scale = 1000.0 / 255.0;
    for (int i = 0; i < color_palette->size; i++) {
        PIXEL curr = color_palette->data[i];

        // offset by NUM_RESERVED_COLORS to avoid overriding
        // terminal defaults
        init_color((i + NUM_RESERVED_COLORS),
                   curr.red_val * color_scale,
                   curr.green_val * color_scale,
                   curr.blue_val * color_scale);

        // copy over values to decouple from lifetime of the provided
        // pointer
        curr_color_palette[i] = color_palette[i];
    }
}

/*** Helper Functions ***/

// Returns the difference between the hex code of two colors
int compare_color_hex(PIXEL rgb1, PIXEL rgb2)
{
    int rgb1hex = rgb1.red_val << 16 | rgb1.green_val << 8 | rgb1.blue_val;
    int rgb2hex = rgb2.red_val << 16 | rgb2.green_val << 8 | rgb2.blue_val;

    return rgb1hex - rgb2hex;
}

int square(int val)
{
    return val*val;
}

// Returns the distance squared between two RGB values, treated as points in 3D space
// Distance squared is used to avoid the expense of a square root
int dist_squared(PIXEL rgb1, PIXEL rgb2)
{
    return square(rgb2.red_val - rgb1.red_val)
    + square(rgb2.green_val - rgb1.green_val)
    + square(rgb2.blue_val - rgb1.blue_val);
}

/*** Color Mapping for SDL ***/

// Returns the RGB value within curr_color_palette with the smallest euclidian distance from the target_color
PIXEL find_nearest_pixel(PIXEL target_color)
{
    int closest_index = 0;
    int shortest_dist = INT32_MAX;

    for (int i = 0; i < curr_color_palette->size; i++) {
        int curr_dist;
        PIXEL curr_color = curr_color_palette->data[i];
        if ((curr_dist = dist_squared(target_color, curr_color)) < shortest_dist) {
            closest_index = i;
            shortest_dist = curr_dist;
        }
    }
    return curr_color_palette->data[closest_index];
}

/*** Color Mapping for NCURSES ***/

// Returns the index of the color with the smallest euclidian distance from the target_color
int find_nearest_ncurses_color(PIXEL target_color)
{
    // TODO: this can be optimized later, just get it working for now
    int closest_index = 0;
    int shortest_dist = INT32_MAX;

    for (int i = 0; i < curr_color_palette->size; i++) {
        int curr_dist;
        PIXEL curr_color = curr_color_palette->data[i];
        if ((curr_dist = dist_squared(target_color, curr_color)) < shortest_dist) {
            closest_index = i;
            shortest_dist = curr_dist;
        }
    }

    // offset by NUM_RESERVED_COLORS to match internal representation of colors
    // kept by ncurses
    return closest_index + NUM_RESERVED_COLORS;
}

// Returns the color pair number matching the given fg & bg combination.
//
// Pair number will be negative if the inverse of the color exists.
//
// Allocates a new color pair if neither this combination nor the inverse already
// exists.
int find_nearest_ncurses_pair(uint8_t fg, uint8_t bg)
{
    int pair_num;

    // check if inverse pair exists before attempting to create a new pair
    if ((pair_num = find_pair(bg, fg)) != -1) {
        return -pair_num;
    } else {
        return alloc_pair(fg, bg);
    }
}

/*** Image Display Functions ***/

// Writes the pixel data of an image to an SDL window
//
// Additionally, resizes the window to best fit the image
void draw_image_SDL(SDL_Window* window, IMAGE* image)
{
    int min_width_height = 100;
    int width = (image->header->width < min_width_height) ? min_width_height : image->header->width;
    int height = (image->header->height < min_width_height) ? min_width_height : image->header->height;

    SDL_SetWindowSize(window, width, height);

    SDL_Surface* surface = SDL_GetWindowSurface(window);
    int pixel_pos = 0;
    for (int row = 0; row < image->header->height; row++) {
        for (int col = 0; col < image->header->width; col++) {
            PIXEL curr_pixel = find_nearest_pixel(image->data[pixel_pos]);
            uint32_t color_val =
                (curr_pixel.red_val << 16)
                | (curr_pixel.green_val << 8)
                | curr_pixel.blue_val;

            uint32_t* surface_pixel =
                (uint32_t*) (surface->pixels
                + row * surface->pitch
                + col * 4);

            *surface_pixel = color_val;

            pixel_pos++;
        }
    }
    SDL_UpdateWindowSurface(window);
}

// Writes a half block with the requested foreground
// and background color at position (x,y) on the screen
//
// This technique allows higher resolution images to be
// displayed within the terminal.
void display_color_pair(WINDOW* win, int y, int x, uint16_t fg, uint16_t bg)
{
    static const wchar_t* half_block = L"\u2580";

    int pair = find_nearest_ncurses_pair(fg, bg);

    if (pair < 0) {
        wattr_set(win, A_REVERSE, -pair ,NULL);
    } else {
        wattr_set(win, 0, pair, NULL);
    }

    mvwaddwstr(win, y, x, half_block);
}

// Writes pixel data of an image to the specified ncurses window
void draw_image_ncurses(WINDOW* canvas, IMAGE* image)
{
    TARGA_HEADER* header = image->header;
    wresize(canvas, header->height >> 1, header->width);

    int pixel_pos = 0;
    for (int row = 0; row < (header->height / 2); row++) {
        for (int col = 0; col < header->width; col++) {
            int top_color = find_nearest_ncurses_color(image->data[pixel_pos]);
            int bottom_color = find_nearest_ncurses_color(image->data[pixel_pos + header->width]);

            display_color_pair(canvas, row, col, top_color, bottom_color);

            pixel_pos++;
        }
        pixel_pos += header->width;
    }

    // If image height is odd, the last row still needs to be printed
    if (header->height % 2 != 0) {

        refresh();
        int last_index = header->height * header->width;
        //int row_pos = last_index - header->width;

        for (int col = 0; col < header->width; col++) {
            int top_color = find_nearest_ncurses_color(image->data[pixel_pos++]);

            display_color_pair(canvas,
                               header->height / 2,
                               col,
                               top_color,
                               COLOR_BLACK);
        }
    }

    wrefresh(canvas);
}
