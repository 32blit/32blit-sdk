#pragma once

#include <cstdint>
#include <string>

#include "../types/size.hpp"

namespace blit {
  struct JPEGImage {
    blit::Size size;
    /// Raw RGB image data
    uint8_t *data;
  };

  JPEGImage decode_jpeg_buffer(const uint8_t *ptr, uint32_t len);
  JPEGImage decode_jpeg_file(const std::string &filename);
}