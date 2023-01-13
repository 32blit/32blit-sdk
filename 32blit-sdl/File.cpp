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
#include "UserCode.hpp"

static std::string base_path, save_path;

static std::string map_path(const std::string &path) {
  // check if the path is under the save path
  if(path.compare(0, save_path.length(), save_path) == 0)
    return path;

  // otherwise it should be under the base path
  return base_path + path;
}

void setup_base_path() {
  auto base_path_str = SDL_GetBasePath();
  if(base_path_str)
    base_path = std::string(base_path_str);
  SDL_free(base_path_str);

  auto tmp = SDL_GetPrefPath(metadata_author, metadata_title);
  if(tmp)
    save_path = std::string(tmp);

  SDL_free(tmp);
}

void *open_file(const std::string &name, int mode) {
  const char *str_mode;

  if(mode == blit::OpenMode::read)
    str_mode = "rb";
  else if(mode == blit::OpenMode::write)
    str_mode = "wb";
  else if(mode == (blit::OpenMode::read | blit::OpenMode::write))
    str_mode = "r+";
  else
    return nullptr;

  auto file = SDL_RWFromFile(map_path(name).c_str(), str_mode);

  return file;
}

int32_t read_file(void *fh, uint32_t offset, uint32_t length, char *buffer) {
  auto file = (SDL_RWops *)fh;

  if(file && SDL_RWseek(file, offset, RW_SEEK_SET) != -1) {
    SDL_ClearError();
    size_t bytes_read = SDL_RWread(file, buffer, 1, length);

    if(!SDL_GetError()[0])
      return (int32_t)bytes_read;
  }

  return -1;
}

int32_t write_file(void *fh, uint32_t offset, uint32_t length, const char *buffer) {
  auto file = (SDL_RWops *)fh;

  if(file && SDL_RWseek(file, offset, RW_SEEK_SET) != -1) {
    size_t bytes_written = SDL_RWwrite(file, buffer, 1, length);

    if(bytes_written > 0)
      return (int32_t)bytes_written;
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

  return (uint32_t)SDL_RWtell(file);
}

void list_files(const std::string &path, std::function<void(blit::FileInfo &)> callback) {
#ifdef WIN32
  HANDLE file;
  WIN32_FIND_DATAA findData;
  file = FindFirstFileA((map_path(path) + "\\*").c_str(), &findData);

  if(file == INVALID_HANDLE_VALUE)
    return;

  do
  {
    blit::FileInfo info;
    info.name = findData.cFileName;

    if(info.name == "." || info.name == "..")
      continue;

    info.flags = 0;
    info.size = findData.nFileSizeLow;

    if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      info.flags |= blit::FileFlags::directory;

    callback(info);
  }
  while(FindNextFileA(file, &findData) != 0);

  FindClose(file);

#else
  auto mapped_path = map_path(path);
  auto dir = opendir(mapped_path.c_str());

  if(!dir)
    return;

  struct dirent *ent;

  while((ent = readdir(dir))) {
    blit::FileInfo info;

    info.name = ent->d_name;

    if(info.name == "." || info.name == "..")
      continue;

    struct stat stat_buf;

    if(stat((mapped_path + "/" + info.name).c_str(), &stat_buf) < 0)
      continue;

    info.flags = 0;
    info.size = stat_buf.st_size;

    if(S_ISDIR(stat_buf.st_mode))
      info.flags |= blit::FileFlags::directory;

    callback(info);
  }

  closedir(dir);
#endif
}

bool file_exists(const std::string &path) {
#ifdef WIN32
	DWORD attribs = GetFileAttributesA(map_path(path).c_str());
	return (attribs != INVALID_FILE_ATTRIBUTES && !(attribs & FILE_ATTRIBUTE_DIRECTORY));
#else
  struct stat stat_buf;
  return (stat(map_path(path).c_str(), &stat_buf) == 0 && S_ISREG(stat_buf.st_mode));
#endif
}

bool directory_exists(const std::string &path) {
#ifdef WIN32
	DWORD attribs = GetFileAttributesA(map_path(path).c_str());
	return (attribs != INVALID_FILE_ATTRIBUTES && (attribs & FILE_ATTRIBUTE_DIRECTORY));
#else
  struct stat stat_buf;
  return (stat(map_path(path).c_str(), &stat_buf) == 0 && S_ISDIR(stat_buf.st_mode));
#endif
}

bool create_directory(const std::string &path) {
#ifdef WIN32
  return _mkdir(map_path(path).c_str()) == 0 || errno == EEXIST;
#else
  return mkdir(map_path(path).c_str(), 0755) == 0 || errno == EEXIST;
#endif
}

bool rename_file(const std::string &old_name, const std::string &new_name) {
  return rename(map_path(old_name).c_str(), map_path(new_name).c_str()) == 0;
}

bool remove_file(const std::string &path) {
  return remove(map_path(path).c_str()) == 0;
}

const char *get_save_path() {
  return save_path.c_str();
}

bool is_storage_available() {
  return true;
}
