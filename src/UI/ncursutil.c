// ncurses util functions here
#define _XOPEN_SOURCE_EXTENDED

#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <menu.h>
#include <wchar.h>

#include "ncursutil.h"
#include "nav.h"
#include "ui.h"

void attach_title_to_win(WINDOW* win, char* text)
{
    int y,x;
    getmaxyx(win, y, x);

    int midpoint = (x >> 1) - (strlen(text) >> 1);
    wattron(win, A_BOLD);
    mvwprintw(win, 0, midpoint, "%s", text);
    wattroff(win, A_BOLD);
}

void draw_frame(WINDOW* win)
{
    box(win, 0, 0);
}

void draw_frame_with_title(WINDOW* win, char* text)
{
    draw_frame(win);
    attach_title_to_win(win, text);
}

void draw_double_wide_frame(WINDOW* win)
{
    cchar_t horiz, vert, top_left, top_right, bot_left, bot_right;
    setcchar(&horiz, L"\u2550", 0, 0, NULL);
    setcchar(&vert, L"\u2551", 0, 0, NULL);
    setcchar(&top_left, L"\u2554", 0, 0, NULL);
    setcchar(&top_right, L"\u2557", 0, 0, NULL);
    setcchar(&bot_left, L"\u255A", 0, 0, NULL);
    setcchar(&bot_right, L"\u255D", 0, 0, NULL);

    wborder_set(win, &vert, &vert, &horiz, &horiz, &top_left, &top_right, &bot_left, &bot_right);
}

void draw_double_wide_frame_with_title(WINDOW* win, char* text)
{
    draw_double_wide_frame(win);
    attach_title_to_win(win, text);
}

void print_in_center(WINDOW* win, char* text)
{
    int y, x;
    getmaxyx(win, y, x);

    int y_mid = y >> 1;
    int x_mid = (x >> 1) - (strlen(text) >> 1);

    mvwprintw(win, y_mid, x_mid, "%s", text);
}

void set_menu_appearance(UI_ELEMENT* elem, int std_pair, int invalid_pair)
{
    set_menu_fore(elem->menu, COLOR_PAIR(std_pair) | A_STANDOUT);
    set_menu_back(elem->menu, COLOR_PAIR(std_pair));
    set_menu_grey(elem->menu, COLOR_PAIR(invalid_pair));
}

void set_form_appearance(UI_ELEMENT* elem, int std_pair, int invalid_pair)
{
    set_field_back(current_field(elem->form), COLOR_PAIR(std_pair) | A_STANDOUT);
    // TODO: Temp logic, assumes that the userptr is assigned to the label string.
    // should be made its own member later (maybe a dedicated single field form struct?)
     mvwprintw(form_win(elem->form), 1, 1, "%s", (char*) form_userptr(elem->form));

}

void set_button_appearance(UI_ELEMENT* elem, int std_pair, int invalid_pair)
{
    print_in_center(elem->button->win, elem->button->label);
}

void set_ui_element_appearance(UI_ELEMENT* element, enum APPEARANCE app)
{
    int standard_pair;
    int invalid_pair;

    if (app != DIM) {
        standard_pair = ACTIVE_UI_ELEM_COLOR_PAIR;
        invalid_pair = INVALID_UI_ELEM_COLOR_PAIR;
    } else {
        standard_pair = INVALID_UI_ELEM_COLOR_PAIR;
        invalid_pair = INACTIVE_INVALID_UI_ELEM_COLOR_PAIR;
    }

    // TODO: duplicated parent_window code here could be replaced by
    // keeping track of the window in the UI_ELEMENT rather than the
    // individual internal elements
    void (*set_appearance_fn)(UI_ELEMENT*, int, int);
    WINDOW* parent_window;
    switch (element->type) {
        case MENU_T:
            parent_window = menu_win(element->menu);
            set_appearance_fn = set_menu_appearance;
            break;
        case FORM_T:
            parent_window = form_win(element->form);
            set_appearance_fn = set_form_appearance;
            break;
        case BUTTON_T:
            parent_window = element->button->win;
            set_appearance_fn = set_button_appearance;
            break;
    }

    wattrset(parent_window, COLOR_PAIR(standard_pair));

    if (app == SELECTED) {
        draw_double_wide_frame_with_title(parent_window, element->window_title);
    } else {
        draw_frame_with_title(parent_window, element->window_title);
    }

    set_appearance_fn(element, standard_pair, invalid_pair);

    wrefresh(parent_window);
}

