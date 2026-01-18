#define _XOPEN_SOURCE_EXTENDED

#include <stdlib.h>
#include <string.h>

#include <ncurses.h>
#include <form.h>
#include <menu.h>

#include <wchar.h>
#include <locale.h>

#include "UI.h"

/* TODO:
 *  Document functions, split file into modules, cleanup
 *
 */

static const int NCURSES_PAIR_OFFSET = 4;
static const int NCURSES_COLOR_OFFSET = 16;

static int ACTIVE_UI_ELEM_COLOR_PAIR = 1;
static int INVALID_UI_ELEM_COLOR_PAIR = 2;

typedef struct {
    int size;
    char* choices[32];
} CHOICES;

typedef struct {
    FORM* form;
    WINDOW* win;
    char* win_title;
} FORM_BUNDLE;

typedef struct {
    MENU* menu;
    ITEM* selected_item;
    WINDOW* win;
    char* win_title;
} MENU_BUNDLE;

typedef struct {
    WINDOW* win;
    char* button_text;
} BUTTON_BUNDLE;

FORM_BUNDLE* filepath_bundle;
MENU_BUNDLE* palette_gen_method_bundle;
MENU_BUNDLE* palette_size_bundle;

/* Tentative UI layout, indexed by posiition on the screen as (row, col):
 *
 *  -----------------------
 * | (0,0) filepath form   |
 *  -----------------------
 * |  (1, 0)   |    (1,1)  |
 * |  palette  |  palette  |
 * |    gen    |   size    |
 * |   menu    |   menu    |
 *  -----------------------
 *               |  (2,0)  |
 *               | confirm |
 *                --------
 */

enum UI_ELEMENT_ID {
    FILEPATH_FORM = FILEPATH_UPDATE - 1,
    PALETTE_GENERATION_MENU =  PALETTE_GEN_UPDATE - 1
};

enum UI_ELEMENT_TYPE {
    FORM_TYPE,
    MENU_TYPE,
    BUTTON_TYPE
};

enum APPEARANCE {
    DIM,
    HOVERED,
    SELECTED
};

typedef struct UI_NODE {
    void* bundle_ptr;
    enum UI_ELEMENT_ID id;
    enum UI_ELEMENT_TYPE type;
    struct UI_NODE* up;
    struct UI_NODE* down;
    struct UI_NODE* left;
    struct UI_NODE* right;
} UI_NODE;

const UI_NODE* UI_graph_head;

const PALETTE* curr_color_palette;

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


void set_menu_bundle_appearance(const void* bundle_ptr, enum APPEARANCE appearance)
{
    
    MENU_BUNDLE* menu_bundle = (MENU_BUNDLE*) bundle_ptr;

    int attr = 0;
    cchar_t top, bottom, left, right, tlcorner, trcorner, blcorner, brcorner;


    switch (appearance) {
        case DIM:
            attr = A_DIM;
            break;
        case HOVERED:
            attr = 0;
            break;
        case SELECTED:
            attr = 0;
            
            break;
    }

    wattrset(menu_bundle->win, COLOR_PAIR(ACTIVE_UI_ELEM_COLOR_PAIR) | attr);
    //box(menu_bundle->win, border_char, border_char);
    //wborder_set(menu_bundle->win, const cchar_t *, const cchar_t *, const cchar_t *, const cchar_t *, const cchar_t *, const cchar_t *, const cchar_t *, const cchar_t *)

    wborder_set(menu_bundle->win, 0, 0, 0, 0, 0, 0, 0, 0);

    attach_title_to_win(menu_bundle->win, menu_bundle->win_title);
    set_menu_fore(menu_bundle->menu, COLOR_PAIR(ACTIVE_UI_ELEM_COLOR_PAIR) | A_REVERSE | attr);
    set_menu_back(menu_bundle->menu, COLOR_PAIR(ACTIVE_UI_ELEM_COLOR_PAIR) | attr);
    set_menu_grey(menu_bundle->menu, COLOR_PAIR(INVALID_UI_ELEM_COLOR_PAIR) | attr);
    set_menu_grey(menu_bundle->menu, COLOR_PAIR(INVALID_UI_ELEM_COLOR_PAIR) | attr);
    wrefresh(menu_bundle->win);
}

void set_form_bundle_appearance(const void* bundle_ptr, enum APPEARANCE appearance)
{
    FORM_BUNDLE* form_bundle = (FORM_BUNDLE*) bundle_ptr;

    int attr = 0;
    char border_char = 0;
    switch (appearance) {
        case DIM:
            attr = A_DIM;
            break;
        case HOVERED:
            attr = 0;
            break;
        case SELECTED:
            attr = 0;
            border_char = '$';
            break;
    }

    wattrset(form_bundle->win, COLOR_PAIR(ACTIVE_UI_ELEM_COLOR_PAIR) | attr);
    box(form_bundle->win, border_char, border_char);
    attach_title_to_win(form_bundle->win, form_bundle->win_title);
    set_field_back(current_field(form_bundle->form), A_STANDOUT | attr);
    mvwprintw(filepath_bundle->win, 1, 1, "filepath:"); // NOTE: temp value, should be made more modular in the future
    wrefresh(form_bundle->win);
}

