#pragma once

#include <stdint.h>
#include <string>
#include <vector>

namespace blit {

  enum FileFlags {
    directory = 1
  };

  struct FileInfo {
    std::string name;
    int flags;
  };

  extern int32_t  (*open_file)               (std::string file);
  extern int32_t  (*read_file)               (uint32_t fh, uint32_t offset, uint32_t length, char* buffer);
  extern int32_t  (*close_file)              (uint32_t fh);
  extern uint32_t (*get_file_length)         (uint32_t fh);

  extern std::vector<FileInfo> (*list_files) (std::string path);
}