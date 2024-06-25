#pragma once

#include <functional>

#include "executable.hpp"
#include "engine/api_private.hpp"

// this is the 32blit's flash erase size, some parts of the API depend on this...
static constexpr unsigned int game_block_size = 64 * 1024;

RawMetadata *get_running_game_metadata();

bool launch_file(const char *path);
blit::CanLaunchResult can_launch(const char *path);
void delayed_launch();

void list_installed_games(std::function<void(const uint8_t *, uint32_t, uint32_t)> callback);

class BlitWriter final {
public:
  void init(uint32_t file_len);

  bool write(const uint8_t *buf, uint32_t len);

  uint32_t get_remaining() const;
  uint32_t get_flash_offset() const;

private:
  bool prepare_write(const uint8_t *buf);

  uint32_t file_len;
  uint32_t file_offset;

  uint32_t flash_offset;
};