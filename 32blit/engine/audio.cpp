/*! \file audio.cpp
    \brief Audio engine
*/
#include "engine.hpp"
#include "input.hpp"

#include "audio.hpp"

namespace blit {

  uint32_t sample_rate = 22050;

  uint16_t volume = 0xffff;
  const int16_t sine_waveform[256] = {0,804,1608,2411,3212,4011,4808,5602,6393,7180,7962,8740,9512,10279,11039,11793,12540,13279,14010,14733,15447,16151,16846,17531,18205,18868,19520,20160,20788,21403,22006,22595,23170,23732,24279,24812,25330,25833,26320,26791,27246,27684,28106,28511,28899,29269,29622,29957,30274,30572,30853,31114,31357,31581,31786,31972,32138,32286,32413,32522,32610,32679,32729,32758,32767,32758,32729,32679,32610,32522,32413,32286,32138,31972,31786,31581,31357,31114,30853,30572,30274,29957,29622,29269,28899,28511,28106,27684,27246,26791,26320,25833,25330,24812,24279,23732,23170,22595,22006,21403,20788,20160,19520,18868,18205,17531,16846,16151,15447,14733,14010,13279,12540,11793,11039,10279,9512,8740,7962,7180,6393,5602,4808,4011,3212,2411,1608,804,0,-804,-1608,-2411,-3212,-4011,-4808,-5602,-6393,-7180,-7962,-8740,-9512,-10279,-11039,-11793,-12540,-13279,-14010,-14733,-15447,-16151,-16846,-17531,-18205,-18868,-19520,-20160,-20788,-21403,-22006,-22595,-23170,-23732,-24279,-24812,-25330,-25833,-26320,-26791,-27246,-27684,-28106,-28511,-28899,-29269,-29622,-29957,-30274,-30572,-30853,-31114,-31357,-31581,-31786,-31972,-32138,-32286,-32413,-32522,-32610,-32679,-32729,-32758,-32768,-32758,-32729,-32679,-32610,-32522,-32413,-32286,-32138,-31972,-31786,-31581,-31357,-31114,-30853,-30572,-30274,-29957,-29622,-29269,-28899,-28511,-28106,-27684,-27246,-26791,-26320,-25833,-25330,-24812,-24279,-23732,-23170,-22595,-22006,-21403,-20788,-20160,-19520,-18868,-18205,-17531,-16846,-16151,-15447,-14733,-14010,-13279,-12540,-11793,-11039,-10279,-9512,-8740,-7962,-7180,-6393,-5602,-4808,-4011,-3212,-2411,-1608,-804};

  AudioChannel channels[CHANNEL_COUNT];

  uint16_t get_audio_frame() {
    int64_t sample = 0;  // used to combine channel output

    for(auto &channel : channels) {

      // increment the waveform position counter. this provides an 
      // Q16 fixed point value representing how far through 
      // the current waveform we are
      channel.waveform_offset += ((channel.frequency * 256) << 8) / sample_rate;

      if(channel.adsr_phase == ADSRPhase::OFF) {
        continue;
      }

      if ((channel.adsr_frame >= channel.adsr_end_frame) && (channel.adsr_phase != ADSRPhase::SUSTAIN)) {
        switch (channel.adsr_phase) {
          case ADSRPhase::ATTACK:
            channel.trigger_decay();
            break;
          case ADSRPhase::DECAY:
            channel.trigger_sustain();
            break;
          case ADSRPhase::RELEASE:
            channel.off();
            break;
        }
      }
 
      channel.adsr += channel.adsr_step;
      channel.adsr_frame++;

      if(channel.waveform_offset & 0xfffff000) {
        // if the waveform offset overflows then generate a new
        // random noise sample
        channel.noise = (rand() & 0xffff) - (0xffff >> 1); 
      }

      channel.waveform_offset &= 0xffff;

      // check if any waveforms are active for this channel
      if(channel.waveforms) {
        int64_t channel_sample = 0;

        if(channel.waveforms & Waveform::NOISE) {
          //channel_sample += (channel.noise - 0x7fff) >> 2;
          channel_sample += channel.noise;
        }

        if(channel.waveforms & Waveform::SAW) {
          channel_sample += (int32_t)channel.waveform_offset - 0x7fff;
        }

        if(channel.waveforms & Waveform::TRIANGLE) {
          if(channel.waveform_offset < 0x3fff) {
            channel_sample += channel.waveform_offset * 2;
          } else if(channel.waveform_offset < 0xbfff) {
            channel_sample += (int32_t)0x7fff - (((int32_t)channel.waveform_offset - 0x3fff) * 2);              
          } else {
            channel_sample += -(int32_t)0x7fff + (((int32_t)channel.waveform_offset - 0xbfff) * 2);              
          }            
        }

        if(channel.waveforms & Waveform::SQUARE) {
          channel_sample += ((channel.waveform_offset >> 8) < channel.pulse_width) ? 0x7fff : -0x7fff;
        }

        if(channel.waveforms & Waveform::SINE) {
          // the sine_waveform sample contains 256 samples in
          // total so we'll just use the most significant bits
          // of the current waveform position to index into it
          channel_sample += sine_waveform[channel.waveform_offset >> 8];
        }

        channel_sample = (channel_sample * int32_t(channel.adsr >> 8)) >> 16;

        // apply channel volume
        channel_sample = (channel_sample * int32_t(channel.volume)) >> 16;

        // combine channel sample into the final sample
        sample += channel_sample;
      }
    }

    sample = (sample * int32_t(volume)) >> 16;

    // clip result to 16-bit and convert to unsigned
    sample = sample <= -0x7fff ? -0x7fff : (sample > 0x7fff ? 0x7fff : sample);      
    sample += 0x7fff;

    return sample;
  }
}