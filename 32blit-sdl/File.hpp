#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "engine/file.hpp"

void setup_base_path();
void *open_file(const std::string &file, int mode);
int32_t read_file(void *fh, uint32_t offset, uint32_t length, char *buffer);
int32_t write_file(void *fh, uint32_t offset, uint32_t length, const char *buffer);
int32_t close_file(void *fh);
uint32_t get_file_length(void *fh);
std::vector<blit::FileInfo> list_files(const std::string &path);
bool file_exists(const std::string &path);
bool directory_exists(const std::string &path);
bool create_directory(const std::string &path);
bool rename_file(const std::string &old_name, const std::string &new_name);
bool remove_file(const std::string &path);
std::string get_save_path();