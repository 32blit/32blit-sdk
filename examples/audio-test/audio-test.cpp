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
  
  fb.pen(rgba(255, 255, 255));

  static uint16_t i = 0;
  i++;
  fb.pixel(point(i % 160, 0));


fb.text(std::to_string(blit::audio.channels[0].pw), &minimal_font[0][0], point(0, 0));
/*  for(auto i = 0; i < 20; i++) {
    uint8_t *sample = &song[i * 25];
    uint32_t f = (sample[1] << 8) | sample[0];
    uint32_t fhz = (f * 98525L) / 1677721L;
    
  }*/
  
}


void update(uint32_t time_ms) {
  static uint32_t last_time_ms = time_ms;
  static uint16_t i = 0;

  i++;
  blit::volume = 255;
  uint16_t row = (i >> 1) % 900;
  uint8_t *sample = song + (row * 25);

  for(auto i = 0; i < 3; i++) {
    uint32_t f = (sample[1] << 8) | sample[0];
    uint16_t voices = sample[4];

    uint32_t fhz = (f * 98525L) / 1677721L;

    blit::audio.channels[i].pw = ((sample[3] & 0xf) << 8) | sample[2];
    blit::audio.channels[i].f = fhz;
    blit::audio.channels[i].voices = voices;
    blit::audio.channels[i].s = sample[6] & 0xf0;
    blit::audio.channels[i].a = a_to_frames[(sample[5] & 0xf0) >> 4];
    blit::audio.channels[i].d = dr_to_frames[sample[5] & 0xf];

    sample += 7;
  }
/*
  blit::audio.channels[0].f = 440;
  blit::audio.channels[0].voices = 0b11110000;
  blit::audio.channels[0].v = 128;
*/
/*
  blit::audio.channels[1].f = 220;
  blit::audio.channels[1].voices = 0b11110000;
  blit::audio.channels[1].v = 128;
*/
  /*blit::audio.channels[0].samples = noise_voice;
  blit::audio.channels[1].f = scale[((i / 100) + 3) % 13];
  //blit::audio.channels[1].samples = blit::square_voice;
  blit::audio.channels[2].f = scale[((i / 100) + 6) % 13];
  blit::audio.channels[2].samples = nullptr;
  blit::audio.channels[3].f = scale[((i / 100) + 9) % 13];
  blit::volume = (blit::joystick.y + 1.0f) * 255.0f;*/
  last_time_ms = time_ms;
}