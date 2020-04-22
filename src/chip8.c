#include <errno.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include "chip8.h"
#include "graphics.h"

#define PROG_C_START 0x200
#define MEMORY_SIZE 4096
#define REGISTER_NUM 16
#define STACK_SIZE 16
#define GFX_WIDTH 64
#define GFX_HEIGHT 32
#define STACK_HEIGHT 16
#define FONT_START 0x50
#define FONT_WIDTH 5
#define FONT_CHARLEN 16

static chip8_register_s registers[REGISTER_NUM];
static chip8_memory_ptr_s prog_c;
static chip8_memory_ptr_s index;

static chip8_stack_s stack[STACK_SIZE];
static chip8_register_s stack_p;

static chip8_memorycell_s graphics_memory[GFX_WIDTH][GFX_HEIGHT];
static chip8_memorycell_s memory[MEMORY_SIZE];

static chip8_memorycell_s fontset[FONT_WIDTH*FONT_CHARLEN] =
{
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

static chip8_register_s delay_timer;
static chip8_register_s sound_timer;

static void load_font( void ) {
    uint8_t count;
    for (count = 0; count < FONT_WIDTH*FONT_CHARLEN; ++count) {
       memory[count+FONT_START] = fontset[count];
    }
}

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
    chip8_memory_ptr_s graphics_memory_w_p;
    for (
        graphics_memory_w_p = 0;
        graphics_memory_w_p < GFX_WIDTH;
        ++graphics_memory_w_p
    ) {
        chip8_memory_ptr_s graphics_memory_h_p;
        for (
            graphics_memory_h_p = 0;
            graphics_memory_h_p < GFX_HEIGHT;
            ++graphics_memory_h_p
        ) {
            graphics_memory[graphics_memory_w_p][graphics_memory_h_p] = 0;
        }
    }
    clear_screen();
}

int chip8_init( void ) {
    graphics_init();
    prog_c = PROG_C_START;
    clear_memory();
    load_font();
    clear_graphics_memory();
    clear_stack();    
    srandom(time(NULL));
    return 0;
}

