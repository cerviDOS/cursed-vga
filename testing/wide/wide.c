#define _XOPEN_SOURCE_EXTENDED
#include <ncursesw/curses.h>
#include <locale.h>
#include <stdlib.h>

int main(void)
{
    initscr();
    start_color();

    cchar_t* ch = malloc(sizeof(cchar_t));
    for (int i = 200; i < 300; i++) {
        init_extended_pair(i, COLOR_MAGENTA, COLOR_MAGENTA);

        
        setcchar(ch, L"@", 0, 0, NULL);
        add_wch(ch);
    }
    
    getch();

    endwin();
}