void set_UI_node_appearance(const UI_NODE* node, enum APPEARANCE appearance)
{
    void (*set_appearance_fn)(const void*, enum APPEARANCE);
    switch(node->type) {
        case FORM_TYPE:
            set_appearance_fn = set_form_bundle_appearance;
            break;
        case MENU_TYPE:
            set_appearance_fn = set_menu_bundle_appearance;
            break;
        case BUTTON_TYPE:
            break;
    }

    set_appearance_fn(node->bundle_ptr, appearance);
}

ITEM** init_items(CHOICES* choices)
{
    int size = choices->size;

    ITEM** items = (ITEM**) calloc(size+1, sizeof(ITEM*));

    for (int i = 0; i < size ; i++) {
        items[i] = new_item(choices->choices[i], NULL);
    }
    items[size] = (ITEM*) NULL;

    return items;
}

void initialize_UI()
{
    // form for file path
    // menu for palette size:
    // menu for palette generation
    setlocale(LC_ALL, "");

    initscr();
    noecho();
    curs_set(0);
    start_color();
    keypad(stdscr, TRUE);

    refresh();

    /**** Initialize Color Pairs ****/

    int INVALID_GREY = 8;
    init_color(INVALID_GREY,
               500,
               500,
               500);

    alloc_pair(COLOR_WHITE, COLOR_BLACK);
    alloc_pair(INVALID_GREY, COLOR_BLACK);

    /**** Initialize Filepath Form ****/

    filepath_bundle = malloc(sizeof(FORM_BUNDLE));

    const int form_win_h = 3;
    const int form_win_w = 51;

    filepath_bundle->win = newwin(form_win_h, form_win_w, 0, 0);
    filepath_bundle->win_title = " Path to image ";
    keypad(filepath_bundle->win, 1);

    // Decorate window
    draw_frame_with_title(filepath_bundle->win, filepath_bundle->win_title);
    char* form_label = "filepath:";
    mvwprintw(filepath_bundle->win, 1, 1, "%s", form_label);

    // Initialize fields & create form
    FIELD* fields[2];

    const int field_width = form_win_w - strlen(form_label) - 2;
    fields[0] = new_field(1, field_width, 0, 0, 0, 0);
    set_field_back(fields[0], A_STANDOUT);

    fields[1] = NULL;

    filepath_bundle->form = new_form(fields);

    // Attach to window
    set_form_win(filepath_bundle->form, filepath_bundle->win);
    set_form_sub(filepath_bundle->form, derwin(filepath_bundle->win, 1, field_width, 1,strlen(form_label)+1));
    post_form(filepath_bundle->form);

    wrefresh(filepath_bundle->win);

    /**** Initialize Generation Method Menu ****/

    palette_gen_method_bundle = malloc(sizeof(MENU_BUNDLE));

    // Define choices, should mirror GENERATION_METHOD enum in palette.h
    CHOICES palette_choices = (CHOICES) {
        .size = 7,
        .choices = {
            "Color Cube",
            "Uniform",
            "Popularity",
            "Median",
            "Octree",
            "K-Means Clustering",
            "First Colors Found"
        }
    };

    // Attach items, create menu, set non-selectable items
    ITEM** palette_items = init_items(&palette_choices);
    item_opts_off(palette_items[1], O_SELECTABLE);
    item_opts_off(palette_items[2], O_SELECTABLE);
    item_opts_off(palette_items[3], O_SELECTABLE);
    item_opts_off(palette_items[4], O_SELECTABLE);
    item_opts_off(palette_items[5], O_SELECTABLE);

    palette_gen_method_bundle->menu = new_menu(palette_items);
    palette_gen_method_bundle->selected_item = current_item(palette_gen_method_bundle->menu);

    set_menu_fore(palette_gen_method_bundle->menu, A_REVERSE);
    //set_menu_back(palette_gen_method_bundle->menu, COLOR_PAIR(ACTIVE_UI_ELEM_COLOR_PAIR));
    set_menu_grey(palette_gen_method_bundle->menu, A_DIM);

    // Create main window
    const int palette_win_h = 9;
    const int palette_win_w = 22;

    palette_gen_method_bundle->win = newwin(palette_win_h, palette_win_w, form_win_h, 0);
    palette_gen_method_bundle->win_title = " Color Palette ";
    keypad(palette_gen_method_bundle->win, TRUE);

    // Attach to window
    set_menu_win(palette_gen_method_bundle->menu, palette_gen_method_bundle->win);

    // Creates a sub window within the main window for the menu choices to reside
    set_menu_sub(palette_gen_method_bundle->menu,
                 derwin(palette_gen_method_bundle->win,
                        palette_choices.size,
                        palette_win_w - 2,
                        1, 1));

    post_menu(palette_gen_method_bundle->menu);

    // Decorate window
    draw_frame_with_title(palette_gen_method_bundle->win,palette_gen_method_bundle->win_title);

    wrefresh(palette_gen_method_bundle->win);

    /**** Construct UI Element Graph ****/

    UI_NODE* filepath_node = malloc(sizeof(UI_NODE));
    filepath_node->bundle_ptr = filepath_bundle;
    filepath_node->id = FILEPATH_FORM;
    filepath_node->type = FORM_TYPE;

    UI_NODE* palette_gen_node = malloc(sizeof(UI_NODE));
    palette_gen_node->bundle_ptr = palette_gen_method_bundle;
    palette_gen_node->id = PALETTE_GENERATION_MENU;
    palette_gen_node->type = MENU_TYPE;

    // Link filepath node
    filepath_node->up = NULL;
    filepath_node->down = palette_gen_node;
    filepath_node->left = NULL;
    filepath_node->right = NULL;

    // Link palette generation node
    palette_gen_node->up = filepath_node;
    palette_gen_node->down = NULL;
    palette_gen_node->left = NULL;
    palette_gen_node->right = NULL;

    // Set filepath node as head
    UI_graph_head = filepath_node;

    /**** Dim all on startup ****/

    set_UI_node_appearance(filepath_node, DIM);
    set_UI_node_appearance(palette_gen_node, DIM);

}

