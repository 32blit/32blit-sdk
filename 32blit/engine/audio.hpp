#pragma once

#include <stdint.h>

namespace blit {

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

  extern int8_t sine_voice[256];
  extern int8_t square_voice[256];
  extern int8_t saw_voice[256];  
  extern uint32_t noise_voice;

  enum audio_voice    {NOISE = 0b10000000, SQUARE = 0b01000000, SAW = 0b00100000, SINE = 0b00010000};

  struct audio_channel {
      uint8_t voices = 0;

      uint16_t a = 1;    // attack period (ms)
      uint16_t d = 1;    // decay period (ms)
      uint8_t  s = 200;  // sustain volume
  //    uint16_t r = 1;    // release period (ms)

      uint8_t  v = 255;  // channel volume
      uint16_t vp = 0;  // voice position 8:8 fixed point

      uint16_t f = 660;  // frequency (Hz)

      int32_t  c = 0;
  };

  struct _audio {
      audio_channel channels[4];
  };

  extern _audio audio;

}