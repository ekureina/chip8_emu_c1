#include <errno.h>
#include <stdio.h>
#include "chip8.h"
#include "graphics.h"

#define PROG_C_START 0x200
#define MEMORY_SIZE 4096
#define REGISTER_NUM 16
#define STACK_SIZE 16
#define GFX_WIDTH 64
#define GFX_HEIGHT 32
#define STACK_HEIGHT 16
#define ASCII_FILL 234

static chip8_register_s registers[REGISTER_NUM];
static chip8_memory_ptr_s prog_c;
static chip8_memory_ptr_s index;

static chip8_stack_s stack[STACK_SIZE];
static chip8_register_s stack_p;

static chip8_memorycell_s graphics_memory[GFX_WIDTH*GFX_HEIGHT];
static chip8_memorycell_s memory[MEMORY_SIZE];

static void clear_stack( void ) {
    for (stack_p = 0; stack_p < STACK_SIZE; ++stack_p) {
        stack[stack_p] = 0;
    }
    stack_p = 0;
}
static void clear_memory( void ) {
    for (index = 0; index < MEMORY_SIZE; ++index) {
        memory[index] = 0;
    }
    index = 0;
}
static void clear_graphics_memory( void ) {
    chip8_memory_ptr_s graphics_memory_p;
    for (
        graphics_memory_p = 0;
        graphics_memory_p < GFX_WIDTH*GFX_HEIGHT;
        ++graphics_memory_p
    ) {
        graphics_memory[graphics_memory_p] = 0;
    }
}

int chip8_init( void ) {
    graphics_init();
    prog_c = PROG_C_START;
    clear_memory();
    clear_graphics_memory();
    clear_stack();    
}

int chip8_load_rom(const char* rom_name) {
    FILE* rom_handle = fopen(rom_name, "rb");
    if (!rom_handle) return -1;
    fread((void*) memory+PROG_C_START, sizeof(memory[0]), sizeof(memory)/sizeof(memory[0])-PROG_C_START, rom_handle);
    if (ferror(rom_handle)) return -1;
    if (fclose(rom_handle)) return -1;
    return 0;
}
int chip8_end( void ) {
    graphics_end();
}

static int get_register(uint8_t register_number, chip8_register_s *reg_val) {
    if (register_number >= REGISTER_NUM) {
        errno = EINVAL;
        *reg_val = 0;
        return -1;
    }
    *reg_val = registers[register_number];
    return 0;
}

static int set_register(uint8_t register_number, chip8_register_s reg_val) {
    if (register_number >= REGISTER_NUM) {
        errno = EINVAL;
        return -1;
    }
    registers[register_number] = reg_val;
    return 0;
}

static int copy_register(uint8_t register_number_A, uint8_t register_number_B) {
    if (register_number_A >= REGISTER_NUM || register_number_B >= REGISTER_NUM) {
        errno = EINVAL;
        return -1;
    }
    registers[register_number_A] = registers[register_number_B];
}

static int jump_to(chip8_memory_ptr_s location) {
    if (location < PROG_C_START) {
        errno = EINVAL;
        return -1;
    }
    prog_c = location;
}

static int next_instruction( void ) {
    prog_c += 2;
}

int perform_instruction( void ) {
    chip8_opcode_s opcode =  memory[prog_c];
    errno = EINVAL;
    return -1;
}
