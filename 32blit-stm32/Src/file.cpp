#include <cstdint>
#include <map>

#include "fatfs.h"

#include "file.hpp"

void *open_file(std::string file, int mode) {
  FIL *f = new FIL();

  BYTE ff_mode = 0;

  if(mode & blit::OpenMode::read)
    ff_mode |= FA_READ;

  if(mode & blit::OpenMode::write)
    ff_mode |= FA_WRITE;

  if(mode == blit::OpenMode::write)
    ff_mode |= FA_CREATE_ALWAYS;

  FRESULT r = f_open(f, file.c_str(), ff_mode);

  if(r == FR_OK)
    return f;
  
  delete f;
  return nullptr;
}

int32_t read_file(void *fh, uint32_t offset, uint32_t length, char *buffer) {  
  FRESULT r = FR_OK;
  FIL *f = (FIL *)fh;

  if(offset != f_tell(f))
    r = f_lseek(f, offset);

  if(r == FR_OK){ 
    unsigned int bytes_read;
    r = f_read(f, buffer, length, &bytes_read);
    if(r == FR_OK){ 
      return bytes_read;
    }
  }
  
  return -1;
}

int32_t write_file(void *fh, uint32_t offset, uint32_t length, const char *buffer) {  
  FRESULT r = FR_OK;
  FIL *f = (FIL *)fh;

  if(offset != f_tell(f))
    r = f_lseek(f, offset);

  if(r == FR_OK) {
    unsigned int bytes_written;
    r = f_write(f, buffer, length, &bytes_written);
    if(r == FR_OK) {
      return bytes_written;
    }
  }

  return -1;
}

int32_t close_file(void *fh) {
  FRESULT r;

  r = f_close((FIL *)fh);

  delete (FIL *)fh;
  return r == FR_OK ? 0 : -1;
}

uint32_t get_file_length(void *fh)
{
  return f_size((FIL *)fh);
}

std::vector<blit::FileInfo> list_files(std::string path) {
  std::vector<blit::FileInfo> ret;

  auto dir = new DIR();

  if(f_opendir(dir, path.c_str()) != FR_OK)
    return ret;

  FILINFO ent;

  while(f_readdir(dir, &ent) == FR_OK && ent.fname[0]) {
    blit::FileInfo info;

    info.name = ent.fname;
    info.flags = 0;

    if(ent.fattrib & AM_DIR)
      info.flags |= blit::FileFlags::directory;

    ret.push_back(info);
  }

  f_closedir(dir);

  return ret;
}

bool file_exists(std::string path) {
  FILINFO info;
  return f_stat(path.c_str(), &info) == FR_OK && !(info.fattrib & AM_DIR);
}

bool directory_exists(std::string path) {
  FILINFO info;
  return f_stat(path.c_str(), &info) == FR_OK && (info.fattrib & AM_DIR);
}

bool create_directory(std::string path) {
  // strip trailing slash
  if(path.back() == '/')
    path = path.substr(0, path.length() - 1);

  FRESULT r = f_mkdir(path.c_str());

  return r == FR_OK || r == FR_EXIST;
}