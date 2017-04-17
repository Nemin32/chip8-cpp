CC=g++

build: main.cpp
	$(CC) -o chip8 main.cpp -lSDL2

clean: chip8
	rm chip8