int chip8_load_rom(const char* rom_name) {
    FILE* rom_handle = fopen(rom_name, "rb");
    if (!rom_handle) return -1;
    fread((void*) (memory+PROG_C_START), sizeof(memory[0]), sizeof(memory)/sizeof(memory[0])-PROG_C_START, rom_handle);
    if (ferror(rom_handle)) return -1;
    if (fclose(rom_handle)) return -1;
    return 0;
}
int chip8_end( void ) {
    graphics_end();
    return 0;
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

static int copy_register(uint8_t register_number_x, uint8_t register_number_y) {
    if (register_number_x >= REGISTER_NUM || register_number_y >= REGISTER_NUM) {
        errno = EINVAL;
        return -1;
    }
    registers[register_number_x] = registers[register_number_y];
    return 0;
}

static int jump_to(chip8_memory_ptr_s location) {
    if (location < PROG_C_START) {
        errno = EINVAL;
        return -1;
    }
    prog_c = location;
    return 0;
}

static void next_instruction( void ) {
    prog_c += 2;
}

static int perform_two_reg(chip8_opcode_s opcode) {
    uint8_t reg_x_number = (uint8_t) ((opcode & 0x0F00) >> 8);
    uint8_t reg_y_number = (uint8_t) ((opcode & 0x00F0) >> 4);
    if (!(opcode & 0x000F)) {
        return copy_register(reg_x_number, reg_y_number);
    }
    else {
        chip8_register_s x_val;
        chip8_register_s y_val;
        if (get_register(reg_x_number, &x_val) == -1)
            return -1;
        if (!((opcode & 0x000F) == 6 || (opcode & 0x000F) == 0xE) &&
            get_register(reg_y_number, &y_val) == -1)
            return -1;
        switch (opcode & 0x000F) {
            case (0x0001):
            {
                return set_register(reg_x_number, x_val | y_val);
            }
            case (0x0002):
            {
                return set_register(reg_x_number, x_val & y_val);
            }
            case (0x0003):
            {
                return set_register(reg_x_number, x_val ^ y_val);
            }
            case (0x0004):
            {
                set_register(REGISTER_NUM-1, ((x_val+(int)y_val) & 0x100) >> 8);
                return set_register(reg_x_number, x_val + y_val);
            }
            case (0x0005):
            {
                set_register(REGISTER_NUM-1, x_val < y_val ? 1 : 0);
                return set_register(reg_x_number, x_val - y_val);
            }
            case (0x0006):
            {
                set_register(REGISTER_NUM-1, x_val & 1);
                return set_register(reg_x_number, x_val >> 1);
            }
            case (0x0007):
            {
                set_register(REGISTER_NUM-1, y_val < x_val ? 1 : 0);
                return set_register(reg_x_number, y_val - x_val);
            }
            case (0x000E):
            {
                set_register(REGISTER_NUM-1, (x_val & 0x80) >> 7);
                return set_register(reg_x_number, x_val << 1);
            }
            default:
                errno = EINVAL;
                return -1;
        }
    }
    return -1; // Never Reached
}

static int perform_base_ops(chip8_opcode_s opcode) {
    switch (opcode & 0x00FF) {
        case (0x00E0): // Clr Screen
        {
            clear_graphics_memory();
            return 0;
        }
        case (0X0000): // NOOP
        {
            return 0;
        }
        case (0x00EE): // Return from subroutine
        {
            return jump_to((chip8_memory_ptr_s) stack[--stack_p]);
        }
        case (0x00FE): // Emulator specific termination instr
            break;
        default:
            errno = ENOSYS;
    }
    return -1;
}

static int perform_f_opcodes(chip8_opcode_s opcode) {
    switch (opcode & 0x00FF) {
        case (0x0007):
        {
            return set_register((opcode & 0x0F00) >> 8, delay_timer);
        }
        case (0x000A):
        {
            int key;
            while ((key = get_key_blocking()) == -1);
            return set_register((opcode & 0x0F00) >> 8, key);
        }
        case (0x0015):
        case (0x0018):
        case (0x001E):
        case (0x0029):
        case (0x0033):
        {
            chip8_register_s val_x;
            if (get_register((opcode & 0x0F00) >> 8, &val_x) == -1) {
                errno = EINVAL;
                return -1;
            }
            if ((opcode & 0x0030) == 0x0010) {
                if (opcode & 0x0002) {
                    index += val_x;
                    set_register(REGISTER_NUM-1, (index & 0x1000) >> 12);
                    index &= 0xFFF;
                } else if (opcode & 0x0008) {
                    sound_timer = val_x;
                } else {
                    delay_timer = val_x; 
                }
            } else if (opcode & 0x0010) {
                memory[index] = val_x / 100;
                memory[index+1] = (val_x % 100) / 10;
                memory[index+2] = val_x % 10;
            } else {
                index = (val_x*FONT_WIDTH) + FONT_START;
            }
            break;
        }
        case (0x0055):
        case (0x0065):
        {
            uint8_t reg_cap = (opcode & 0x0F00) >> 8;
            if (reg_cap >= REGISTER_NUM) {
                errno = EINVAL;
                return -1;
            }
            chip8_memory_ptr_s track;
            for (track = 0; track < ((opcode & 0x0010) >> 4) * 2 * reg_cap; ++track) {
                if ((opcode & 0x00FF) == 0x0055) {
                    get_register(track, memory+index+track);
                    ++track;
                } else {
                    set_register(track, memory[index+track]);
                }
            }
        }
        default:
            errno = ENOSYS;
            return -1;
    }
    return 0;
}

int chip8_perform_instruction( void ) {
    if (prog_c >= MEMORY_SIZE) {
        errno = EINVAL;
        return -1;
    }
    chip8_opcode_s opcode =  (memory[prog_c] << sizeof(chip8_memorycell_s)*CHAR_BIT) | memory[prog_c+1];
    next_instruction();
    switch (opcode & 0xF000) {
        case (0x0000):
        {
            if (perform_base_ops(opcode) == -1)
                return -1;
            break;
        }
        case (0x1000): // Unconditional Jump
        {
            if (jump_to(opcode & 0x0FFF) == -1)
                return -1;
            break;
        }
        case (0x2000): // Call Subroutine
        {
            if (stack_p == STACK_SIZE) {
                errno = ENOMEM;
                return -1;
            }
            stack[stack_p++] = opcode & 0x0FFF;
            break;
        }
        case (0x3000): // Skip if equal
        {
            if (registers[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
                next_instruction();
            break;
        }
        case (0x4000): // Skip if not equal
        {
            if (registers[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
                next_instruction();
            break;
        }
        case (0x5000): // Skip if register vals equal
        {
            if (opcode & 0x000F) {
                errno = EINVAL;
                return -1;
            }
            chip8_register_s val_x;
            chip8_register_s val_y;
            if (get_register(opcode & 0x0F00 >> 8, &val_x) == -1)
                return -1;
            if (get_register(opcode & 0x00F0 >> 4, &val_y) == -1)
                return -1;
            if (val_x == val_y)
                next_instruction();
            break;
        }
        case (0x6000): // SetReg
        {
            registers[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
            break;
        }
        case (0x7000): // AddReg
        {
            uint8_t reg_number = (uint8_t) ((opcode & 0x0F00) >> 8);
            chip8_register_s new_val;
            if (get_register(reg_number, &new_val) == -1)
                return -1;
            new_val += opcode & 0x00FF;
            if (set_register(reg_number, new_val) == -1)
                return -1;
            break;
        }
        case (0x8000): // Two reg opcodes
        {
            perform_two_reg(opcode);
        }
        case (0x9000): // Skip if register vals unequal
        {
            if (opcode & 0x000F) {
                errno = EINVAL;
                return -1;
            }
            chip8_register_s val_x;
            chip8_register_s val_y;
            if (get_register((opcode & 0x0F00) >> 8, &val_x) == -1)
                return -1;
            if (get_register((opcode & 0x00F0) >> 4, &val_y) == -1)
                return -1;
            if (val_x != val_y)
                next_instruction();
            break;
        }
        case (0xA000):
        {
            index = opcode & 0x0FFF;
            break;
        }
        case (0xB000):
        {
            chip8_register_s reg_0_val;
            // No need to error check this read, is valid
            get_register(0, &reg_0_val);
            if (jump_to(prog_c+reg_0_val) == -1)
                return -1;
            break;
        }
        case (0xC000):
        {
            if (set_register((opcode & 0x0F01) >> 8, random() % (opcode & 0x00FF)) == -1)
                return -1;
            break;
        }
        case (0xD000):
        {
            chip8_register_s val_x;
            chip8_register_s val_y;
            chip8_register_s overflow_val = 0;
            if (get_register((opcode & 0x0F00) >> 8, &val_x) == -1)
                return -1;
            if (get_register((opcode & 0x00F0) >> 4, &val_y) == -1)
                return -1;
            uint8_t w_count, h_count;
            uint8_t h_count_max = opcode & 0x000F;
            if (val_x+CHAR_BIT >= GFX_WIDTH || val_y+h_count_max >= GFX_HEIGHT) {
                errno = EINVAL;
                return -1;
            }
            for (h_count = 0; h_count < h_count_max; ++h_count) {
                chip8_memorycell_s row = memory[index+h_count];
                //printf("%d %d\n", h_count, row);
                for (w_count = 0; w_count < CHAR_BIT; ++w_count) {
                    chip8_memorycell_s draw_mask = (row >> (7 - w_count)) & 1;
                    //printf("%d", draw_mask);
                    overflow_val |= draw_mask & graphics_memory[val_x+w_count][val_y+h_count];
                    graphics_memory[val_x+w_count][val_y+h_count] ^= draw_mask;
                }
            }
            set_register(REGISTER_NUM-1, overflow_val);
            if (draw_screen(graphics_memory, GFX_WIDTH, GFX_HEIGHT) == -1)
                return -1;
            break;
        }
        case (0xE000):
        {
            if (!((opcode & 0x00FF) == 0x009E ||
                (opcode & 0x00FF) == 0x00A1)) {
                errno = ENOSYS;
                return -1;
            }
            chip8_register_s val_x = registers[(opcode & 0x0F00) >> 8];
            if (val_x > 16) {
                errno = EINVAL;
                return -1;
            }
            int8_t key_status = get_key();
            if (((key_status == val_x) && (opcode & 0x00FF) == 0x009E) ||
                (key_status != val_x)) {
                next_instruction();
            }
            break;
        }
        case (0xF000):
        {
            if (perform_f_opcodes(opcode) == -1)
                return -1;
            break;
        }
        default:
        {
            errno = ENOSYS;
            return -1;
        }
    }
    if (delay_timer > 0) {
        --delay_timer;
    }
    if (sound_timer > 0) {
        --sound_timer;
        if (!sound_timer) {
            emit_noise();
        }
    }
    return 0;
}

void chip8_coredump( void ) {
    fprintf(stderr, "I = 0x%04X\n", index);
    fprintf(stderr, "PC = 0x%04X\n", prog_c);
    uint16_t counter;
    for (counter = 0; counter < REGISTER_NUM; ++counter) {
        fprintf(stderr, "V%X = 0x%02X\n", counter, registers[counter]);
    }
    fprintf(stderr, "MEMORY:\n");
    for (counter = PROG_C_START; counter >= MEMORY_SIZE || !(memory[counter] << 8 | memory[counter+1]); counter+=8) {
        fprintf(stderr, "0x%04X: %02X %02X %02X %02X %02X %02X %02X %02X\n",
                counter, memory[counter], memory[counter+1],
                memory[counter+2], memory[counter+3], memory[counter+4],
                memory[counter+5], memory[counter+6], memory[counter+7]);
    }
}
