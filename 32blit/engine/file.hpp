#pragma once

#include <stdint.h>
#include <string>

namespace blit {
  extern int32_t  (*open_file)        (std::string file);
  extern int32_t  (*read_file)        (uint32_t fh, uint32_t offset, uint32_t length, char* buffer);
  extern int32_t  (*close_file)       (uint32_t fh);
}