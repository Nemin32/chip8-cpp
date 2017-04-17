#include <iostream>
#include <fstream>
#include <SDL2/SDL.h>

using namespace std;

	unsigned short oc;
	unsigned char memory[4096];
	unsigned char V[16];

	unsigned short I;
	unsigned short pc;

	//TODO: hang
	unsigned char delay;
	unsigned char sound;

	unsigned short stack[16];
	unsigned short sp;
	unsigned char gfx[64 * 32];

	unsigned char keys[16];
	bool drawFlag = false;

unsigned char chip8_fontset[80] =
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

void initSystem() {
	pc = 0x200;
	oc = 0;
	I = 0;
	sp = 0;
	cout << "[SYSTEM]: Inner variables set to default.\n";

	for (int i = 0; i < 16; i++) {V[i] = 0; stack[i] = 0;}
	for (int i = 80; i < 4096; i++) memory[i] = 0;
	for (int i = 0; i < 80; i++) memory[i] = chip8_fontset[i];
	cout << "[SYSTEM]: Font loaded.\n";
}

bool loadProgram(char* filename) {
	ifstream file(filename, ios::binary | ios::ate);

	if (file.is_open()) {
		cout << "[FILE]: Opened " << filename << ".\n";

		int size = file.tellg();
		file.clear(); file.seekg(0, ios::beg); //Go to beginning

		for (int i = 0; i < size; i++) {
			file >> noskipws >> memory[i + 512];
		}
		cout << "[FILE]: Memory is filled.\n";


		file.close();
		cout << "[FILE]: File is closed.\n";
	} else {
		cout << "[FILE]: ERROR: Could not open file. Exiting.\n";
		exit(1); 
	}


}



void drawRect(SDL_Renderer* renderer, int x, int y, int w, int h) {
	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;

	SDL_RenderFillRect(renderer, &rect);
}

void processOpcode() {
	oc = memory[pc] << 8 | memory[pc + 1];

	//cout << "[OPCODE]: OpCode " << std::hex << oc << std::dec <<" has been decoded.\n";

	unsigned short nnn = oc & 0x0FFF;
	unsigned short nn = oc & 0x00FF;
	unsigned short n = oc & 0x000F;
	
	unsigned short x = (oc & 0x0F00) >> 8;
	unsigned short y = (oc & 0x00F0) >> 4;

	switch (oc & 0xF000) {
		case 0x0000:
			if (n == 0x0000) { // OC == 00E0
				for (int i = 0; i < 32*64; i++) gfx[i] = 0;
				drawFlag = true;
			} else if (n == 0x000E) { //OC == 00EE
				--sp;
				pc = stack[sp];
				
			}
			
			pc+=2;
			break;

		case 0x1000:
			pc = nnn;
			break;

		case 0x2000:
			stack[sp] = pc;
			sp++;
			pc = nnn;
			break;

		case 0x3000:
			if (V[x] == nn) {
				pc+=4;
			} else {
				pc+=2;
			}
			break;

		case 0x4000:
			if (V[x] != nn) {
				pc+=4;
			} else {
				pc+=2;
			}
			break;

		case 0x5000:
			if (V[x] == V[y]) {
				pc+=4;
			} else {
				pc+=2;
			}
			break;

		case 0x6000:
			V[x] = nn;
			pc+=2;
			break;

		case 0x7000:
			V[x] += nn;
			pc+=2;
			break;

		case 0x8000:
			switch (n) {
				case 0x0:
					V[x] = V[y];
					break;

				case 0x1:
					//V[0xF] = 0;
					V[x] = V[x]|V[y];
					break;

				case 0x2:
					//V[0xF] = 0;
					V[x] = V[x]&V[y];
					break;

				case 0x3:
					//V[0xF] = 0;
					V[x] = V[x]^V[y];
					break;

				case 0x4:
					V[0xF] = (V[y] > (0xFF - V[x])) ? 1 : 0;
					V[x] += V[y];
					break;

				case 0x5:
					V[0xF] = (V[y] > V[x]) ? 0 : 1;
					V[x] -= V[y];
					break;

				case 0x6:
					V[0xF] = V[x] & 1;
					V[x] >>= 1;
					break;

				case 0x7:
					V[0xF] = (V[y] > V[x]) ? 1 : 0;
					V[x] = V[y]-V[x];
					break;

				case 0xE:
					V[0xF] = V[x] >> 7;
					V[x] <<= 1;
					break;
			}
			pc+=2;
			break;

		case 0x9000:
			if (V[x] != V[y]) {
				pc+=4;
			} else {
				pc+=2;
			}
			break;

		case 0xA000:
			I = nnn;
			pc+=2;
			break;

		case 0xB000:
			pc = V[0] + nnn;
			break;

		case 0xC000:
			V[x] = (rand() % 256) & nn;
			pc+=2;
			break;

		case 0xD000:
			unsigned short line;

			V[0xF] = 0;
			for (int yline = 0; yline < n; yline++) {
				line = memory[I + yline];

				for (int xline = 0; xline < 8; xline++) {
					if ((line & (0x80 >> xline)) != 0) {
						if (gfx[V[x] + xline + ((V[y] + yline) * 64)] == 1) V[0xF] = 1;

						gfx[(V[x] + xline) + (V[y] + yline) * 64] ^= 1;
					}
				}
			}

			drawFlag = true;
			pc+=2;
			break;

		case 0xE000:
			//printf("%d", V[x]);
			if (nn == 0x009E) {
				if (keys[V[x]] != 0) {
					pc+=4;				
				} else {
					pc+=2;
				}
			} else if (nn == 0x00A1) {
				if (keys[V[x]] == 0) {
					pc+=4;				
				} else {
					pc+=2;
				}
			}
			break;

		case 0xF000:
			switch (nn) {
				case 0x0007:
					V[x] = delay;
					pc+=2;
					break;

				case 0x000A:
					{
					bool kp = false;
					for (int i = 0; i < 16; i++) {
						if (keys[i] != 0) {
							V[x] = keys[i];
							
							kp = true;					
						}
					}
					if (!kp) return;
					pc+=2;
					}
					break;

				case 0x0015:
					delay = V[x];
					pc+=2;
					break;

				case 0x0018:
					sound = V[x];
					pc+=2;
					break;

				case 0x001E:
					V[0xF] = (I + V[x] > 0xFFF) ? 1 : 0;
					I += V[x];
					pc+=2;
					break;

				case 0x0029:
					I = V[x] * 0x5;
					pc+=2;
					break;

				case 0x0033:
					memory[I] = V[x] / 100;
					memory[I+1] = (V[x] / 10) % 10;
					memory[I+2] = (V[x] % 100) % 10;
					pc+=2;					
					break;

				case 0x0055:
					for (int i = 0x0; i < x; i++) {
						memory[I+i] = V[i];
					}
					
					I += x + 1;
					pc+=2;
					break;

				case 0x0065:
					for (int i = 0x0; i < x; i++) {
						V[i] = memory[I+i];
					}

					I += x + 1;
					pc+=2;
					break;
			}
			
			break;
	}

	if (delay > 0) --delay;
	if (sound > 0) {
		if (sound == 1) cout << "[SOUND] Beep!\n";
		--sound;
	}
}

