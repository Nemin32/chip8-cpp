#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

constexpr size_t FRAMEBUFFER_SIZE = 64*32;
constexpr uint8_t FONTSET[80] =
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

class Chip8 {
public:
  Chip8(const std::string& fileName)
      : pc(0x200), I(0), sp(0), delayTimer(0), soundTimer(0), drawFlag(false) {
    memory.fill(0);
    framebuffer.fill(0);
    V.fill(0);
    stack.fill(0);
    keys.fill(false);
    
    this->load_fontmap();
    this->load_rom(fileName);
  }
  
  void tick();
  bool should_draw() const {return this->drawFlag;}
  void update_timers();

// private:
  void load_fontmap();
  void load_rom(const std::string& fileName);
  void handle_instruction(const uint16_t instruction);

  // Memory
  std::array<uint8_t, FRAMEBUFFER_SIZE> framebuffer;
  std::array<uint8_t, 4096> memory;
  std::array<uint8_t, 16> V;
  std::array<bool, 16> keys;

  // Stack
  std::array<uint16_t, 16> stack;
  uint16_t sp;

  // Not directly addressable registers
  uint16_t I;
  uint16_t pc;

  // Timers
  uint8_t delayTimer;
  uint8_t soundTimer;

  bool drawFlag;
};