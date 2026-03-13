#ifndef UI_DEFINITIONS_H
#define UI_DEFINITIONS_H

// Ncurses color pairs to be used when setting the
// appearance of UI elements
#define ACTIVE_UI_ELEM_COLOR_PAIR 1
#define INVALID_UI_ELEM_COLOR_PAIR 2
#define INACTIVE_UI_ELEM_COLOR_PAIR 3
#define INACTIVE_INVALID_UI_ELEM_COLOR_PAIR 4

// First 4 color pairs are reserved so the above
// UI appearance pairs are not overridden
#define NUM_RESERVED_PAIRS 4

// First 16 colors are reserved to avoid overriding
// the default terminal colors
#define NUM_RESERVED_COLORS 16

#endif
