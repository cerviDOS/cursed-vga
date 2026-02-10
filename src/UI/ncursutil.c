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

void draw_frame_with_title(WINDOW* win, char* text)
{
    box(win, 0, 0);
    attach_title_to_win(win, text);
}

void draw_double_wide_frame_with_title(WINDOW* win, char* text)
{
    cchar_t horiz, vert, top_left, top_right, bot_left, bot_right;
    setcchar(&horiz, L"\u2550", 0, 0, NULL);
    setcchar(&vert, L"\u2551", 0, 0, NULL);
    setcchar(&top_left, L"\u2554", 0, 0, NULL);
    setcchar(&top_right, L"\u2557", 0, 0, NULL);
    setcchar(&bot_left, L"\u255A", 0, 0, NULL);
    setcchar(&bot_right, L"\u255D", 0, 0, NULL);

    wborder_set(win, &vert, &vert, &horiz, &horiz, &top_left, &top_right, &bot_left, &bot_right);

    attach_title_to_win(win, text);
}

void set_ui_element_appearance(UI_ELEMENT* element, enum APPEARANCE app)
{
    int standard_pair = ACTIVE_UI_ELEM_COLOR_PAIR;
    int invalid_pair = INVALID_UI_ELEM_COLOR_PAIR;

    // TODO: compress this
    switch (app) {
        case DIM:
            standard_pair = INACTIVE_UI_ELEM_COLOR_PAIR;
            invalid_pair = INACTIVE_INVALID_UI_ELEM_COLOR_PAIR;
            break;
        case HOVERED:
            standard_pair = ACTIVE_UI_ELEM_COLOR_PAIR;
            invalid_pair = INVALID_UI_ELEM_COLOR_PAIR;
            break;
        case SELECTED:
            standard_pair = ACTIVE_UI_ELEM_COLOR_PAIR;
            invalid_pair = INVALID_UI_ELEM_COLOR_PAIR;
            break;
    }

    WINDOW* parent_window;
    switch (element->type) {
        case MENU_T:
            parent_window = menu_win(element->menu);
            set_menu_fore(element->menu, COLOR_PAIR(standard_pair) | A_STANDOUT);
            set_menu_back(element->menu, COLOR_PAIR(standard_pair));
            set_menu_grey(element->menu, COLOR_PAIR(invalid_pair));
            break;
        case FORM_T:
            parent_window = form_win(element->form);
            set_field_back(current_field(element->form), COLOR_PAIR(standard_pair) | A_STANDOUT);
            break;
        default:
            break;
    }

    wattrset(parent_window, COLOR_PAIR(standard_pair));

    if (app == SELECTED) {
        draw_double_wide_frame_with_title(parent_window, element->window_title);
    } else {
        draw_frame_with_title(parent_window, element->window_title);
    }

    wrefresh(parent_window);
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

UI_ELEMENT* create_menu_ui_element(char** choices, int num_choices, int y_pos, int x_pos, char* window_title)
{
    MENU* menu = new_menu(init_menu_items(choices, num_choices));
    int min_height, min_width;
    scale_menu(menu, &min_height, &min_width);

    // TODO: handle case where title > 32
    if (min_width < strlen(window_title)) {
        min_width = strlen(window_title);
    }

    min_width = (min_width < strlen(window_title)) ? strlen(window_title) : min_width;

    WINDOW* parent_window = newwin(min_height+2, min_width+2, y_pos, x_pos);

    keypad(parent_window, TRUE);

    set_menu_win(menu, parent_window);
    set_menu_sub(menu, derwin(parent_window, min_height, min_width, 1, 1));
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
