#include <string>
#include <map>

#ifdef WIN32
#include "shlobj.h"
#else
#include <dirent.h>
#endif

#include "SDL.h"

#include "File.hpp"

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

uint32_t get_file_length(uint32_t fh)
{
  auto file = open_files[fh];
  SDL_RWseek(file, 0, RW_SEEK_END);

  return SDL_RWtell(file);
}

std::vector<blit::FileInfo> list_files(std::string path) {
  std::vector<blit::FileInfo> ret;

#ifdef WIN32
  HANDLE file;
  WIN32_FIND_DATAA findData;
  file = FindFirstFileA((basePath + path + "\\*").c_str(), &findData);

  if(file == INVALID_HANDLE_VALUE)
    return ret;

  do
  {
    blit::FileInfo info;
    info.name = findData.cFileName;

    if(info.name == "." || info.name == "..")
      continue;

    info.flags = 0;

    if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      info.flags |= blit::FileFlags::directory;

    ret.push_back(info);
  }
  while(FindNextFileA(file, &findData) != 0);

  FindClose(file);

#else
  auto dir = opendir((basePath + path).c_str());

  if(!dir)
    return ret;

  struct dirent *ent;

  while((ent = readdir(dir))) {
    blit::FileInfo info;

    info.name = ent->d_name;

    if(info.name == "." || info.name == "..")
      continue;

    info.flags = 0;

    if(ent->d_type == DT_DIR)
      info.flags |= blit::FileFlags::directory;

    ret.push_back(info);
  }

  closedir(dir);
#endif

  return ret;
}