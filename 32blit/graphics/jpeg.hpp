#pragma once

#include <stdint.h>
#include <string>

#include "types/size.hpp"

namespace blit {
  struct JPEGImage {
    blit::Size size;
    uint8_t *data;
  };

  JPEGImage decode_jpeg_buffer(uint8_t *ptr, uint32_t len);
  JPEGImage decode_jpeg_file(std::string filename);
}