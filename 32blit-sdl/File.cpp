#include <cerrno>
#include <string>
#include <map>

#ifdef WIN32
#include <direct.h>
#include <shlobj.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

#include "SDL.h"

#include "File.hpp"

static std::string basePath;

void setup_base_path()
{
  auto basePathPtr = SDL_GetBasePath();
  basePath = std::string(basePathPtr);
  SDL_free(basePathPtr);
}

void *open_file(std::string name) {
  auto file = SDL_RWFromFile((basePath + name).c_str(), "rb");
  return file;
}

int32_t read_file(void *fh, uint32_t offset, uint32_t length, char *buffer) {
  auto file = (SDL_RWops *)fh;

  if(file && SDL_RWseek(file, offset, RW_SEEK_SET) != -1) {
    size_t bytes_read = SDL_RWread(file, buffer, 1, length);

    if(bytes_read > 0)
      return bytes_read;
  }

  return -1;
}

int32_t close_file(void *fh) {
  return SDL_RWclose((SDL_RWops *)fh) == 0 ? 0 : -1;
}

uint32_t get_file_length(void *fh)
{
  auto file = (SDL_RWops *)fh;
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

    if(ent->d_type == DT_LNK) {
      // lookup link target
      struct stat stat_buf;
  
      if(stat((basePath + path + "/" + info.name).c_str(), &stat_buf) >= 0 && S_ISDIR(stat_buf.st_mode))
        info.flags |= blit::FileFlags::directory;
    } else if(ent->d_type == DT_DIR)
      info.flags |= blit::FileFlags::directory;

    ret.push_back(info);
  }

  closedir(dir);
#endif

  return ret;
}

bool create_directory(std::string path) {
#ifdef WIN32
  return _mkdir((basePath + path).c_str()) == 0 || errno == EEXIST;
#else
  return mkdir((basePath + path).c_str(), 0755) == 0 || errno == EEXIST;
#endif
}