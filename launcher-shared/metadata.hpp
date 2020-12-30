#pragma once
#include <cstdint>
#include <string>

#include "graphics/surface.hpp"

struct BlitGameMetadata {
  uint16_t length = 0;
  uint32_t crc32 = 0;
  std::string title, description, version, author, category;

  std::vector<std::string> filetypes;

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

void parse_metadata(char *data, uint16_t metadata_len, BlitGameMetadata &metadata, bool unpack_images);
