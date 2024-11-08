#include "jpeg.hpp"
#include "../engine/api_private.hpp"

namespace blit {

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
    return {{0, 0}, nullptr};
  }

  /**
   * Decode a JPEG image from a file. See ::decode_jpeg_buffer for limitations.
   *
   * \param filename File to decode
   *
   * \return Decoded image.
   */
  JPEGImage decode_jpeg_file(const std::string &filename) {
    return {{0, 0}, nullptr};
  }
}
