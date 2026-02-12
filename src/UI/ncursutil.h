#ifndef NCURSES_UTIL_H
#define NCURSES_UTIL_H

#include <ncurses.h>
#include <menu.h>
#include <form.h>

// TODO: move UI_ELEMENT mentions into its own header
#include "nav.h"

void attach_title_to_win(WINDOW* win, char* text);
void draw_frame_with_title(WINDOW* win, char* text);
void draw_double_wide_frame_with_title(WINDOW* win, char* text);

MENU* create_menu(char **choices,
                  int num_choices,
                  int y_pos,
                  int x_pos,
                  int l,
                  int w);


UI_ELEMENT* create_button_ui_element(char* label,
                                     int y_pos,
                                     int x_pos);

UI_ELEMENT* create_field_ui_element(char* label,
                                    int form_buf_width,
                                    int y_pos,
                                    int x_pos,
                                    char* window_title);

UI_ELEMENT* create_menu_ui_element(char** choices,
                                   int num_choices,
                                   int req_height,
                                   int req_width,
                                   int y_pos,
                                   int x_pos,
                                   char* window_title);

enum APPEARANCE {
    DIM,
    HOVERED,
    SELECTED
};

void set_ui_element_appearance(UI_ELEMENT* element, enum APPEARANCE app);
void accept_input(UI_ELEMENT* element);

#endif


