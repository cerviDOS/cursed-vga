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
// Struct representing a pressable button
// within the TUI
typedef struct {
    WINDOW* win;
    char label[BUTTON_LABEL_LIMIT];
} BUTTON;

// Umbrella struct to encapsulate away the dedicated logic
// for ncurses' menu and forms while providing a touch of polymorphism
#define WINDOW_TITLE_LIMIT 32
typedef struct UI_ELEMENT {
    enum UI_ELEMENT_TYPE type;

    // Title displayed in the border surrounding the
    // element
    char window_title[WINDOW_TITLE_LIMIT];

    // Internal element that this struct is wrapping
    union {
        MENU* menu;
        FORM* form;
        BUTTON* button;
    };

    // Links to adjancent elements,
    // used to enable keyboard navigation
    // between elements
    struct UI_ELEMENT* up;
    struct UI_ELEMENT* down;
    struct UI_ELEMENT* left;
    struct UI_ELEMENT* right;

    // Function to be called when this UI element is selected
    void (*callback_fn)(struct UI_ELEMENT* self);
} UI_ELEMENT;

// Creates a button with the provided label, with its
// top left corner located at (y_pos, x_pos)
UI_ELEMENT* create_button_ui_element(char* label,
                                     int y_pos,
                                     int x_pos);

// Creates a field that accepts text entry with the provided label and its
// top left corner located at (y_pos, x_pos)

//
// form_buf_width determines both the visible width of text entry and the size
// of the interal buffer
UI_ELEMENT* create_field_ui_element(char* label,
                                    int form_buf_width,
                                    int y_pos,
                                    int x_pos,
                                    char* window_title);

// Creates a menu with the provided choices and its
// top left corner located at (y_pos, x_pos)
//
// req_height/width specifies a requested height/width for the menu.
// If req_height < num_choices or req_width < length of the longest
// menu option, the size for each respective dimension will snap to
// the minimum size needed to view the menu contents
UI_ELEMENT* create_menu_ui_element(char** choices,
                                   int num_choices,
                                   int req_height,
                                   int req_width,
                                   int y_pos,
                                   int x_pos,
                                   char* window_title);

// Deallocates the provided UI element
void free_ui_element(UI_ELEMENT* element);

enum UI_ELEMENT_APPEARANCE {
    DIM,
    HOVERED,
    SELECTED
};

// Sets the appearance of a UI element
//
// DIM : grayed-out single-wide border and contents
// HOVERED : bright white single-wide border and contents
// SELECTED : bright white double-wide border and contents
void set_ui_element_appearance(UI_ELEMENT* element, enum UI_ELEMENT_APPEARANCE app);



#endif
