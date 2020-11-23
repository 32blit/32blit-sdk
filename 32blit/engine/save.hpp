#include <cstdint>

namespace blit {
  void read_save(char *data, uint32_t length, int slot = 0);
  void write_save(const char *data, uint32_t length, int slot = 0);

  template<class T>
  void read_save(T &data, int slot = 0) {
    read_save(reinterpret_cast<char *>(&data), sizeof(T), slot);
  }

  template<class T>
  void write_save(const T &data, int slot = 0) {
    write_save(reinterpret_cast<const char *>(&data), sizeof(T), slot);
  }
}