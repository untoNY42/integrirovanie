.PHONY: all clean arc

all: main.o root.o integral.o math_funcs.o main_prog 

main_prog: main.o root.o integral.o math_funcs.o
	gcc -m32 -lm main.o root.o integral.o math_funcs.o -o main_prog

main.o: main.c
	gcc -m32 -c main.c -o main.o

root.o: root.c
	gcc -m32 -c root.c -o root.o

integral.o: integral.c
	gcc -m32 -c integral.c -o integral.o

math_funcs.o: math_funcs.asm
	nasm -f elf32 math_funcs.asm -o math_funcs.o

clean:
	rm -f *.o main_prog 

arc: main_prog
	mkdir -p ARC
	zip `date +%Y.%m.%d_%N`.zip main_prog main.c root.c integral.c math_funcs.asm Makefile
	mv *.zip ARC/
	make clean