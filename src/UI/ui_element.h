#ifndef UI_ELEMENT_H
#define UI_ELEMENT_H

#include <ncurses.h>
#include <menu.h>
#include <form.h>

enum UI_ELEMENT_TYPE {
    MENU_T,
    FORM_T,
    BUTTON_T,
};

#define BUTTON_LABEL_LIMIT 32
typedef struct {
    WINDOW* win;
    char label[BUTTON_LABEL_LIMIT];
} BUTTON;


#define WINDOW_TITLE_LIMIT 32
typedef struct UI_ELEMENT {
    enum UI_ELEMENT_TYPE type;
    char window_title[WINDOW_TITLE_LIMIT];
    union {
        MENU* menu;
        FORM* form;
        BUTTON* button;
    };
    struct UI_ELEMENT* up;
    struct UI_ELEMENT* down;
    struct UI_ELEMENT* left;
    struct UI_ELEMENT* right;
    void (*callback_fn)(struct UI_ELEMENT* self); // TODO: document this
} UI_ELEMENT;


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

// TODO: implement
// void free_ui_element(UI_ELEMENT* element);


enum UI_ELEMENT_APPEARANCE {
    DIM,
    HOVERED,
    SELECTED
};

void set_ui_element_appearance(UI_ELEMENT* element, enum UI_ELEMENT_APPEARANCE app);
void accept_ui_element_input(UI_ELEMENT* element);

#endif
