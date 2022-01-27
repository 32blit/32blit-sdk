#pragma once

#include <cstdint>

namespace blit {
  bool read_save(char *data, uint32_t length, int slot = 0);
  void write_save(const char *data, uint32_t length, int slot = 0);
  bool remove_save(int slot = 0);

  /**
   * \overload
   */
  template<class T>
  bool read_save(T &data, int slot = 0) {
    return read_save(reinterpret_cast<char *>(&data), sizeof(T), slot);
  }

  /**
   * \overload
   */
  template<class T>
  void write_save(const T &data, int slot = 0) {
    write_save(reinterpret_cast<const char *>(&data), sizeof(T), slot);
  }
}
