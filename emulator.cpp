#include "emulator.hpp"
#include <cstdint>
#include <iostream>
#include <fstream>

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
	std::ifstream file(fileName, std::ios::binary | std::ios::ate);

	if (file.is_open()) {
		int size = file.tellg();
		file.clear(); file.seekg(0, std::ios::beg); //Go to beginning

		for (int i = 0; i < size; i++) {
			file >> std::noskipws >> memory[i + 512];
		}

		file.close();
	} else {
		std::cout << "[FILE]: ERROR: Could not open file. Exiting.\n";
		exit(1); 
	}
}

void Chip8::tick() {
  uint16_t instruction = this->memory[this->pc];
  instruction = (instruction & 0xFF00 >> 8) | (instruction & 0xFF << 8);

  this->handle_instruction(instruction);
  this->update_timers();
}

void Chip8::handle_instruction(const uint16_t instruction) {
  const uint8_t op = (instruction & 0xF000) >> (3 * 4);
  const uint8_t x = (instruction & 0x0F00) >> (2 * 4);
  const uint8_t y = (instruction & 0xF0) >> (1 * 4);
  const uint16_t nnn = instruction & 0x0FFF;
  const uint16_t kk = instruction & 0xFF;
  const uint16_t n = instruction & 0x0F;

  switch (op) {
  case 0x0:
    if (n == 0x0) {
      for (int i = 0; i < FRAMEBUFFER_SIZE; i++)
        framebuffer[i] = 0;
      drawFlag = true;
    } else if (n == 0xE) {
      --sp;
      pc = stack[sp];
    }

    pc += 2;
    break;

  case 0x1:
    pc = nnn;
    break;

  case 0x2:
    stack[sp] = pc;
    ++sp;
    pc = nnn;
    break;

  case 0x3:
    pc += (V[x] == kk) ? 4 : 2;
    break;

  case 0x4:
    pc += (V[x] != kk) ? 4 : 2;
    break;

  case 0x5:
    pc += (V[x] != V[y]) ? 4 : 2;
    break;

  case 0x6:
    V[x] = kk;
    pc += 2;
    break;

  case 0x7:
    V[x] += kk;
    pc += 2;
    break;

  case 0x8:
    switch (n) {
    case 0x0:
      V[x] = V[y];
      break;

    case 0x1:
      V[x] |= V[y];
      break;

    case 0x2:
      V[x] &= V[y];
      break;

    case 0x3:
      V[x] ^= V[y];
      break;

    case 0x4:
      V[x] += V[y];
      V[0xF] = (V[y] > (0xFF - V[x])) ? 1 : 0;
      break;

    case 0x5:
      V[0xF] = (V[y] > V[x]) ? 0 : 1;
      V[x] -= V[y];
      break;

    case 0x6:
      V[0xF] = V[x] & 0x1;
      V[x] >>= 1;
      break;

    case 0x7:
      V[0xF] = (V[x] > V[y]) ? 0 : 1;
      V[x] = V[y] - V[x];
      break;

    case 0xE:
      V[0xF] = V[x] >> 7;
      V[x] <<= 1;
      break;
    }
    pc += 2;
    break;

  case 0x9:
    if (V[x] != V[y]) {
      pc += 4;
    } else {
      pc += 2;
    }
    break;

  case 0xA:
    I = nnn;
    pc += 2;
    break;

  case 0xB:
    pc = V[0] + nnn;
    break;

  case 0xC:
    V[x] = (rand() % 256) & kk;
    pc += 2;
    break;

  case 0xD: {
    unsigned short line;

    V[0xF] = 0;
    for (int yline = 0; yline < n; yline++) {
      line = memory[I + yline];

      for (int xline = 0; xline < 8; xline++) {
        if ((line & (0x80 >> xline)) != 0) {
          if (framebuffer[V[x] + xline + ((V[y] + yline) * 64)] == 1)
            V[0xF] = 1;

          framebuffer[(V[x] + xline) + (V[y] + yline) * 64] ^= 1;
        }
      }
    }
  }

    drawFlag = true;
    pc += 2;
    break;

  case 0xE:
    // printf("%d", V[x]);
    if (kk == 0x9E) {
      if (keys[V[x]] != 0) {
        pc += 4;
      } else {
        pc += 2;
      }
    } else if (kk == 0xA1) {
      if (keys[V[x]] == 0) {
        pc += 4;
      } else {
        pc += 2;
      }
    }
    break;

  case 0xF:
    switch (kk) {
    case 0x07:
      V[x] = delayTimer;
      pc += 2;
      break;

    case 0x0A: {
      bool kp = false;
      for (int i = 0; i < 16; i++) {
        if (keys[i] != 0) {
          V[x] = i;

          kp = true;
        }
      }
      if (!kp)
        return;
      pc += 2;
    } break;

    case 0x15:
      delayTimer = V[x];
      pc += 2;
      break;

    case 0x18:
      soundTimer = V[x];
      pc += 2;
      break;

    case 0x1E:
      V[0xF] = (I + V[x] > 0xFFF) ? 1 : 0;
      I += V[x];
      pc += 2;
      break;

    case 0x29:
      I = V[x] * 0x5;
      pc += 2;
      break;

    case 0x33:
      memory[I] = V[x] / 100;
      memory[I + 1] = (V[x] / 10) % 10;
      memory[I + 2] = (V[x] % 100) % 10;
      pc += 2;
      break;

    case 0x55:
      for (int i = 0x0; i <= x; i++) {
        memory[I + i] = V[i];
      }

      I += x + 1;
      pc += 2;
      break;

    case 0x65:
      for (int i = 0x0; i <= x; i++) {
        V[i] = memory[I + i];
      }

      I += x + 1;
      pc += 2;
      break;
    }

    break;
  }
}