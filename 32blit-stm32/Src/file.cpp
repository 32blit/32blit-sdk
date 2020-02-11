#include <cstdint>
#include <map>

#include "fatfs.h"

std::map<uint32_t, FIL *> open_files;
uint32_t current_file_handle = 0;

int32_t open_file(std::string file) {
  FIL *f = new FIL();

  FRESULT r = f_open(f, file.c_str(), FA_READ);

  if(r == FR_OK){      
    current_file_handle++;  
    open_files[current_file_handle] = f;
    return current_file_handle;
  }
  
  return -1;
}

int32_t read_file(uint32_t fh, uint32_t offset, uint32_t length, char *buffer) {  
  FRESULT r = FR_OK;
  FIL *f = open_files[fh];

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

int32_t close_file(uint32_t fh) {
  FRESULT r;

  r = f_close(open_files[fh]);

  return r == FR_OK ? 0 : -1;
}
