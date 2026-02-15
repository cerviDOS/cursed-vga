#ifndef UI_NAV_H
#define UI_NAV_H

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

void initialize_navigator(UI_ELEMENT** elements, int num_elements);

enum ACTION {
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    SELECT
};

void nav_act(enum ACTION);

void nav_move_up();
void nav_move_down();
void nav_move_left();
void nav_move_right();

#endif
