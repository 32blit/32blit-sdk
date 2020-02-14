#pragma once
#include <cstdint>

namespace blit {
  struct Asset {
    Asset(const uint8_t *data, uint32_t length) : data(data), length(length) {}

    const uint8_t *data;
    uint32_t length;
  };
}