#include "useri2c.hpp"

namespace blit {
  /**
   * Request data from an i2c slave.
   *
   * @param address Address of slave.
   * @param reg Register value.
   * @param result Reference to array to store result.
   * @param result_size Expected size of result.
   */
  void (*i2c_receive) (uint8_t address, uint8_t reg, uint8_t &result, uint8_t result_size)  = nullptr;

  /**
   * Send data to an i2c slave.
   *
   * @param address Address of slave.
   * @param reg Register value.
   * @param data Reference to array of data to send.
   * @param data_size Size of data to send.
   */
  void (*i2c_send)    (uint8_t address, uint8_t reg, uint8_t &data, uint8_t data_size)     = nullptr;
}