#ifndef _CHIP8_H
#define _CHIP8_H
#include <stdint.h>

typedef uint16_t chip8_opcode_s;
typedef uint8_t chip8_register_s;
typedef uint8_t chip8_memorycell_s;
typedef uint16_t chip8_memory_ptr_s;
typedef uint16_t chip8_stack_s;

int chip8_init( void );
int chip8_load_rom(const char* rom_name);
int chip8_end( void );

int perform_instruction( void );

#endif
