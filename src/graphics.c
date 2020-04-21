#if 0
#else
#include <ncurses.h>
#define g_init initscr
#define g_end endwin
#endif
#include "graphics.h"

void graphics_init( void ) {
    g_init();
}
void graphics_end( void ) {
    g_end();
}
int draw_screen( ) {
}
