#pragma once
#include <cstdint>

constexpr uint32_t blit_game_magic = 0x54494C42; // "BLIT"

using BlitRenderFunction = void(*)(uint32_t);
using BlitTickFunction = bool(*)(uint32_t);
using BlitInitFunction = bool(*)(uint32_t);

// should match the layout in startup_user.s
struct BlitGameHeader {
  uint32_t magic;

  BlitRenderFunction render;
  BlitTickFunction tick;
  BlitInitFunction init;

  uint32_t end;
  uint32_t start;
};

// missing the "BLITMETA" header and size
struct RawMetadata {
  uint32_t crc32;
  char datetime[16];
  char title[25];
  char description[129];
  char version[17];
  char author[17];
};