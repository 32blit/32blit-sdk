#pragma once
#include <cstdint>
#include <string>

#include "graphics/surface.hpp"

struct BlitGameMetadata {
  uint16_t length = 0;
  std::string title, description, version, author;

  blit::Surface *icon = nullptr, *splash = nullptr;

  void free_surfaces() {
    if(icon) {
      delete[] icon->data;
      delete[] icon->palette;
      delete icon;
      icon = nullptr;
    }

    if(splash) {
      delete[] splash->data;
      delete[] splash->palette;
      delete splash;
      splash = nullptr;
    }
  }
};

bool parse_flash_metadata(uint32_t offset, BlitGameMetadata &metadata, bool unpack_images = false);
bool parse_file_metadata(const std::string &filename, BlitGameMetadata &metadata, bool unpack_images = false);