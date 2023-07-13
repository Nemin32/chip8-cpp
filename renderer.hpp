#pragma once

#include <cstddef>
#include <cstdint>
#include <SDL2/SDL.h>

constexpr int SIZE = 16;
constexpr size_t KEYMAP_SIZE = 16;
constexpr uint8_t keymap[KEYMAP_SIZE] = { 
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