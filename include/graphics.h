#ifndef _GRAPHICS_H
#define _GRAPHICS_H
#include <stdint.h>

void graphics_init( void );
void graphics_end( void );
int draw_screen(uint8_t membuf[][32], uint8_t screen_width, uint8_t screen_height);
int clear_screen( void );
void emit_noise( void );

#endif
