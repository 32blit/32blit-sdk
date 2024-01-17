// simple audio for the PicoSystem's piezo buzzer
#include "audio.hpp"
#include "config.h"

#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"

static const uint32_t PWM_WRAP = 65535;
static uint32_t slice_num = 0;
static float clock_hz = 0.0;
static uint32_t beep_time = 0;

#include "audio/audio.hpp"

void init_audio() {
  clock_hz = clock_get_hz(clk_sys);

  gpio_set_function(AUDIO_BEEP_PIN, GPIO_FUNC_PWM);
  slice_num = pwm_gpio_to_slice_num(AUDIO_BEEP_PIN);

  pwm_set_wrap(slice_num, PWM_WRAP);
}

void update_audio(uint32_t time) {
  bool on = false;
  uint32_t elapsed = time - beep_time;
  beep_time = time;

  for(auto f = 0u; f < elapsed * blit::sample_rate / 1000; f++) {
    blit::get_audio_frame();
  }

  // Find the first square wave enabled channel and use freq/pulse width to drive the beeper
  for(int c = 0; c < CHANNEL_COUNT; c++) {
    auto &channel = blit::channels[c];

    if(channel.waveforms & blit::Waveform::SQUARE) {
      on = channel.volume
        && channel.adsr_phase != blit::ADSRPhase::RELEASE
        && channel.adsr_phase != blit::ADSRPhase::OFF;

      if(on) {
        pwm_set_clkdiv(slice_num, (clock_hz / PWM_WRAP) / channel.frequency);
        pwm_set_gpio_level(AUDIO_BEEP_PIN, channel.pulse_width);
        break;
      }
    }
  }

  pwm_set_enabled(slice_num, on);
}
