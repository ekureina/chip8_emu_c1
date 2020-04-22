#include <stdio.h>
#include "chip8.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "INVALID CALL to %s!\n", argv[0]); 
        return -1;
    }
    const char *rom_name = argv[1];
    chip8_init();
    chip8_load_rom(rom_name);
    int perform_status;
#ifdef DEBUG
    chip8_coredump();
#endif
    while ((perform_status = chip8_perform_instruction()) != -1) {
#ifdef DEBUG
        chip8_coredump();
#endif
    }
    chip8_end();
    chip8_coredump();
    return 0;
}