UI_ELEMENT* create_button_ui_element(char* label, int y_pos, int x_pos)
{
    BUTTON* button = malloc(sizeof(BUTTON));
    memcpy(button->label, label, BUTTON_LABEL_LIMIT);

    WINDOW* button_window = newwin(3, strlen(label)+2, y_pos, x_pos);
    button->win = button_window;

    UI_ELEMENT* elem = calloc(1, sizeof(UI_ELEMENT));
    *elem = (UI_ELEMENT) {
        .type = BUTTON_T,
        .button = button,
    };

    print_in_center(button_window, label);
    draw_frame(button_window);

    wrefresh(button_window);
    return elem;
}

UI_ELEMENT* create_field_ui_element(char* label, int form_buf_width, int y_pos, int x_pos, char* window_title)
{
    FIELD* fields[2];

    const int field_width = form_buf_width - strlen(label);

    fields[0] = new_field(1, field_width, 0, 0, 0, 0);
    set_field_back(fields[0], A_STANDOUT);

    fields[1] = NULL;

    FORM* form = new_form(fields);

    WINDOW* parent_window = newwin(3, form_buf_width+2, y_pos, x_pos);

    set_form_win(form, parent_window);
    set_form_sub(form, derwin(parent_window, 1, field_width, 1, strlen(label)+1));

    mvwprintw(parent_window, 1, 1, "%s", label);

    // TODO: TEMP - use userptr for storing the label for now
    set_form_userptr(form, malloc(sizeof(char) * strlen(label)));
    strcpy(form_userptr(form), label);

    post_form(form);

    UI_ELEMENT* elem = calloc(1, sizeof(UI_ELEMENT));

    *elem = (UI_ELEMENT) {
        .type = FORM_T,
        .form = form
    };
    memcpy(elem->window_title, window_title, WINDOW_TITLE_LIMIT);

    draw_frame_with_title(parent_window, window_title);
    wrefresh(parent_window);

    return elem;
}

ITEM** init_menu_items(char** choices, int num)
{
    ITEM** items = (ITEM**) calloc(num+1, sizeof(ITEM*));

    for (int i = 0; i < num ; i++) {
        items[i] = new_item(choices[i], NULL);
    }
    items[num] = (ITEM*) NULL;

    return items;
}

UI_ELEMENT* create_menu_ui_element(char** choices,
                                   int num_choices,
                                   int req_height,
                                   int req_width,
                                   int y_pos,
                                   int x_pos,
                                   char* window_title)
{
    MENU* menu = new_menu(init_menu_items(choices, num_choices));

    int min_height, min_width;
    scale_menu(menu, &min_height, &min_width);

    // TODO: handle case where title > 32
    if (min_width < strlen(window_title)) {
        min_width = strlen(window_title);
    }
    min_width = (min_width < strlen(window_title)) ? strlen(window_title) : min_width;

    int real_height = (req_height < min_height) ? min_height : req_height;
    int real_width = (req_width < min_width) ? min_width : req_width;

    WINDOW* parent_window = newwin(real_height+2, real_width+2, y_pos, x_pos);

    keypad(parent_window, TRUE);

    set_menu_win(menu, parent_window);
    set_menu_sub(menu, derwin(parent_window, real_height, real_width, 1, 1));
    set_menu_fore(menu, A_STANDOUT);
    set_menu_grey(menu, A_DIM);

    post_menu(menu);

    UI_ELEMENT* elem = calloc(1, sizeof(UI_ELEMENT));

    *elem = (UI_ELEMENT) {
        .type = MENU_T,
        .menu = menu
    };
    memcpy(elem->window_title, window_title, WINDOW_TITLE_LIMIT);

    draw_frame_with_title(parent_window, window_title);
    wrefresh(parent_window);

    return elem;
}
