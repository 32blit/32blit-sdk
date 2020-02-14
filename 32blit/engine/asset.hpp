#pragma once
#include <cstdint>

namespace blit {
  struct Asset {
    Asset(uint8_t *data, uint32_t length) : data(data), length(length) {}

    uint8_t *data;
    uint32_t length;
  };
}