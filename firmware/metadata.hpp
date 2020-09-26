#pragma once
#include <cstdint>
#include <string>

#include "graphics/surface.hpp"

struct BlitGameMetadata {
  uint16_t length = 0;
  std::string title, description, version;

  blit::Surface *icon = nullptr, *splash = nullptr;
};

bool parse_flash_metadata(uint32_t offset, BlitGameMetadata &metadata, bool unpack_images = false);
bool parse_file_metadata(const std::string &filename, BlitGameMetadata &metadata, bool unpack_images = false);