void end_UI()
{
    endwin();
}

char* get_filepath()
{
    form_driver(filepath_bundle->form, REQ_VALIDATION);
    return field_buffer(current_field(filepath_bundle->form), 0);
}

enum GENERATION_METHOD get_palette_gen_method()
{
    return item_index(palette_gen_method_bundle->selected_item);
}

int accept_form_input(void* bundle_ptr)
{
    FORM_BUNDLE* form_bundle = (FORM_BUNDLE*) bundle_ptr;

    form_driver(form_bundle->form, REQ_END_FIELD);
    curs_set(1);

    int c = '\0';
    int modified = 0;
    while((c = wgetch(form_bundle->win)) != 10) { // 10: Enter key pressed
        switch(c) {
            case KEY_BACKSPACE: // Backspace
                form_driver(form_bundle->form, REQ_LEFT_CHAR);
                form_driver(form_bundle->form, REQ_DEL_CHAR);
                break;
            default:
                form_driver(form_bundle->form, c);
                break;
        }
        modified = 1;
    }
    curs_set(0);

    return modified;
}

int accept_menu_input(MENU_BUNDLE* bundle_ptr)
{
    MENU_BUNDLE* menu_bundle = (MENU_BUNDLE*) bundle_ptr;

    int in = '\0';
    int modified = 0;
    ITEM* curr;

    do {
        switch((in = wgetch(menu_bundle->win))) {
            case KEY_DOWN:
                menu_driver(menu_bundle->menu, REQ_DOWN_ITEM);
                break;
            case KEY_UP:
                menu_driver(menu_bundle->menu, REQ_UP_ITEM);
                break;
            case KEY_ENTER:
            case 10:
                curr = current_item(menu_bundle->menu);
                if (item_opts(curr) & O_SELECTABLE) {
                    menu_bundle->selected_item = curr;
                    modified = 1;
                }
                break;
        }
        wrefresh(menu_bundle->win);
    } while(in != 10); // 10: Enter key pressed

    return modified;
}

int accept_input(const UI_NODE* node)
{
    uint8_t ret = 0;

    set_UI_node_appearance(node, SELECTED);

    switch(node->type) {
        case FORM_TYPE:
            ret = accept_form_input(node->bundle_ptr) << node->id;
            break;
        case MENU_TYPE:
            ret = accept_menu_input(node->bundle_ptr) << node->id;
            break;
        case BUTTON_TYPE:
            break;
    }

    set_UI_node_appearance(node, HOVERED);

    return ret;
}

