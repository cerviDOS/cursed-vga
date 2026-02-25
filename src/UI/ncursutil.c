#define _XOPEN_SOURCE_EXTENDED

#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <menu.h>
#include <wchar.h>

#include "ncursutil.h"


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
