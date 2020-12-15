#include <cstdint>

#include "SDL.h"
#include "SDL_image.h"

#include "JPEG.hpp"

static blit::JPEGImage decode_jpeg_rwops(SDL_RWops *rwops)
{
  auto image = IMG_LoadJPG_RW(rwops);
  SDL_RWclose(rwops);
  blit::JPEGImage ret = {};

  if(!image)
    return ret;

  ret.size = blit::Size(image->w, image->h);

  // FIXME: assuming surface is RGB
  ret.data = new uint8_t[image->w * image->h * 3];
  memcpy(ret.data, image->pixels, image->w * image->h * 3);

  SDL_FreeSurface(image);

  return ret;
}

// these don't bother using the allocation callback since there's only one heap anyway
blit::JPEGImage blit_decode_jpeg_buffer(const uint8_t *ptr, uint32_t len, blit::AllocateCallback alloc) {
  auto rwops = SDL_RWFromConstMem(ptr, len);
  return decode_jpeg_rwops(rwops);
}

blit::JPEGImage blit_decode_jpeg_file(const std::string &filename, blit::AllocateCallback alloc) {
  auto rwops = SDL_RWFromFile(filename.c_str(), "rb");
  return decode_jpeg_rwops(rwops);
}
