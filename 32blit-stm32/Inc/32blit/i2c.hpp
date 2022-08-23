// 32blit_i2c.hpp
//
// 32Blit Firmware I2C features
//

#pragma once

namespace i2c {
  void init();
  void tick();

  bool user_send(uint8_t address, uint8_t reg, const uint8_t *data, uint16_t len);
  bool user_receive(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len);
}
