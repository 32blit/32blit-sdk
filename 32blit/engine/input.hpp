#pragma once

#include "../types/vec2.hpp"
#include "../types/vec3.hpp"
#include <stdint.h>

namespace blit {

  enum button : unsigned int {
    DPAD_LEFT = 1,
    DPAD_RIGHT = 2,
    DPAD_UP = 4,
    DPAD_DOWN = 8,
    A = 16,
    B = 32,
    X = 64,
    Y = 128,
    MENU = 256,
    HOME = 512,
    JOYSTICK = 1024
  };

  extern uint32_t buttons;
  extern vec2 joystick;
  extern vec3 tilt;
  extern float hack_left;
  extern float hack_right;
  extern float battery;
  extern uint8_t battery_vbus_status;
  extern uint8_t battery_charge_status;
  extern uint8_t battery_fault;
  extern uint8_t volume;

  // The duration a note is played is determined by the amount of attack, 
  // decay, and release, combined with the length of the note as defined by
  // the user.
  //
  // - Attack:  number of quarter beats it takes for a note to hit full volume
  // - Decay:   number of quarter beats it takes for a note to settle to sustain volume
  // - Sustain: percentage of full volume that the note sustains at (duration implied by other factors)
  // - Release: number of quarter beats it takes for a note to reduce to zero volume after the end of the beat
  //
  // - Voice - type of sound produced (sine, square, saw, noise)
  //
  // Attack 1-3     Decay 4-5              Sustain              Release (12-13)
  // 
  //                +         +                                  +    +
  //                |         |                                  |    |
  //                |         |                                  |    |
  // Beat #         |         |                                  |    |
  //                v         v                                  v    v
  // 1                   2                   3                   4                   5
  //                                                                                  
  // |              XXXX |                   |                   |                   |
  // |             X    X|XX                 |                   |                   |
  // |            X      |  XXX              |                   |                   |
  // |           X       |     XXXXXXXXXXXXXX|XXXXXXXXXXXXXXXXXXX|                   |
  // |          X        |                   |                   |X                  |
  // |         X         |                   |                   |X                  |
  // |        X          |                   |                   | X                 |
  // |       X           |                   |                   | X                 |
  // |      X            |                   |                   |  X                |
  // |     X             |                   |                   |  X                |
  // |    X              |                   |                   |   X               |
  // |   X               |                   |                   |   X               |
  // |  X +    +    +    |    +    +    +    |    +    +    +    |    +    +    +    |    +
  // | X  |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |
  // |X   |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |
  // +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+--->

  extern uint8_t sine_voice[256];

  enum audio_voice    {SINE = 0, SQUARE = 1, SAWTOOTH = 2, NOISE = 3, OFF = 255};

  struct audio_channel {
    audio_voice vo = audio_voice::SINE;

    uint16_t a = 1;    // attack period (ms)
    uint16_t d = 1;    // decay period (ms)
    uint8_t  s = 200;  // sustain volume
//    uint16_t r = 1;    // release period (ms)

    uint8_t  v = 255;  // channel volume

    uint16_t f = 440;  // frequency (Hz)

    int32_t  c = 0;
  };

  struct _audio {
    audio_channel channels[4];
  };

  extern _audio audio;

  extern bool pressed(uint32_t button);
 
}
