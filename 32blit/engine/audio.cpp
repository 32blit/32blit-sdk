/*! \file audio.cpp
    \brief Audio engine
*/
#include "engine.hpp"
#include "input.hpp"

#include "audio.hpp"

namespace blit {

  namespace audio {

    uint32_t sample_rate = 22050;
    uint32_t frame_ms = (1000 << 16) / 22050;

    uint16_t volume = 0xffff;
    int16_t sine_voice[256] = {0,804,1608,2411,3212,4011,4808,5602,6393,7180,7962,8740,9512,10279,11039,11793,12540,13279,14010,14733,15447,16151,16846,17531,18205,18868,19520,20160,20788,21403,22006,22595,23170,23732,24279,24812,25330,25833,26320,26791,27246,27684,28106,28511,28899,29269,29622,29957,30274,30572,30853,31114,31357,31581,31786,31972,32138,32286,32413,32522,32610,32679,32729,32758,32767,32758,32729,32679,32610,32522,32413,32286,32138,31972,31786,31581,31357,31114,30853,30572,30274,29957,29622,29269,28899,28511,28106,27684,27246,26791,26320,25833,25330,24812,24279,23732,23170,22595,22006,21403,20788,20160,19520,18868,18205,17531,16846,16151,15447,14733,14010,13279,12540,11793,11039,10279,9512,8740,7962,7180,6393,5602,4808,4011,3212,2411,1608,804,0,-804,-1608,-2411,-3212,-4011,-4808,-5602,-6393,-7180,-7962,-8740,-9512,-10279,-11039,-11793,-12540,-13279,-14010,-14733,-15447,-16151,-16846,-17531,-18205,-18868,-19520,-20160,-20788,-21403,-22006,-22595,-23170,-23732,-24279,-24812,-25330,-25833,-26320,-26791,-27246,-27684,-28106,-28511,-28899,-29269,-29622,-29957,-30274,-30572,-30853,-31114,-31357,-31581,-31786,-31972,-32138,-32286,-32413,-32522,-32610,-32679,-32729,-32758,-32768,-32758,-32729,-32679,-32610,-32522,-32413,-32286,-32138,-31972,-31786,-31581,-31357,-31114,-30853,-30572,-30274,-29957,-29622,-29269,-28899,-28511,-28106,-27684,-27246,-26791,-26320,-25833,-25330,-24812,-24279,-23732,-23170,-22595,-22006,-21403,-20788,-20160,-19520,-18868,-18205,-17531,-16846,-16151,-15447,-14733,-14010,-13279,-12540,-11793,-11039,-10279,-9512,-8740,-7962,-7180,-6393,-5602,-4808,-4011,-3212,-2411,-1608,-804};

    audio_channel channels[CHANNEL_COUNT];

    uint16_t get_audio_frame() {
      int64_t sample = 0;  // used to combine channel output

      for(auto &channel : channels) {

        // increment the voice position counter. this provides an 
        // Q16 fixed point value representing how far through 
        // the current voice pattern we are
        channel.voice_offset += ((channel.frequency * 256) << 8) / sample_rate;

        if(channel.voice_offset & 0xfffff000) {
          // if the voice offset overflows then generate a new random
          // noise sample
          channel.noise = (rand() & 0xffff) - (0xffff >> 1); 
        }

        channel.voice_offset &= 0xffff;

        // gate bit has changed, reset note timer
        if((channel.flags & 0b1) != (channel.gate & 0b1)) {
          channel.time_ms = 0;

          channel.attack_start_adsr = channel.adsr;
        }        

        // check if any voices are active for this channel
        if(channel.voices) {
          int64_t channel_sample = 0;
                  
          
          if(channel.voices & audio_voice::NOISE) {
            //channel_sample += (channel.noise - 0x7fff) >> 2;
            channel_sample += channel.noise;
          }

          if(channel.voices & audio_voice::SAW) {
            channel_sample += (int32_t)channel.voice_offset - 0x7fff;
          }

          if(channel.voices & audio_voice::TRIANGLE) {     
            if(channel.voice_offset < 0x3fff) {
              channel_sample += channel.voice_offset * 2;
            } else if(channel.voice_offset < 0xbfff) {
              channel_sample += (int32_t)0x7fff - (((int32_t)channel.voice_offset - 0x3fff) * 2);              
            } else {
              channel_sample += -(int32_t)0x7fff + (((int32_t)channel.voice_offset - 0xbfff) * 2);              
            }            
          }

          if(channel.voices & audio_voice::SQUARE) {
            channel_sample += ((channel.voice_offset >> 8) < channel.pulse_width) ? 0x7fff : -0x7fff;
          }

          if(channel.voices & audio_voice::SINE) {
            // the sine_voice sample contains 256 samples in total
            // so we'll just use the most significant bits of the
            // current voice position to index into it
            channel_sample += sine_voice[channel.voice_offset >> 8];
          }

          if(channel.gate) {
            if((channel.time_ms >> 16) < channel.attack_ms) {
              // attack phase
              //channel.adsr = (((channel.time_ms / channel.attack_ms) * (0xffff - channel.attack_start_adsr)) >> 16) + channel.attack_start_adsr; // (Q16)
              channel.adsr = channel.time_ms / channel.attack_ms;
            } else if((channel.time_ms >> 16) < (channel.attack_ms + channel.decay_ms)) {
              // decay phase
              uint32_t decay = (channel.time_ms - (channel.attack_ms << 16)) / channel.decay_ms;
              channel.adsr = 0xffff - (((0xffff - channel.sustain) * decay) >> 16);
            } else {
              // sustain phase
              channel.adsr = channel.sustain;
            }  
          }else{
            if((channel.time_ms >> 16) < channel.release_ms) {
              // release phase
              uint32_t release = channel.time_ms / channel.release_ms;
              channel.adsr = channel.sustain - ((channel.sustain * release) >> 16);
            }
          }      

          channel_sample = (channel_sample * channel.adsr) >> 16;

          // apply channel volume
          channel_sample = (channel_sample * channel.volume) >> 16;

          // combine channel sample into the final sample
          sample += channel_sample;     

          channel.time_ms += frame_ms;          
        }

        // copy the current gate value into the status flags
        channel.flags &= ~0b0000001;
        channel.flags |= channel.gate & 0b0000001;
      }

      sample = (sample * volume) >> 16;


      static int32_t last_sample = 0;

      // apply global volume
      //float delay = blit::joystick.y + 1.0f;
      //float freq = blit::joystick.x + 1.0f;

      float ePow = (1-exp(- 0.02 * 2 * M_PI * 5.0));
      
      last_sample += (sample - last_sample) * ePow;
      sample = last_sample;
      // apply low pass filter

      // clip result to 16-bit and convert to unsigned
      sample = sample <= -0x7fff ? -0x7fff : (sample > 0x7fff ? 0x7fff : sample);      
      sample += 0x7fff;

      return sample;
    }
  }
}