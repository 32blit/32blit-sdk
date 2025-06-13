#pragma once
#include <cstdint>

constexpr uint32_t blit_game_magic = 0x54494C42; // "BLIT"

#if defined(TARGET_32BLIT_HW) || defined(PICO_BUILD) // TODO: generic "is hardware" define?
using BlitRenderFunction = void(*)(uint32_t);
using BlitTickFunction = int(*)(uint32_t);
using BlitInitFunction = bool(*)(uint32_t);
#else
using BlitRenderFunction = uint32_t;
using BlitTickFunction = uint32_t;
using BlitInitFunction = uint32_t;
#endif

enum class BlitDevice : uint8_t {
  STM32H7_32BlitOld = 0, // 32blit hw, old header
  STM32H7_32Blit = 1, // 32blit hw
  RP2040 = 2, // any RP2040-based device
  RP2350 = 3,
};

// should match the layout in startup_user.s
struct BlitGameHeader {
  uint32_t magic;

  BlitRenderFunction render;
  BlitTickFunction tick;
  BlitInitFunction init;

  uint32_t end;

  BlitDevice device_id;
  uint8_t unused[3];

  // if device_id != 0
  uint16_t api_version_major;
  uint16_t api_version_minor;
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

// "BLITTYPE"
struct RawTypeMetadata {
  char category[17];
  char url[129];
  uint8_t num_filetypes;
  char filetypes[][5];
};
