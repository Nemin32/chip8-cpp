#include "emulator.hpp"
#include "renderer.hpp"

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <ratio>

void Chip8::update_timers() {
  if (this->delayTimer > 0)
    delayTimer--;
  if (this->soundTimer > 0) {
    if (this->soundTimer == 1)
      std::cout << "BEEP!" << std::endl;
  }
}

void Chip8::load_fontmap() {
    std::copy(std::begin(FONTSET), std::end(FONTSET), std::begin(memory));
}

void Chip8::load_rom(const std::string& fileName) {
	std::ifstream file(fileName, std::ios::binary);
  file.unsetf(std::ios::skipws);

	if (file.is_open()) {
    int i = 0;

    while (!file.eof()) {
      uint8_t b1 = file.get();
      memory[512+i] = b1;
      i++;
      
      if (i > 4095) {
        std::cerr << "[FILE]: Input file was longer than 4096 bytes. Exiting.";
        exit(1);
      }
    }

		file.close();
	} else {
		std::cout << "[FILE]: ERROR: Could not open file. Exiting.\n";
		exit(1); 
	}
}

void Chip8::tick() {
  uint8_t b1 = this->memory[this->pc + 0];
  uint8_t b2 = this->memory[this->pc + 1];
  uint16_t instruction = (b1 << 8) | b2;

  this->handle_instruction(instruction);
  this->update_timers();
}

void Chip8::handle_instruction(const uint16_t instruction) {
  const uint16_t op  = (instruction & 0xF000) >> (3 * 4);
  const uint16_t x   = (instruction & 0x0F00) >> (2 * 4);
  const uint16_t y   = (instruction & 0x00F0) >> (1 * 4);
  const uint16_t nnn = (instruction & 0x0FFF);
  const uint16_t kk  = (instruction & 0x00FF);
  const uint16_t n   = (instruction & 0x000F);
  
  constexpr int step = 2;
  
  // std::cout << std::hex << "[" << instruction << "] [" << op << "] [" << x << "] [" << y << "] [" << nnn << "] [PC: " << pc << "] [VF: " << (bool)V[0xF] << "]\n";
  
  switch (op) {
    case 0x0: {
      switch (n) {
        case 0x0: framebuffer.fill(0); drawFlag = true; break;
        case 0xE: pc = stack[--sp]; break;
      }
    } break;
    case 0x1: pc = nnn - step; break;
    case 0x2: stack[sp++] = pc; pc = nnn - step; break;
    case 0x3: pc += (V[x] == kk) ? step : 0; break;
    case 0x4: pc += (V[x] != kk) ? step : 0; break;
    case 0x5: pc += (V[x] == V[y]) ? step : 0; break;
    case 0x6: V[x] = kk; break;
    case 0x7: V[x] += kk; break;
    case 0x8: {
      switch (n) {
        case 0x0: V[x] = V[y];  break;
        case 0x1: V[x] |= V[y]; break;
        case 0x2: V[x] &= V[y]; break;
        case 0x3: V[x] ^= V[y]; break;
        case 0x4: V[x] += V[y]; V[0xF] = (V[y] > (0xFF - V[x])) ? 1 : 0; break;
        case 0x5: V[0xF] = (V[x] < V[y]) ? 0 : 1; V[x] -= V[y]; break;
        case 0x6: V[0xF] = V[x] & 0x1; V[x] >>= 1; break;
        case 0x7: V[0xF] = (V[x] < V[y]) ? 0 : 1; V[x] = V[y] - V[x]; break;
        case 0xE: V[0xF] = V[x] >> 7; V[x] <<= 1; break;
      }
    } break;
    case 0x9: pc += (V[x] != V[y]) ? step : 0; break;
    case 0xA: I = nnn; break;
    case 0xB: pc = V[0] + nnn - step; break;
    case 0xC: V[x] = (rand() % 256) & kk; break;
    case 0xD: {
      uint16_t line;

      V[0xF] = 0;
      drawFlag = true;
      for (int yline = 0; yline < n; yline++) {
        line = memory[I + yline];

        for (int xline = 0; xline < 8; xline++) {
          if ((line & (0x80 >> xline)) != 0) {
            const int position = V[x] + xline + ((V[y] + yline) * 64);

            V[0xF] |= (framebuffer[position] == 1);
            framebuffer[position] ^= 1;
          }
        }
      }
    } break;
    case 0xE: {
      switch (kk) {
        case 0x9E: pc += (keys[V[x]] != 0) ? step : 0; break;
        case 0xA1: pc += (keys[V[x]] == 0) ? step : 0; break;
      }
    } break;
    case 0xF: {
      switch (kk) {
        case 0x07: V[x] = delayTimer; break;
        case 0x0A: {
          pc -= step;
          int i = 0;
          while (i < KEYMAP_SIZE && keys[i] == 0) i++;
          if (i < KEYMAP_SIZE) {
            V[x] = i;
            pc+=step;
          }
        } break;
        case 0x15: delayTimer = V[x]; break;
        case 0x18: soundTimer = V[x]; break;
        case 0x1E: V[0xF] = (I + V[x] > 0xFFF) ? 1 : 0; I += V[x]; break;
        case 0x29: I = V[x] * 0x5; break;
        case 0x33: {
          memory[I + 0] = V[x] / 100;
          memory[I + 1] = (V[x] / 10) % 10;
          memory[I + 2] = (V[x] % 100) % 10;
        } break;
        case 0x55: for (int i = 0; i <= x; i++) memory[I+i] = V[i]; I += x+1; break;
        case 0x65: for (int i = 0; i <= x; i++) V[i] = memory[I+i]; I += x+1; break;
      }
    } break;
  }
  
  pc+=2;
}