#ifndef NCURSES_UTIL_H
#define NCURSES_UTIL_H

#include <ncurses.h>
#include <menu.h>
#include <form.h>

void draw_frame(WINDOW* win);
void draw_frame_with_title(WINDOW* win, char* text);
void draw_double_wide_frame_with_title(WINDOW* win, char* text);

void print_in_center(WINDOW* win, char* text);

#endif


