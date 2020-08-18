#include <algorithm>
#include <cstring>
#include <map>

#include "file.hpp"
#include "api_private.hpp"

namespace blit {
  struct BufferFile {
    const uint8_t *ptr;
    uint32_t length;
  };

  static std::map<std::string, BufferFile> buf_files;

  std::vector<FileInfo> list_files(const std::string &path) {
    auto ret = api.list_files(path);

    for(auto &buf_file : buf_files) {
      auto slash_pos = buf_file.first.find_last_of('/');
      if(slash_pos == std::string::npos)
        slash_pos = 0;
      
      if(buf_file.first.substr(0, slash_pos) == path) {
        FileInfo info = {};
        info.name = buf_file.first.substr(slash_pos == 0 ? 0 : slash_pos + 1);
        ret.push_back(info);
      }
    }

    return ret;
  }

  bool file_exists(const std::string &path) {
    return api.file_exists(path) || buf_files.find(path) != buf_files.end();
  }
  bool directory_exists(const std::string &path) {
    return api.directory_exists(path);
  }

  bool create_directory(const std::string &path) {
    return api.create_directory(path);
  }

  bool rename_file(const std::string &old_name, const std::string &new_name) {
    return api.rename_file(old_name, new_name);
  }

  bool remove_file(const std::string &path) {
    return api.remove_file(path);
  }

  bool File::open(const std::string &file, int mode) {
    close();

    // check for buffer
    auto it = buf_files.find(file);

    if (mode == OpenMode::read && it != buf_files.end()) {
      buf = it->second.ptr;
      buf_len = it->second.length;
      return true;
    }

    fh = api.open_file(file, mode);
    return fh != nullptr;
  }

  int32_t File::read(uint32_t offset, uint32_t length, char *buffer) {

    if (buf) {
      auto len = std::min(length, buf_len - offset);
      memcpy(buffer, buf + offset, len);
      return len;
    }

    return api.read_file(fh, offset, length, buffer);
  }

  int32_t File::write(uint32_t offset, uint32_t length, const char *buffer) {
    return api.write_file(fh, offset, length, buffer);
  }

  void File::close() {
    buf = nullptr;

    if(!fh)
      return;

    api.close_file(fh);
    fh = nullptr;
  }

  uint32_t File::get_length() {
    if (buf)
      return buf_len;

    return api.get_file_length(fh);
  }

  void File::add_buffer_file(std::string path, const uint8_t *ptr, uint32_t len) {
    buf_files.emplace(path, BufferFile{ptr, len});
  }
}