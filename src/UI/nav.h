#ifndef UI_NAV_H
#define UI_NAV_H

#include "ui_element.h"

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
