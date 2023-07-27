#include <cstring>

#include "pico/stdlib.h"

#include "blit-launch.hpp"
#include "file.hpp"

#include "engine/engine.hpp"

// code related to blit files and launching

extern int (*do_tick)(uint32_t time);

static const unsigned int game_block_size = 64 * 1024; // this is the 32blit's flash erase size, some parts of the API depend on this...

static uint32_t requested_launch_offset = 0;
static uint32_t current_game_offset = 0;

RawMetadata *get_running_game_metadata() {
#ifdef BUILD_LOADER
  if(!current_game_offset)
    return nullptr;

  auto game_ptr = reinterpret_cast<uint8_t *>(XIP_NOCACHE_NOALLOC_BASE + current_game_offset);

  auto header = reinterpret_cast<BlitGameHeader *>(game_ptr);

  if(header->magic == blit_game_magic) {
    auto end_ptr = game_ptr + header->end;
    if(memcmp(end_ptr, "BLITMETA", 8) == 0)
      return reinterpret_cast<RawMetadata *>(end_ptr + 10);
  }

#endif
  return nullptr;
}

bool launch_file(const char *path) {
  if(strncmp(path, "flash:/", 7) == 0) {
    uint32_t offset = atoi(path + 7) * game_block_size;

    auto header = (BlitGameHeader *)(XIP_NOCACHE_NOALLOC_BASE + offset);
    // check header magic + device
    if(header->magic != blit_game_magic || header->device_id != BlitDevice::RP2040)
      return false;

    if(!header->init || !header->render || !header->tick)
      return false;

    requested_launch_offset = offset;
    return true;
  }

  return false;
}

blit::CanLaunchResult can_launch(const char *path) {
  if(strncmp(path, "flash:/", 7) == 0) {
    // assume anything flashed is compatible for now
    return blit::CanLaunchResult::Success;
  }

  return blit::CanLaunchResult::UnknownType;
}

void delayed_launch() {
  if(!requested_launch_offset)
    return;

  auto header = (BlitGameHeader *)(XIP_NOCACHE_NOALLOC_BASE + requested_launch_offset);

  // save in case launch fails
  uint32_t last_game_offset = current_game_offset;

  current_game_offset = requested_launch_offset;
  requested_launch_offset = 0;

  if(!header->init(0)) {
    blit::debugf("failed to init game!\n");
    current_game_offset = last_game_offset;
    return;
  }

  blit::render = header->render;
  do_tick = header->tick;
}

void list_installed_games(std::function<void(const uint8_t *, uint32_t, uint32_t)> callback) {
  for(uint32_t off = 0; off < PICO_FLASH_SIZE_BYTES;) {
    auto header = (BlitGameHeader *)(XIP_NOCACHE_NOALLOC_BASE + off);

    // check header magic + device
    if(header->magic != blit_game_magic || header->device_id != BlitDevice::RP2040) {
      off += game_block_size;
      continue;
    }

    auto size = header->end;

    // check metadata
    auto meta_offset = off + size;
    if(memcmp((char *)(XIP_NOCACHE_NOALLOC_BASE + meta_offset), "BLITMETA", 8) != 0) {
      off += ((size - 1) / game_block_size + 1) * game_block_size;
      continue;
    }

    // add metadata size
    size += *(uint16_t *)(XIP_NOCACHE_NOALLOC_BASE + meta_offset + 8) + 10;

    callback((const uint8_t *)(XIP_NOCACHE_NOALLOC_BASE + off), off / game_block_size, size);

    off += ((size - 1) / game_block_size + 1) * game_block_size;
  }
}