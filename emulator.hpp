#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

constexpr size_t FRAMEBUFFER_SIZE = 64*32;

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

private:
  void load_fontmap();
  void load_rom(const std::string& fileName);
  void handle_instruction(const uint16_t instruction);

  // Memory
  std::array<uint8_t, FRAMEBUFFER_SIZE> framebuffer;
  std::array<uint16_t, 4096> memory;
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