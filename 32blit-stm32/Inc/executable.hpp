#pragma once
#include <cstdint>

constexpr uint32_t blit_game_magic = 0x54494C42; // "BLIT"

using BlitRenderFunction = void(*)(uint32_t);
using BlitTickFunction = bool(*)(uint32_t);
using BlitInitFunction = void(*)();

// should match the layout in startup_user.s
struct BlitGameHeader {
  uint32_t magic;

  BlitRenderFunction render;
  BlitTickFunction tick;
  BlitInitFunction init;

  uint32_t start;
  uint32_t end;
};