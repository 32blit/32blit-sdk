#include <string>
#include <string.h>
#include <memory>
#include <cstdlib>

#include "audio-test.hpp"
#include "gauntlet.h"

using namespace blit;

/* setup */
void init() {

}

void render(uint32_t time_ms) {

  fb.pen(rgba(0, 0, 0, 255));
  fb.clear();  

  fb.watermark();

}


void update(uint32_t time_ms) {
  static uint32_t last_time_ms = time_ms;
  static uint16_t i = 0;

  uint16_t scale[13] = {440, 466, 494, 523, 554, 587, 622, 659, 698, 740, 784, 831, 880};

  i++;

  uint16_t row = (i >> 1) % 900;
  uint8_t *sample = song[row * 25];

  for(auto i = 0; i < 1; i++) {
    uint16_t f = sample[0] + (sample[1] << 8);
    uint16_t c = sample[4];
    uint16_t v = ((sample[6] & 0xf0) >> 4) * 16;

    

    blit::audio.channels[i].f = (f * 985250L)/16777216L;
    blit::audio.channels[i].voices = c;
    blit::audio.channels[i].v = v;


    sample += 7;
  }

  

  /*blit::audio.channels[0].samples = noise_voice;
  blit::audio.channels[1].f = scale[((i / 100) + 3) % 13];
  //blit::audio.channels[1].samples = blit::square_voice;
  blit::audio.channels[2].f = scale[((i / 100) + 6) % 13];
  blit::audio.channels[2].samples = nullptr;
  blit::audio.channels[3].f = scale[((i / 100) + 9) % 13];
  blit::volume = (blit::joystick.y + 1.0f) * 255.0f;*/
  last_time_ms = time_ms;
}