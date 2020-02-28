#include "file.hpp"

namespace blit {
  void *(*open_file)(std::string file)          = nullptr;
  int32_t (*read_file)(void *fh, uint32_t offset, uint32_t length, char* buffer) = nullptr;
  int32_t (*close_file)(void *fh)              = nullptr;
  uint32_t (*get_file_length)(void *fh)        = nullptr;

  std::vector<FileInfo> (*list_files) (std::string path) = nullptr;
  bool (*file_exists) (std::string path) = nullptr;
  bool (*directory_exists) (std::string path) = nullptr;

  bool (*create_directory) (std::string path);

  bool File::open(std::string file) {
    close();
    fh = open_file(file);
    return fh != nullptr;
  }

  int32_t File::read(uint32_t offset, uint32_t length, char *buffer) {
    return read_file(fh, offset, length, buffer);
  }

  void File::close() {
    if(!fh)
      return;

    close_file(fh);
    fh = nullptr;
  }

  uint32_t File::get_length() {
    return get_file_length(fh);
  }
}