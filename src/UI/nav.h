#ifndef UI_NAV_H
#define UI_NAV_H

#include "ui_element.h"

// Initializes the UI navigator with the provided array of UI elements.
// Assumes that the links between the elements (up,down,left,right) have
// already been configured
void initialize_navigator(UI_ELEMENT** elements, int num_elements);

enum ACTION {
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    SELECT
};

// Performs an action on the currently hovered
// UI element.
//
// MOVE_X actions will move the currently hovered element
// to an adjacent element if it exists
//
// SELECT will run the on-select callback of the currently hovered element
void nav_act(enum ACTION);

#endif
