#include "save.hpp"
#include "api_private.hpp"
#include "file.hpp"

namespace blit {
  bool read_save(char *data, uint32_t length, int slot) {
    File file(api.get_save_path() + "save" + std::to_string(slot));

    return file.is_open() && uint32_t(file.read(0, length, data)) == length;
  }

  void write_save(const char *data, uint32_t length, int slot) {
    File(api.get_save_path() + "save" + std::to_string(slot), OpenMode::write).write(0, length, data);
  }
}