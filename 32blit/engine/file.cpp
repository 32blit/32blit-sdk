#include "file.hpp"
#include "api_private.hpp"

namespace blit {
  std::vector<FileInfo> list_files(std::string path) {
    return api.list_files(path);
  }

  bool file_exists(std::string path) {
    return api.file_exists(path);
  }
  bool directory_exists(std::string path) {
    return api.directory_exists(path);
  }

  bool create_directory(std::string path) {
    return api.create_directory(path);
  }

  bool File::open(std::string file, int mode) {
    close();
    fh = api.open_file(file, mode);
    return fh != nullptr;
  }

  int32_t File::read(uint32_t offset, uint32_t length, char *buffer) {
    return api.read_file(fh, offset, length, buffer);
  }

  int32_t File::write(uint32_t offset, uint32_t length, const char *buffer) {
    return api.write_file(fh, offset, length, buffer);
  }

  void File::close() {
    if(!fh)
      return;

    api.close_file(fh);
    fh = nullptr;
  }

  uint32_t File::get_length() {
    return api.get_file_length(fh);
  }
}