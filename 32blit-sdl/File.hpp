#pragma once

#include <cstdint>
#include <string>

void setup_base_path();
int32_t open_file(std::string file);
int32_t read_file(uint32_t fh, uint32_t offset, uint32_t length, char *buffer);
int32_t close_file(uint32_t fh);