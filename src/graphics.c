#ifdef DEBUG
#include <stdio.h>
#define g_init
#define g_end
#define g_clear
#define g_noise printf("\a");
#define g_get_key(key) key = getchar()
#define ASCII_FILL '#'
#else
#include <ncurses.h>
#define g_init initscr();\
    noecho();\
    curs_set(0);\
    cbreak();
#define g_end endwin();
#define g_clear refresh();
#define g_noise beep();
#define g_get_key(key) key = getch()
#define ASCII_FILL '\xDB'
#endif
#include <errno.h>
#include "graphics.h"

void graphics_init( void ) {
    g_init
}

void graphics_end( void ) {
    g_end
}

int draw_screen(uint8_t membuf[][32], uint8_t screen_width, uint8_t screen_height) {
#ifndef DEBUG
    //uint8_t max_height, max_width;
    //getmaxyx(stdscr, max_height, max_width);
    uint8_t width, height;
    for (height = 0; height < screen_height; ++height) {
        for (width = 0; width < screen_width; ++width) {
            mvaddch(height, width, membuf[width][height] ? ASCII_FILL : ' ');
        }
    }
    refresh();
#else
    uint8_t width, height;
    for (height = 0; height < screen_height; ++height) {
        for (width = 0; width < screen_width; ++width) {
            printf("%c", membuf[width][height] ? ASCII_FILL : ' ');
        }
        printf("\n");
    }
    for (width = 0; width < screen_width; ++width) {
        printf("-");
    }
    printf("\n");
#endif
    return 0;
}

int clear_screen( void ) {
    g_clear
    return 0;
}

void emit_noise( void ) {
    g_noise
}

int8_t get_key_blocking() {
    static char* key_array = "q&[{;,.aoe'j}puk";
    int out;
    g_get_key(out);
    char* key_search = key_array;
    while (*key_search != out) {
        ++key_search;
        if (!*key_search) {
            return -1;
        }
    }
    return key_search-key_array;
}
