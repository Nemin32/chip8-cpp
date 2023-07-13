CC=g++

chip8: main.cpp emulator.cpp
	$(CC) -g -Og -o chip8 main.cpp emulator.cpp -lSDL2
	
oldchip8: oldmain.cpp
	$(CC) -g -Og -o oldchip8 oldmain.cpp -lSDL2
	
oldrun: oldchip8
	./oldchip8 ../chip8/roms/breakout.rom

run: chip8
	./chip8 ../chip8/roms/breakout.rom

clean:
	!rm chip8
	!rm oldchip8