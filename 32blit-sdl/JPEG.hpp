#pragma once
#include "graphics/jpeg.hpp"

blit::JPEGImage blit_decode_jpeg_buffer(uint8_t *ptr, uint32_t len);
blit::JPEGImage blit_decode_jpeg_file(std::string filename);