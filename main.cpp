#include "emulator.hpp"
#include "renderer.hpp"
#include <cstdio>
#include <iomanip>
#include <ios>
#include <iostream>
#include <thread>


inline void drawRect(SDL_Renderer* renderer, int x, int y) {
	SDL_Rect rect;
	rect.w = SIZE;
	rect.h = SIZE;
	rect.x = x;
	rect.y = y;

	SDL_RenderFillRect(renderer, &rect);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        exit(1);
    }
    
    //rect.w = SIZE;
    //rect.h = SIZE;

    std::string fileName(argv[1]);
    Chip8 emu(fileName);
    
    /*
    for (int i = 0; i < 0xF; i++) {
    printf("%.4x: %.4x\n", 0x200 + i, emu.memory[0x200 + i]);
    }
    exit(1);
    
    for (int i = 0; i < 4096; i++) {
        printf("%.4x ", emu.memory[i]);
        if (i % 32 == 0) std::cout << std::endl;
    }
    
    exit(1);
    */

    SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Event event;

	SDL_CreateWindowAndRenderer(64 * SIZE, 32 * SIZE, SDL_WINDOW_SHOWN, &window, &renderer);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);

	while (true) {
		SDL_PollEvent(&event);

		if (event.type == SDL_QUIT) {
			break;
		} else if (event.type == SDL_KEYDOWN) {

			if (event.key.keysym.sym == SDLK_ESCAPE) break;

			for (int i = 0; i < 16; i++) {
				if (keymap[i] == event.key.keysym.sym) {
					emu.keys[i] = 1;
				}
			}	
		} else if (event.type == SDL_KEYUP) {
			for (int i = 0; i < 16; i++) {
				if (keymap[i] == event.key.keysym.sym) {
					emu.keys[i] = 0;
				}
			}
		}

        emu.tick();
		
		if (emu.drawFlag) {
			SDL_RenderClear(renderer);
			SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);


			for (int y = 0; y <  32; y++) {
				for (int x = 0; x < 64; x++) {		
					if (emu.framebuffer[x + y * 64] != 0) drawRect(renderer, x * SIZE, y * SIZE);
				}
			}

			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderPresent(renderer);

			emu.drawFlag = false;
		}

		std::this_thread::sleep_for(std::chrono::microseconds(1500));
	}

	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);

	SDL_Quit();
	return 0;
}