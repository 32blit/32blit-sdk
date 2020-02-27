#include <cstdint>

#include "SDL.h"
#include "SDL_image.h"

#include "graphics/jpeg.hpp"

namespace blit {

  static JPEGImage decode_jpeg_rwops(SDL_RWops *rwops)
  {
    auto image = IMG_LoadJPG_RW(rwops);
    SDL_RWclose(rwops);
    JPEGImage ret = {};

    if(!image)
      return ret;

    ret.size = Size(image->w, image->h);

    // FIXME: assuming surface is RGB
    ret.data = new uint8_t[image->w * image->h * 3];
    memcpy(ret.data, image->pixels, image->w * image->h * 3);

    SDL_FreeSurface(image);

    return ret;
  }

  JPEGImage decode_jpeg_buffer(uint8_t *ptr, uint32_t len) {
    auto rwops = SDL_RWFromMem(ptr, len);
    return decode_jpeg_rwops(rwops);
  }

  JPEGImage decode_jpeg_file(std::string filename) {
    auto rwops = SDL_RWFromFile(filename.c_str(), "rb");
    return decode_jpeg_rwops(rwops);
  }
}