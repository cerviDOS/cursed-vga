#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <locale.h>

#include <SDL3/SDL.h>

#include "ncursutil.h"
#include "nav.h"
#include "ui.h"

#include "disp.h"

static SDL_Window* image_window_SDL;
static WINDOW* image_window_ncurses;

static int submit_button_hit = 0;
static int exit_button_hit = 0;
static UI_RETURN_DATA data;

void set_filepath_callback(UI_ELEMENT* e)
{
    form_driver(e->form, REQ_VALIDATION);
    strcpy(data.req_filepath, field_buffer(current_field(e->form), 0));
}

void set_generation_method_callback(UI_ELEMENT* e)
{
    data.req_gen_method =
        item_index(current_item(e->menu));
}

void set_palette_size_callback(UI_ELEMENT* e)
{
    switch(item_index(current_item(e->menu))) {
        case 0:
            data.req_palette_size = COMPRESSED_240;
            break;
        case 1:
            data.req_palette_size = COMPRESSED_216;
            break;
        case 2:
            data.req_palette_size = COMPRESSED_128;
            break;
        case 3:
            data.req_palette_size = COMPRESSED_64;
            break;
        case 4:
            data.req_palette_size = COMPRESSED_32;
            break;
        case 5:
            data.req_palette_size = COMPRESSED_16;
            break;
    }
}

void set_submit_button_callback(UI_ELEMENT* e)
{
    submit_button_hit = 1;
}

int check_and_reset_submit_button()
{
    int ret = submit_button_hit;
    submit_button_hit = 0;
    return ret;
}

void initialize_ui()
{
    setlocale(LC_ALL, "");

    initscr();
    timeout(100);
    noecho();
    curs_set(0);
    start_color();
    keypad(stdscr, TRUE);

    SDL_Init(SDL_INIT_VIDEO);

    /***** Color Pairs *****/

    int INVALID = 8;
    int INACTIVE = 9;
    int INACTIVE_INVALID = 10;

    init_color(INVALID,
               500,
               500,
               500);

    init_color(INACTIVE,
               500,
               500,
               500);

    init_color(INACTIVE_INVALID,
               250,
               250,
               250);

    alloc_pair(COLOR_WHITE, COLOR_BLACK); // pair 1, ACTIVE_UI_ELEM_COLOR_PAIR
    alloc_pair(INVALID, COLOR_BLACK); // pair 2, INVALID_UI_ELEM_COLOR_PAIR
    alloc_pair(INACTIVE, COLOR_BLACK); // pair 3, INACTIVE_UI_ELEM_COLOR_PAIR
    alloc_pair(INACTIVE_INVALID, COLOR_BLACK); // pair 4, INACTIVE_INVALID_UI_ELEM_COLOR_PAIR

    refresh();

    /***** create UI elements *****/

    UI_ELEMENT* filepath_form =
        create_field_ui_element("filepath:",
                                48,
                                0,
                                0,
                                " IMAGE PATH ");

    enum {NUM_CHOICES_GEN = 7};
    char* gen_choices[NUM_CHOICES_GEN] = {
        "COLOR CUBE", "UNIFORM", "POPULARITY",
        "MEDIAN_CUT", "OCTREE", "K_MEANS", "FIRST_COLORS_FOUND"
    };

    UI_ELEMENT* palette_gen_menu =
        create_menu_ui_element(gen_choices,
                               NUM_CHOICES_GEN,
                               0,
                               31,
                               3,
                               0,
                               " QUANTIZATION METHOD ");


    ITEM** palette_gen_items = menu_items(palette_gen_menu->menu);
    item_opts_off(palette_gen_items[1], O_SELECTABLE);
    item_opts_off(palette_gen_items[3], O_SELECTABLE);
    item_opts_off(palette_gen_items[4], O_SELECTABLE);
    item_opts_off(palette_gen_items[5], O_SELECTABLE);

    enum {NUM_CHOICES_SIZE = 6};
    char* size_choices[NUM_CHOICES_SIZE] = {"240", "216 ", "128", "64", "32", "16"};
    UI_ELEMENT* palette_size_menu =
        create_menu_ui_element(size_choices,
                               NUM_CHOICES_SIZE,
                               7,
                               15,
                               3,
                               33,
                               " COLOR LIMIT ");

    UI_ELEMENT* submit_button =
        create_button_ui_element("submit", 12, 42);


    /***** link UI elements *****/

    filepath_form->down = palette_gen_menu;

    palette_gen_menu->up = filepath_form;
    palette_gen_menu->right = palette_size_menu;
    palette_gen_menu->down = submit_button;

    palette_size_menu->up = filepath_form;
    palette_size_menu->left = palette_gen_menu;
    palette_size_menu->down = submit_button;

    submit_button->up = palette_size_menu;

    /***** set up callbacks *****/

    filepath_form->callback_fn = set_filepath_callback;
    palette_gen_menu->callback_fn = set_generation_method_callback;
    palette_size_menu->callback_fn = set_palette_size_callback;
    submit_button->callback_fn = set_submit_button_callback;

    /***** collect and register elements in navigator *****/
    enum {TOTAL_ELEMENTS = 4};
    UI_ELEMENT* elements[TOTAL_ELEMENTS] = {
        filepath_form,
        palette_gen_menu, palette_size_menu,
        submit_button
    };

    initialize_navigator(elements, TOTAL_ELEMENTS);

    /***** set up image display windows *****/
    image_window_ncurses = newwin(0,0,0,52);
    image_window_SDL = SDL_CreateWindow("cursedvga", 0, 0, SDL_WINDOW_HIDDEN);
    SDL_PumpEvents();
}

UI_RETURN_DATA navigate_ui()
{
    int input;
    enum ACTION action;
    do {
        // Maybe put this on another thread?
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                SDL_HideWindow(image_window_SDL);
            }
        }

        if ((input = getch()) == -1) {
            continue;
        }

        switch (input) {
            case KEY_UP:
                action = MOVE_UP;
                break;
            case KEY_DOWN:
                action = MOVE_DOWN;
                break;
            case KEY_LEFT:
                action = MOVE_LEFT;
                break;
            case KEY_RIGHT:
                action = MOVE_RIGHT;
                break;
            case 10:
                action = SELECT;
                break;
        }
        nav_act(action);
    } while (!check_and_reset_submit_button());


    refresh();
    return data;
}

void display_image(IMAGE* image, PALETTE* palette)
{
    initialize_palette(palette);

    wclear(image_window_ncurses);
    wrefresh(image_window_ncurses);
    SDL_HideWindow(image_window_SDL);

    if (image->header->height > 100 || image->header->width > 100) {
        draw_image_SDL(image_window_SDL, image);

        if (SDL_GetWindowFlags(image_window_SDL) & SDL_WINDOW_HIDDEN) {
            SDL_ShowWindow(image_window_SDL);
        }
    } else {
        draw_image_ncurses(image_window_ncurses, image);
    }
}

void destroy_ui()
{
    reset_color_pairs();
    endwin();

    SDL_Quit();
}


