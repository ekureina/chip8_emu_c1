COMPILER:=gcc
OUTPUT_DIRS:=bin/ obj/

obj/:
	mkdir -p obj

bin/:
	mkdir -p bin

obj/%.o: src/%.c
	$(COMPILER) -I include -o $@ -c $<

bin/chip8: obj/ obj/chip8.o obj/main.o obj/graphics.o bin/
	$(COMPILER) -o bin/chip8 obj/chip8.o obj/main.o obj/graphics.o -lncurses

clean:
	rm -rf $(OUTPUT_DIRS)


.PHONY: clean
