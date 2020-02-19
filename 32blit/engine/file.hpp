#pragma once

#include <stdint.h>
#include <string>
#include <vector>

namespace blit {

  enum FileFlags {
    directory = 1
  };

  struct FileInfo {
    std::string name;
    int flags;
  };

  extern int32_t  (*open_file)               (std::string file);
  extern int32_t  (*read_file)               (uint32_t fh, uint32_t offset, uint32_t length, char* buffer);
  extern int32_t  (*close_file)              (uint32_t fh);
  extern uint32_t (*get_file_length)         (uint32_t fh);

  extern std::vector<FileInfo> (*list_files) (std::string path);
  
  class File final {
  public:
    File() {}
    File(std::string filename) {open(filename);}
    File(const File &) = delete;
    File(File &&other) {
      *this = std::move(other);
    }

    ~File() {
      close();
    }

    File &operator=(const File &) = delete;

    File &operator=(File &&other) {
      if (this != &other) {
        close();
        std::swap(fh, other.fh);
      }
      return *this;
    }

    bool open(std::string file) {
      close();
      fh = open_file(file);
      return fh != -1;
    }

    int32_t read(uint32_t offset, uint32_t length, char *buffer) {
      return read_file(fh, offset, length, buffer);
    }

    void close() {
      if(fh == -1)
        return;

      close_file(fh);
      fh = -1;
    }

    uint32_t get_length() {
      return get_file_length(fh);
    }

    bool is_open() const {
      return fh != -1;
    }

  private:
    int32_t fh = -1;
  };
}
