#include "file.hpp"

namespace blit {
  int32_t (*open_file)(std::string file)          = nullptr;
  int32_t (*read_file)(uint32_t fh, uint32_t offset, uint32_t length, char* buffer) = nullptr;
  int32_t (*close_file)(uint32_t fh)              = nullptr;
  uint32_t (*get_file_length)(uint32_t fh)        = nullptr;

  std::vector<FileInfo> (*list_files) (std::string path) = nullptr;


  bool File::open(std::string file) {
    close();
    fh = open_file(file);
    return fh != -1;
  }

  int32_t File::read(uint32_t offset, uint32_t length, char *buffer) {
    return read_file(fh, offset, length, buffer);
  }

  void File::close() {
    if(fh == -1)
      return;

    close_file(fh);
    fh = -1;
  }

  uint32_t File::get_length() {
    return get_file_length(fh);
  }
}