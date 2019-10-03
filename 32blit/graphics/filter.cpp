
namespace blit {
/*
  void blur(uint8_t passes) {
    // horizontal
    uint8_t last;

    for (uint8_t pass = 0; pass < passes; pass++) {
      uint8_t *p = (uint8_t *)target;
      for (uint8_t y = 0; y < 120; y++) {
        last = *p;
        p++;

        for (uint8_t x = 1; x < 159; x++) {
          *p = (*(p + 1) + last + *p + *p) >> 2;
          last = *p;
          p++;
        }
      }
    }

    // vertical      
    for (uint8_t pass = 0; pass < passes; pass++) {
      for (uint8_t x = 0; x < 160; x++) {
        uint8_t *p = (uint8_t *)target + x;

        last = *p;
        p += 160;
         
        for (uint8_t y = 1; y < 119; y++) {
          *p = (*(p + 160) + last + *p + *p) >> 2;
          last = *p;
          p += 160;
        }
      }
    }
  }

  uint32_t g_seed = 0;
  inline uint8_t rand_uint8() {
    g_seed = (214013 * g_seed + 2531011);
    return (g_seed >> 16) & 0xFF;
  }

  void noise(uint8_t alpha) {
    uint8_t *p = (uint8_t *)target;
    for (uint8_t y = 0; y < 120; y++) {
      for (uint8_t x = 0; x < 160; x++) {
        *p++ = ((*p * (255 - alpha)) + (rand_uint8() * alpha)) >> 8;
      }
    }
  }*/

}