uint8_t navigate_UI()
{
    static const UI_NODE* selected_node = NULL;
    if (selected_node == NULL) {
        selected_node = UI_graph_head;
    }

    uint8_t bitflags = NO_UPDATE;

    /*
    // use f1 as exit for now, if pressed send "1"
    //bitflags |= accept_form_input(filepath_bundle);
    bitflags |= navigate_menu(palette_gen_method_bundle);
    */

    // interact with program with keyboard up down left right
    // menus, forms, buttons, arranged as grid,
    // press x to select
    // F1 to exit

    // Want to dim all widgets but the selected one, need to know:
    // Whether the widget is a form or menu
    // The widget previously selected

    // 2D array of "widget structs" that store void pointer to 
    // the bundle and an enum that indicates what type of widget it is

    int in = 0;
    UI_NODE* tentative_node = NULL;

    do {
        if ((in = getch()) == 'x') {
            bitflags |= accept_input(selected_node);
        } else {
            switch (in) {
                case 'd':
                    return bitflags;
                case KEY_LEFT:
                    tentative_node = selected_node->left;
                    break;
                case KEY_RIGHT:
                    tentative_node = selected_node->right;
                    break;
                case KEY_UP:
                    tentative_node = selected_node->up;
                    break;
                case KEY_DOWN:
                    tentative_node = selected_node->down;
                    break;
            }

            if (tentative_node != NULL) {
                set_UI_node_appearance(selected_node, DIM);
                set_UI_node_appearance(tentative_node, HOVERED);
                selected_node = tentative_node;
                tentative_node = NULL;
            }
        }
    } while (1);
}

int compare_color_hex(PIXEL rgb1, PIXEL rgb2)
{
    int rgb1hex = rgb1.red_val << 16 | rgb1.green_val << 8 | rgb1.blue_val;
    int rgb2hex = rgb2.red_val << 16 | rgb2.green_val << 8 | rgb2.blue_val;

    return rgb1hex - rgb2hex;
}

int square(int val)
{
    return val*val;
}

int dist_squared(PIXEL rgb1, PIXEL rgb2)
{
    return square(rgb2.red_val - rgb1.red_val)
    + square(rgb2.green_val - rgb1.green_val)
    + square(rgb2.blue_val - rgb1.blue_val);
}

int find_nearest_color(PIXEL target_color)
{
    // TODO: this can be optimized later, just get it working for now
    int closest_index = 0;
    int shortest_dist = INT32_MAX;

    for (int i = 0; i < curr_color_palette->size; i++) {
        int curr_dist;
        PIXEL curr_color = curr_color_palette->data[i];
        if ((curr_dist = dist_squared(target_color, curr_color)) < shortest_dist) {
            closest_index = i;
            shortest_dist = curr_dist;
        }
    }
    return closest_index + NCURSES_COLOR_OFFSET;
}

// Returns the color pair number matching the given fg & bg combination.
// Pair number will be negative if the inverse of the color exists.
// Allocates a new color pair if neither this combination nor the inverse already
// exists.
int find_nearest_pair(uint8_t fg, uint8_t bg)
{
    int pair_num;

    // check if inverse pair exists before attempting to create a new pair
    if ((pair_num = find_pair(bg, fg)) != -1) {
        return -pair_num;
    } else {
        return alloc_pair(fg, bg);
    }
}

void initialize_palette(PALETTE* color_palette)
{
    const double color_scale = 1000.0 / 255.0;
    for (int i = 0; i < color_palette->size; i++) {
        PIXEL curr = color_palette->data[i];
        init_color(i + NCURSES_COLOR_OFFSET,
                   curr.red_val * color_scale,
                   curr.green_val * color_scale,
                   curr.blue_val * color_scale);
    }

    curr_color_palette = color_palette;
}

void display_color_pair(WINDOW* win, int y, int x, uint16_t fg, uint16_t bg)
{
    static const wchar_t* half_block = L"\u2580";

    int pair = find_nearest_pair(fg, bg);

    if (pair < 0) {
        wattr_set(win, A_REVERSE, -pair ,NULL);
    } else {
        wattr_set(win, 0, pair, NULL);
    }

    mvwaddwstr(win, y, x, half_block);
}

void display_image(TARGA_HEADER header, PIXEL* pixel_data)//, PALETTE* color_palette)
{
    WINDOW* img_win = newwin(header.height*4, header.width*4, 0, 52);

    refresh();
    wrefresh(img_win);

    int pixel_pos = 0;
    for (int row = 0; row < (header.height / 2); row++) {
        for (int col = 0; col < header.width; col++) {
            int top_color = find_nearest_color(pixel_data[pixel_pos]);
            int bottom_color = find_nearest_color(pixel_data[pixel_pos + header.width]);

            display_color_pair(img_win, row, col, top_color, bottom_color);

            pixel_pos++;
        }
        pixel_pos += header.width;
    }

    // If image height is odd, the last row still needs to be printed
    if (header.height % 2 != 0) {

        refresh();
        int last_index = header.height * header.width;
        //int row_pos = last_index - header.width;

        for (int col = 0; col < header.width; col++) {
            int top_color = find_nearest_color(pixel_data[pixel_pos++]);

            display_color_pair(img_win,
                               header.height / 2,
                               col,
                               top_color,
                               COLOR_BLACK);
        }
    }

    wrefresh(img_win);
}
