#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "engine/file.hpp"

int32_t open_file(std::string file);
int32_t read_file(uint32_t fh, uint32_t offset, uint32_t length, char *buffer);
int32_t close_file(uint32_t fh);
uint32_t get_file_length(uint32_t fh);
std::vector<blit::FileInfo> list_files(std::string path);

namespace blit {
  extern int32_t  (*open_file)               (std::string file);
  extern int32_t  (*read_file)               (uint32_t fh, uint32_t offset, uint32_t length, char* buffer);
  extern int32_t  (*close_file)              (uint32_t fh);
  extern uint32_t (*get_file_length)         (uint32_t fh);
}