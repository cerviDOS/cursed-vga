#include <stdlib.h>
#include "nav.h"
#include "ncursutil.h"

static UI_ELEMENT** elements;
static UI_ELEMENT* curr;

void initialize_navigator(UI_ELEMENT** elements_in, int num_elements)
{
    elements = calloc(num_elements, sizeof(UI_ELEMENT));
    for (int i = 0; i < num_elements; i++) {
        elements[i] = elements_in[i];

        set_ui_element_appearance(elements[i], DIM);

        // Call all callbacks to set default values in each form/menu.
        // Assumes that button callbacks are "on press" actions, so
        // ignore those callbacks until an actual press occurs.
        // In the future, might be worth making this the responsibility of the
        // caller (e.g. set defaults explicitly in init())
        if (elements[i]->type != BUTTON_T) {
            elements[i]->callback_fn(elements[i]);
        }
    }

    curr = elements[0];
    set_ui_element_appearance(curr, HOVERED);
}

void accept_menu_input(MENU* menu)
{
    int in = '\0';
    int done = 0;
    int modified = 0;
    ITEM* curr = NULL;

    WINDOW* parent_window = menu_win(menu);
    do {
        switch((in = wgetch(parent_window))) {
            case KEY_DOWN:
                menu_driver(menu, REQ_DOWN_ITEM);
                break;
            case KEY_UP:
                menu_driver(menu, REQ_UP_ITEM);
                break;
            case KEY_ENTER:
            case 10: // 10: Enter key pressed
                curr = current_item(menu);
                if (item_opts(curr) & O_SELECTABLE) {
                    return;
                } else {
                    continue;
                }
        }
    } while(1);
}

void accept_form_input(FORM* form)
{
    form_driver(form, REQ_END_FIELD);
    curs_set(1);

    WINDOW* parent_window = form_win(form);

    int c = '\0';
    while((c = wgetch(parent_window)) != 10) { // 10: Enter key pressed
        switch(c) {
            case KEY_BACKSPACE:
            case 127: // kitty on MacOS reports backspace as 127
                form_driver(form, REQ_LEFT_CHAR);
                form_driver(form, REQ_DEL_CHAR);
                break;
            default:
                form_driver(form, c);
                break;
        }
    }
    curs_set(0);
}

void accept_input(UI_ELEMENT* element)
{
    switch (element->type) {
        case MENU_T:
            accept_menu_input(element->menu);
            break;
        case FORM_T:
            accept_form_input(element->form);
            break;
        case BUTTON_T:
            break;
    }
    element->callback_fn(element);
}

void nav_act(enum ACTION action)
{
    UI_ELEMENT* next = NULL;
    
    switch (action) {
        case MOVE_UP:
            next = curr->up;
            break;
        case MOVE_DOWN:
            next = curr->down;
            break;
        case MOVE_LEFT:
            next = curr->left;
            break;
        case MOVE_RIGHT:
            next = curr->right;
            break;
        case SELECT:
            set_ui_element_appearance(curr, SELECTED);
            accept_input(curr);
            set_ui_element_appearance(curr, HOVERED);
            break;
    }

    if (next == NULL) {
        return;
    }

    set_ui_element_appearance(curr, DIM);
    set_ui_element_appearance(next, HOVERED);
    curr = next;
}
