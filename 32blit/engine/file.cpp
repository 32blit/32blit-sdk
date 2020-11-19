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

  /**
   * Lists files on the SD card (device), the game directory (SDL) or in memory.
   *
   * \param path Path to list files at, relative to the root of the SD card or game directory (SDL).
   * 
   * \return Vector of files/directories
   */
  std::vector<FileInfo> list_files(const std::string &path) {
    auto ret = api.list_files(path);

    for(auto &buf_file : buf_files) {
      auto slash_pos = buf_file.first.find_last_of('/');
      
      bool match = false;
      if(slash_pos == std::string::npos) // file in root
        match = path.empty() || path == "/";
      else {
        if(!path.empty() && path.back() == '/') // path has trailing slash
          match = buf_file.first.substr(0, slash_pos + 1) == path;
        else
          match = buf_file.first.substr(0, slash_pos) == path;
      }

      if(match) {
        FileInfo info = {};
        info.name = buf_file.first.substr(slash_pos == std::string::npos ? 0 : slash_pos + 1);
        ret.push_back(info);
      }
    }

    return ret;
  }

  /**
   * Check if the specified path exists and is a file
   *
   * \param path Path to check existence of, relative to the root of the SD card (device) or game directory (SDL).
   * 
   * \return true if file exists
   */
  bool file_exists(const std::string &path) {
    return api.file_exists(path) || buf_files.find(path) != buf_files.end();
  }

  /**
   * Check if the specified path exists and is a directory
   *
   * \param path Path to check existence of, relative to the root of the SD card (device) or game directory (SDL).
   * 
   * \return true if directory exists
   */
  bool directory_exists(const std::string &path) {
    return api.directory_exists(path);
  }

  /**
   * Create a directory
   *
   * \param path Path to create, relative to the root of the SD card (device) or game directory (SDL).
   * 
   * \return true if directory created successfully
   */
  bool create_directory(const std::string &path) {
    return api.create_directory(path);
  }

  /**
   * Rename a file/directory
   *
   * \param old_name Path to rename, relative to the root of the SD card (device) or game directory (SDL).
   * \param new_name New name
   * 
   * \return true if file renamed successfully
   */
  bool rename_file(const std::string &old_name, const std::string &new_name) {
    return api.rename_file(old_name, new_name);
  }

  /**
   * Remove a file
   *
   * \param path Path to remove, relative to the root of the SD card (device) or game directory (SDL).
   * 
   * \return true if file removed successfully
   */
  bool remove_file(const std::string &path) {
    return api.remove_file(path);
  }

  bool File::open(const uint8_t *buf, uint32_t buf_len) {
    close();

    this->buf = buf;
    this->buf_len = buf_len;
    return true;
  }

  /**
   * Open a file. If a file is already open it will be automatically closed.
   *
   * \param file Path to open.
   * \param mode ::OpenMode to open file as. Cannot contain ::write for in-memory files.
   * 
   * \return true if file opened successfully
   */
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

  /**
   * Read a block of data from the file. Should not be called if the file was not opened for reading.
   *
   * \param offset Offset to read from
   * \param length Length to read
   * \param buffer Pointer to buffer to store data into, should be at least `length` bytes
   * 
   * \return Number of bytes read successfully or -1 if an error occurred.
   */
  int32_t File::read(uint32_t offset, uint32_t length, char *buffer) {

    if (buf) {
      auto len = std::min(length, buf_len - offset);
      memcpy(buffer, buf + offset, len);
      return len;
    }

    return api.read_file(fh, offset, length, buffer);
  }

  /**
   * Write a block of data to the file. Should not be called if the file was not opened for writing.
   *
   * \param offset Offset to write to
   * \param length Length to write
   * \param buffer Pointer to data to write, should be at least `length` bytes
   * 
   * \return Number of bytes written successfully or -1 if an error occurred.
   */
  int32_t File::write(uint32_t offset, uint32_t length, const char *buffer) {
    return api.write_file(fh, offset, length, buffer);
  }

  /**
   * Close the file. Also called automatically by the destructor.
   */
  void File::close() {
    buf = nullptr;

    if(!fh)
      return;

    api.close_file(fh);
    fh = nullptr;
  }

  /**
   * Get file length
   *
   * \return Length of the file in bytes.
   */
  uint32_t File::get_length() {
    if (buf)
      return buf_len;

    return api.get_file_length(fh);
  }

  /**
   * Creates an in-memory file, which can be used like a regular (read-only) file.
   * 
   * This is useful for porting code which assumes files, or for transparently moving data to flash
   * for extra performance.
   * 
   * Example using a packed asset:
   * ```
   * File::add_buffer_file("asset_name.bin", asset_name, asset_name_length);
   * ```
   * 
   * Notes:
   * The directory part of the path is not created if it does not exist, so ::list_files/::directory_exists
   * may not work as expected in that case: (Assuming `path/to` does not exist on the SD card)
   * ```
   * File::add_buffer_file("path/to/a.file");
   * 
   * file_exists("path/to/a.file"); // true
   * directory_exists("path/to"); // false!
   *
   * list_files("path/to"); // vector containing info for "a.file"
   * list_files("path"); // empty!
   * ```
   * 
   * \param path Path for the file
   * \param ptr Pointer to file data
   * \param len Length of file data
   */
  void File::add_buffer_file(std::string path, const uint8_t *ptr, uint32_t len) {
    buf_files.emplace(path, BufferFile{ptr, len});
  }
}
