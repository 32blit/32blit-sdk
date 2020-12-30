#include "save.hpp"
#include "api_private.hpp"
#include "file.hpp"

namespace blit {
  /**
   * Read a block of save data from a save slot.
   *
   * \param data Pointer to store data into, should be at least `length` bytes
   * \param length Expected length of save data
   * \param slot Save slot to load, can be any number
   * 
   * \return `true` if a save exists and contains enough data
   */
  bool read_save(char *data, uint32_t length, int slot) {
    File file(std::string(api.get_save_path()) + "save" + std::to_string(slot));

    return file.is_open() && uint32_t(file.read(0, length, data)) == length;
  }

  /**
   * Write a block of save data to a save slot.
   *
   * \param data Pointer to data to write, should be `length` bytes
   * \param length Length of data to write
   * \param slot Save slot to write to, can be any number
   */
  void write_save(const char *data, uint32_t length, int slot) {
    File(std::string(api.get_save_path()) + "save" + std::to_string(slot), OpenMode::write).write(0, length, data);
  }
}