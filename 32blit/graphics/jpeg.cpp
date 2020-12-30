#include "jpeg.hpp"
#include "../engine/api_private.hpp"

namespace blit {

  static uint8_t *alloc_func(size_t len) {
    return new uint8_t[len];
  }

  /**
   * Decode a JPEG image from memory. The resolution of the image should be kept low to avoid running out of memory.
   * May not support all JPEG files due to limitations of the hardware decoder.
   * 
   * \param ptr Pointer to data
   * \param len Length of data
   *
   * \return Decoded image.
   */
  JPEGImage decode_jpeg_buffer(const uint8_t *ptr, uint32_t len) {
    return api.decode_jpeg_buffer(ptr, len, alloc_func);
  }

  /**
   * Decode a JPEG image from a file. See ::decode_jpeg_buffer for limitations.
   * 
   * \param filename File to decode
   *
   * \return Decoded image.
   */
  JPEGImage decode_jpeg_file(const std::string &filename) {
    return api.decode_jpeg_file(filename, alloc_func);
  }
}