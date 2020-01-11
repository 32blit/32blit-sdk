#pragma once

#include <stdint.h>

namespace blit {  

  // The duration a note is played is determined by the amount of attack, 
  // decay, and release, combined with the length of the note as defined by
  // the user.
  //
  // - Attack:  number of milliseconds it takes for a note to hit full volume
  // - Decay:   number of milliseconds it takes for a note to settle to sustain volume
  // - Sustain: percentage of full volume that the note sustains at (duration implied by other factors)
  // - Release: number of milliseconds it takes for a note to reduce to zero volume after it has ended
  //
  // Attack (750ms) - Decay (500ms) -------- Sustain ----- Release (250ms)
  // 
  //                +         +                                  +    +
  //                |         |                                  |    |
  //                |         |                                  |    |
  //                |         |                                  |    |
  //                v         v                                  v    v
  // 0ms               1000ms              2000ms              3000ms              4000ms
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

  namespace audio {
    #define CHANNEL_COUNT 4

    extern uint32_t sample_rate;
    extern uint32_t frame_ms;       // number of milliseconds per audio frame (Q16)
    extern uint16_t volume;
    extern uint16_t  sine_voice[256];

    enum audio_voice {
      NOISE     = 128, 
      SQUARE    = 64, 
      SAW       = 32, 
      TRIANGLE  = 16, 
      SINE      = 8,
      WAVE      = 4    // to be implemented...
    };

    struct audio_channel {
        uint8_t   voices        = 0;      // bitmask for enabled voices (see audio_voice enum for values)
        uint16_t  frequency     = 660;    // frequency of the voice (Hz)
        uint32_t  time_ms       = 0;      // play time of current note in milliseconds used for ADSR calculations (Q16)
        uint16_t  volume        = 0xffff; // channel volume (default 50%)

        uint16_t  attack_ms     = 2;      // attack period
        uint16_t  decay_ms      = 6;      // decay period
        uint16_t  sustain       = 0xffff; // sustain volume
        uint16_t  release_ms    = 1;      // release period
        uint16_t  pulse_width   = 0x7f;   // duty cycle of square voice (default 50%)
        uint16_t  noise         = 0;      // current noise value
    
        uint32_t  voice_offset  = 0;      // voice offset (Q8)

        uint8_t   gate          = 0;      // gate triggers the start of a sound when set to 1, triggers the release phase when set to 0
        uint8_t   flags         = 0;      // bit 7: last gate value
    };

    extern audio_channel channels[CHANNEL_COUNT];

    uint16_t get_audio_frame();
  };

}