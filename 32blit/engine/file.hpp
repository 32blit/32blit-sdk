#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace blit {

  enum OpenMode {
    read  = 1 << 0,
    write = 1 << 1
  };

  enum FileFlags {
    directory = 1
  };

  struct FileInfo {
    std::string name;
    int flags;
  };

  extern std::vector<FileInfo> (*list_files) (std::string path);
  extern bool (*file_exists) (std::string path);
  extern bool (*directory_exists) (std::string path);

  extern bool (*create_directory) (std::string path);
  
  class File final {
  public:
    File() {}
    File(std::string filename, int mode = OpenMode::read) {open(filename, OpenMode::read);}
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

    bool open(std::string file, int mode = OpenMode::read);
    int32_t read(uint32_t offset, uint32_t length, char *buffer);
    int32_t write(uint32_t offset, uint32_t length, const char *buffer);
    void close();
    uint32_t get_length();

    bool is_open() const {
      return fh != nullptr;
    }

  private:
     void *fh = nullptr;
  };
}
