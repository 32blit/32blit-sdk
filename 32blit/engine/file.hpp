#pragma once

#include <cstdint>
#include <string>
#include <vector>

/** 32blit namespace */
namespace blit {

  enum OpenMode {
    /// Open file for reading
    read  = 1 << 0,
    /// Open file for writing
    write = 1 << 1
  };

  enum FileFlags {
    /// Is a directory
    directory = 1
  };

  struct FileInfo {
    /// Name of the file
    std::string name;
    /// Flags (see ::FileFlags)
    int flags;
    /// Size of the file
    uint32_t size;
  };

  std::vector<FileInfo> list_files(const std::string &path);
  bool file_exists(const std::string &path);
  bool directory_exists(const std::string &path);

  bool create_directory(const std::string &path);

  bool rename_file(const std::string &old_name, const std::string &new_name);
  bool remove_file(const std::string &path);
  
  /**
   * Class for accessing files on the SD card (device), the game directory (SDL) or in memory. 
   */
  class File final {
  public:
    File() = default;
    File(const std::string &filename, int mode = OpenMode::read) {open(filename, mode);}
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

    bool open(const std::string &file, int mode = OpenMode::read);
    bool open(const uint8_t *buf, uint32_t buf_len);
    int32_t read(uint32_t offset, uint32_t length, char *buffer);
    int32_t write(uint32_t offset, uint32_t length, const char *buffer);
    void close();
    uint32_t get_length();

    /** \returns `true` if file is open */
    bool is_open() const {
      return buf != nullptr || fh != nullptr;
    }

    /** \returns pointer to data for in-memory files */
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
