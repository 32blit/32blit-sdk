#include "jpeg.hpp"
#include "../engine/api_private.hpp"

namespace blit {
  JPEGImage decode_jpeg_buffer(const uint8_t *ptr, uint32_t len) {
    return api.decode_jpeg_buffer(ptr, len);
  }
  JPEGImage decode_jpeg_file(std::string filename) {
    return api.decode_jpeg_file(filename);
  }
}