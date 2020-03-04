#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "engine/file.hpp"

void *open_file(std::string file, int mode);
int32_t read_file(void *fh, uint32_t offset, uint32_t length, char *buffer);
int32_t write_file(void *fh, uint32_t offset, uint32_t length, const char *buffer);
int32_t close_file(void *fh);
uint32_t get_file_length(void *fh);
std::vector<blit::FileInfo> list_files(std::string path);
bool file_exists(std::string path);
bool directory_exists(std::string path);
bool create_directory(std::string path);
