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

  std::vector<FileInfo> list_files(std::string path);
  bool file_exists(std::string path);
  bool directory_exists(std::string path);

  bool create_directory(std::string path);

  bool rename_file(std::string old_name, std::string new_name);
  bool remove_file(std::string path);
  
  class File final {
  public:
    File() = default;
    File(std::string filename, int mode = OpenMode::read) {open(filename, mode);}
    File(const File &) = delete;
    File(File &&other) noexcept {
      *this = std::move(other);
    }

    ~File() {
      close();
    }

    File &operator=(const File &) = delete;

    File &operator=(File &&other) noexcept {
      if (this != &other) {
        close();
        std::swap(fh, other.fh);
        std::swap(buf, other.buf);
        std::swap(buf_len, other.buf_len);
      }
      return *this;
    }

    bool open(std::string file, int mode = OpenMode::read);
    int32_t read(uint32_t offset, uint32_t length, char *buffer);
    int32_t write(uint32_t offset, uint32_t length, const char *buffer);
    void close();
    uint32_t get_length();

    bool is_open() const {
      return buf != nullptr || fh != nullptr;
    }

    // for buffers pretending to be files
    const uint8_t *get_ptr() const {
      return buf;
    }

    static void add_buffer_file(std::string path, const uint8_t *ptr, uint32_t len);

  private:
      void *fh = nullptr;

      // buffer "files"
      const uint8_t *buf = nullptr;
      uint32_t buf_len;
  };
}
