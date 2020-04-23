#ifdef DEBUG
#include <stdio.h>
#define g_init() do {} while (0)
#define g_end() do {} while (0)
#define g_clear() do {} while (0)
#define g_noise() do { printf("\a"); } while (0)
#define g_get_key(key) do { key = getchar(); } while (0)
#define g_get_key_nb(key) do { fread(&key, sizeof(char), 1, stdin); } while (0)
#define g_output_char(height, width, ch) do { printf("%c", ch); } while (0)
#define g_write_screen(screen_width) do {\
    for (width = 0; width < screen_width; ++width) {\
        printf("-");\
    }\
    printf("\n"); } while (0)
#define g_initialize_screen(screen_width) do {\
    for (width = 0; width < screen_width; ++width) {\
        printf("-");\
    }\
} while (0)
#define g_increment_line() do { printf("\n"); } while (0)
#define ASCII_FILL '#'
#else
#include <ncurses.h>
#define g_init() do {initscr();\
    noecho();\
    curs_set(0);\
    cbreak(); } while (0)
#define g_end() do { endwin(); } while (0)
#define g_clear() do { refresh(); } while (0)
#define g_noise() do { beep(); } while (0)
#define g_get_key(key) do { key = getch(); } while (0)
#define g_get_key_nb(key) do { nodelay(stdscr, TRUE);\
    key = getch();\
    nodelay(stdscr, FALSE); } while (0)
#define g_output_char(height, width, ch) do {\
            mvaddch(height, width, ch);\
} while (0)
#define g_write_screen(width) do { refresh(); } while (0)
#define g_initialize_screen(width) do {\
    /*uint8_t max_height, max_width;\
    getmaxyx(stdscr, max_height, max_width);*/\
} while (0)
#define g_increment_line() do { } while (0)
#define ASCII_FILL '\xDB'
#endif
#include <errno.h>
#include "graphics.h"

void graphics_init( void ) {
    g_init();
}

void graphics_end( void ) {
    g_end();
}

int draw_screen(uint8_t membuf[][32], uint8_t screen_width, uint8_t screen_height) {
    uint8_t width, height;
    g_initialize_screen(width);
    for (height = 0; height < screen_height; ++height) {
        for (width = 0; width < screen_width; ++width) {
            g_output_char(height, width, membuf[width][height] ? ASCII_FILL : ' ');
        }
        g_increment_line();
    }
    g_write_screen(width);
    return 0;
}

int clear_screen( void ) {
    g_clear();
    return 0;
}

void emit_noise( void ) {
    g_noise();
}

int8_t get_key_blocking( void ) {
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

int8_t get_key( void ) {
    static char* key_array = "q&[{;,.aoe'j}puk";
    int out;
    g_get_key_nb(out);
    char* key_search = key_array;
    while (*key_search != out) {
        ++key_search;
        if (!*key_search) {
            return -1;
        }
    }
    return key_search-key_array;
}
