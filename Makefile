COMPILER:=gcc
CFLAGS:=-Wall -Wpedantic -Werror
OUTPUT_DIRS:=bin/ obj/

bin/chip8: obj/ obj/chip8.o obj/main.o obj/graphics.o bin/
	$(COMPILER) -o bin/chip8 obj/chip8.o obj/main.o obj/graphics.o -lncurses

obj/:
	mkdir -p obj

bin/:
	mkdir -p bin

obj/%.o: CFLAGS+= -I include
obj/%.o: src/%.c
	$(COMPILER) $(CFLAGS) -o $@ -c $<

debug: CFLAGS+=-DDEBUG
debug: bin/chip8

clean:
	rm -rf $(OUTPUT_DIRS)

.PHONY: clean
