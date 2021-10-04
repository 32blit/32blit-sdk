#include <cstdint>
#include <cstring>
#include <functional>
#include <vector>

#include "pico/binary_info.h"

#include "ff.h"
#include "diskio.h"

#include "file.hpp"
#include "storage.hpp"

static FATFS fs;
static bool initialised = false;

std::vector<void *> open_files;

// fatfs io funcs
DSTATUS disk_initialize(BYTE pdrv) {
  initialised = storage_init();
  return initialised ? RES_OK : STA_NOINIT;
}

DSTATUS disk_status(BYTE pdrv) {
  return initialised ? RES_OK : STA_NOINIT;
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count) {
  static_assert(FF_MIN_SS == FF_MAX_SS);
  return storage_read(sector, 0, buff, FF_MIN_SS * count) == int32_t(FF_MIN_SS * count) ? RES_OK : RES_ERROR;
}

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {
  return storage_write(sector, 0, buff, FF_MIN_SS * count) == int32_t(FF_MIN_SS * count) ? RES_OK : RES_ERROR;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff) {
  uint16_t block_size;
  uint32_t num_blocks;

  switch(cmd) {
    case CTRL_SYNC:
      return RES_OK;

    case GET_SECTOR_COUNT:
      get_storage_size(block_size, num_blocks);
      *(LBA_t *)buff = num_blocks;
      return RES_OK;

    case GET_BLOCK_SIZE:
      *(DWORD *)buff = 1;
      return RES_OK;
  }

  return RES_PARERR;
}

void init_fs() {
  auto res = f_mount(&fs, "", 1);

  if(res == FR_NO_FILESYSTEM) {
    printf("No filesystem found, formatting...\n");

    MKFS_PARM opts{};
    opts.fmt = FM_ANY | FM_SFD;
    res = f_mkfs("", &opts, fs.win, FF_MAX_SS);

    if(res != FR_OK) {
      printf("...failed! (%i)\n", res);
      return;
    }

    res = f_mount(&fs, "", 1);
  }

  if(res != FR_OK)
    printf("Failed to mount filesystem! (%i)\n", res);
}

bool get_files_open() {
  return open_files.size() > 0;
}

void close_open_files() {
  while(!open_files.empty())
    close_file(open_files.back());
}

void *open_file(const std::string &file, int mode) {
  FIL *f = new FIL();

  BYTE ff_mode = 0;

  if(mode & blit::OpenMode::read)
    ff_mode |= FA_READ;

  if(mode & blit::OpenMode::write)
    ff_mode |= FA_WRITE;

  if(mode == blit::OpenMode::write)
    ff_mode |= FA_CREATE_ALWAYS;

  FRESULT r = f_open(f, file.c_str(), ff_mode);

  if(r == FR_OK) {
    open_files.push_back(f);
    return f;
  }

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

  for(auto it = open_files.begin(); it != open_files.end(); ++it) {
    if(*it == fh) {
      open_files.erase(it);
      break;
    }
  }

  delete (FIL *)fh;
  return r == FR_OK ? 0 : -1;
}

uint32_t get_file_length(void *fh) {
  return f_size((FIL *)fh);
}

void list_files(const std::string &path, std::function<void(blit::FileInfo &)> callback) {
  DIR dir;

  if(f_opendir(&dir, path.c_str()) != FR_OK)
    return;

  FILINFO ent;

  while(f_readdir(&dir, &ent) == FR_OK && ent.fname[0]) {
    blit::FileInfo info;

    info.name = ent.fname;
    info.flags = 0;
    info.size = ent.fsize;

    if(ent.fattrib & AM_DIR)
      info.flags |= blit::FileFlags::directory;

    callback(info);
  }

  f_closedir(&dir);
}

bool file_exists(const std::string &path) {
  FILINFO info;
  return f_stat(path.c_str(), &info) == FR_OK && !(info.fattrib & AM_DIR);
}

bool directory_exists(const std::string &path) {
  FILINFO info;
  return f_stat(path.c_str(), &info) == FR_OK && (info.fattrib & AM_DIR);
}

bool create_directory(const std::string &path) {
  FRESULT r;

  // strip trailing slash
  if(path.back() == '/')
     r = f_mkdir(path.substr(0, path.length() - 1).c_str());
  else
    r = f_mkdir(path.c_str());

  return r == FR_OK || r == FR_EXIST;
}

bool rename_file(const std::string &old_name, const std::string &new_name) {
  return f_rename(old_name.c_str(), new_name.c_str()) == FR_OK;
}

bool remove_file(const std::string &path) {
  return f_unlink(path.c_str()) == FR_OK;
}

static char save_path[32]; // max game title length is 24 + ".blit/" + "/"

const char *get_save_path() {
  const char *app_name = "_unknown";

  if(!directory_exists(".blit"))
    create_directory(".blit");

  app_name = "_unknown";

  // fint the program name in the binary info
  extern binary_info_t *__binary_info_start, *__binary_info_end;

  for(auto tag_ptr = &__binary_info_start; tag_ptr != &__binary_info_end ; tag_ptr++) {
    if((*tag_ptr)->type != BINARY_INFO_TYPE_ID_AND_STRING || (*tag_ptr)->tag != BINARY_INFO_TAG_RASPBERRY_PI)
      continue;

    auto id_str_tag = (binary_info_id_and_string_t *)*tag_ptr;

    if(id_str_tag->id == BINARY_INFO_ID_RP_PROGRAM_NAME) {
      app_name = id_str_tag->value;
      break;
    }
  }

  snprintf(save_path, sizeof(save_path), ".blit/%s/", app_name);

  // make sure it exists
  if(!directory_exists(save_path))
    create_directory(save_path);


  return save_path;
}
