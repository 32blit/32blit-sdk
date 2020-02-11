#include "file.hpp"

namespace blit {
  int32_t (*open_file)(std::string file)          = nullptr;
  int32_t (*read_file)(uint32_t fh, uint32_t offset, uint32_t length, char* buffer) = nullptr;
  int32_t (*close_file)(uint32_t fh)              = nullptr;
}