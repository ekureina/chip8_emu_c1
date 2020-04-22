#ifdef DEBUG
#define g_init
#define g_end
#define g_clear
#else
#include <ncurses.h>
#define g_init initscr();
#define g_end endwin();
#define g_clear refresh();
#endif
#include <errno.h>
#include "graphics.h"

void graphics_init( void ) {
    g_init
}

void graphics_end( void ) {
    g_end
}

int draw_screen( ) {
    errno = ENOSYS;
    return -1;
}

int clear_screen( void ) {
    g_clear
    return 0;
}
