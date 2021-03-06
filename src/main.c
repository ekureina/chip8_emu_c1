#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include "chip8.h"
#include "graphics.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "INVALID CALL to %s!\n", argv[0]); 
        return -1;
    }
    const char *rom_name = argv[1];
    errno = 0;
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
    if (errno) {
        fprintf(stderr, "ERROR: %d\n", errno);
#ifdef DEBUG
    }
    {
#endif
        chip8_coredump();
    }
    return 0;
}
