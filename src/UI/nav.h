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

typedef struct {
    WINDOW* win;
    char label[32];
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

//void configure_ui(UI_ELEMENT* elements, ...);
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


// struct UI_GRAPH { UI_ELEMENT* head, UI_ELEMENT* curr }
// struct UI_ELEMENT { union {MENU* FORM* BUTTON*}, up, down, left, right}
// enum ELEMENT_TYPE {MENU, FORM, BUTTON}

// void configure_ui(UI_ELEMENT elements...) (caller should configure the links ahead of time and pass in all the pointers)

// void nav_move_up();
// void nav_move_down();
// void nav_move_left();
// void nav_move_right();
// void nav_select();
//
// Flow: navigate UI
// On receiving a "submit" or "done" request, return an array with all
// pointers to all UI elements that involve accepting input
// 
// How to keep track of which is which, maybe with UI_IDs again?
// Keep track of relevant data in UI.c, make user pointer to update that data whenever its changed in nav
//
