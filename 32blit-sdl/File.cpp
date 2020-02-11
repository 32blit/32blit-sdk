#include <string>
#include <map>

#include "SDL.h"

static std::string basePath;
static std::map<uint32_t, SDL_RWops *> open_files;
static uint32_t current_file_handle = 0;

void setup_base_path()
{
  auto basePathPtr = SDL_GetBasePath();
  basePath = std::string(basePathPtr);
  SDL_free(basePathPtr);
}

int32_t open_file(std::string name) {
  auto file = SDL_RWFromFile((basePath + name).c_str(), "rb");

  if(file) {
    current_file_handle++;
    open_files[current_file_handle] = file;
    return current_file_handle;
  }

  return -1;
}

int32_t read_file(uint32_t fh, uint32_t offset, uint32_t length, char *buffer) {
  auto file = open_files[fh];

  if(file && SDL_RWseek(file, offset, RW_SEEK_SET) != -1) {
    size_t bytes_read = SDL_RWread(file, buffer, 1, length);

    if(bytes_read > 0)
      return bytes_read;
  }

  return -1;
}

int32_t close_file(uint32_t fh) {
  return SDL_RWclose(open_files[fh]) == 0 ? 0 : -1;
}