const int SIZE = 16;

const uint8_t keymap[16] = { 
	SDLK_1,
	SDLK_2,
	SDLK_3,
	SDLK_4,

	SDLK_q,
	SDLK_w,
	SDLK_e,
	SDLK_r,

	SDLK_a,
	SDLK_s,
	SDLK_d,
	SDLK_f,

	SDLK_y,
	SDLK_x,
	SDLK_c,
	SDLK_v
};

int main(int argc, char *argv[]) {
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Event event;

	SDL_CreateWindowAndRenderer(64 * SIZE, 32 * SIZE, SDL_WINDOW_SHOWN, &window, &renderer);

	initSystem();
	loadProgram(argv[1]);

	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);

	while (true) {
		SDL_PollEvent(&event);

		if (event.type == SDL_QUIT) {
			break;
		} else if (event.type == SDL_KEYDOWN) {
			//cout << event.key.keysym.sym << "\n";

			for (int i = 0; i < 16; i++) {
				if (keymap[i] == event.key.keysym.sym) {
					keys[i] = 1;
					//cout << i << " is ON\n";
				}
			}	
		} else if (event.type == SDL_KEYUP) {
			for (int i = 0; i < 16; i++) {
				if (keymap[i] == event.key.keysym.sym) {
					keys[i] = 0;
					//cout << i << " is OFF\n";
				}
			}
		}		
		processOpcode();

		
		if (drawFlag) {
			SDL_RenderClear(renderer);
			SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);


			for (int y = 0; y <  32; y++) {
				for (int x = 0; x < 64; x++) {		
					if (gfx[x + y * 64] != 0) drawRect(renderer, x * SIZE, y * SIZE, SIZE, SIZE);
				}
			}

			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderPresent(renderer);

			drawFlag = false;
		}

		SDL_Delay(1000/60);
	}

	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);

	SDL_Quit();
	return 0;
}
