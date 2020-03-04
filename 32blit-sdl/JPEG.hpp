#pragma once
#include "engine/api_private.hpp"
#include "graphics/jpeg.hpp"

blit::JPEGImage blit_decode_jpeg_buffer(const uint8_t *ptr, uint32_t len, blit::AllocateCallback alloc);
blit::JPEGImage blit_decode_jpeg_file(std::string filename, blit::AllocateCallback alloc);