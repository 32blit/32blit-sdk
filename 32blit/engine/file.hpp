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

    bool open(std::string file);
    int32_t read(uint32_t offset, uint32_t length, char *buffer);
    void close();
    uint32_t get_length();

    bool is_open() const {
      return fh != -1;
    }

  private:
    int32_t fh = -1;
  };
}
