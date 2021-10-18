#include "audio.hpp"
#include "config.h"

#ifdef AUDIO_I2S
#include "pico/audio_i2s.h"
#define HAVE_AUDIO
#define AUDIO_SAMPLE_FREQ 44100
#endif

#ifdef AUDIO_PWM
#include "pico/audio_pwm.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#define HAVE_AUDIO
#define AUDIO_SAMPLE_FREQ 22050
#endif

#ifdef AUDIO_BEEP
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"

static const uint32_t PWM_WRAP = 65535;
static uint32_t slice_num = 0;
static float clock_hz = 0.0;
static uint32_t beep_time = 0;
#endif

#include "audio/audio.hpp"

#ifdef HAVE_AUDIO
static audio_buffer_pool *audio_pool = nullptr;
#endif

void init_audio() {
#ifdef AUDIO_BEEP
  clock_hz = clock_get_hz(clk_sys);

  gpio_set_function(AUDIO_BEEP_PIN, GPIO_FUNC_PWM);
  slice_num = pwm_gpio_to_slice_num(AUDIO_BEEP_PIN);

  pwm_set_wrap(slice_num, PWM_WRAP);
#endif

#ifdef HAVE_AUDIO
  static audio_format_t audio_format = {
    .sample_freq = AUDIO_SAMPLE_FREQ,
    .format = AUDIO_BUFFER_FORMAT_PCM_S16,
    .channel_count = 1
  };

  static struct audio_buffer_format producer_format = {
    .format = &audio_format,
    .sample_stride = 2
  };

  struct audio_buffer_pool *producer_pool = audio_new_producer_pool(&producer_format, 4, 441);
  const struct audio_format *output_format;

#ifdef AUDIO_I2S
  struct audio_i2s_config config = {
    .data_pin = PICO_AUDIO_I2S_DATA_PIN,
    .clock_pin_base = PICO_AUDIO_I2S_CLOCK_PIN_BASE,
    .dma_channel = 1,
    .pio_sm = 1,
  };

  output_format = audio_i2s_setup(&audio_format, &config);
  if (!output_format) {
    panic("PicoAudio: Unable to open audio device.\n");
  }

  bool ok = audio_i2s_connect(producer_pool);
  assert(ok);
  audio_i2s_set_enabled(true);
#endif

#ifdef AUDIO_PWM
  struct audio_pwm_channel_config audio_pwm_config = {
    .core = {
      .base_pin = PICO_AUDIO_PWM_MONO_PIN,
      .dma_channel = 1,
      .pio_sm = 1,
    },
    .pattern = 3,
  };
  output_format = audio_pwm_setup(&audio_format, -1, &audio_pwm_config);
  if (!output_format) {
      panic("PicoAudio: Unable to open audio device.\n");
  }

  // PWM PIO program assumes 48MHz
  pio_sm_set_clkdiv(pio1, 1, clock_get_hz(clk_sys) / 48000000.0f);

  bool ok = audio_pwm_default_connect(producer_pool, false);
  assert(ok);
  audio_pwm_set_enabled(true);
  gpio_set_drive_strength(PICO_AUDIO_PWM_MONO_PIN, GPIO_DRIVE_STRENGTH_4MA);
  gpio_set_slew_rate(PICO_AUDIO_PWM_MONO_PIN, GPIO_SLEW_RATE_FAST);
#endif

  audio_pool = producer_pool;
#endif
}

void update_audio(uint32_t time) {
#ifdef AUDIO_BEEP
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
#endif
#ifdef HAVE_AUDIO
  // audio
  struct audio_buffer *buffer = take_audio_buffer(audio_pool, false);
  if(buffer) {
    auto samples = (int16_t *) buffer->buffer->bytes;
#ifdef AUDIO_I2S
    for(uint32_t i = 0; i < buffer->max_sample_count; i += 2) {
      int val = (int)blit::get_audio_frame() - 0x8000;
      *samples++ = val;
      *samples++ = val;
    }
#endif
#ifdef AUDIO_PWM
    for(uint32_t i = 0; i < buffer->max_sample_count; i++) {
      int val = (int)blit::get_audio_frame() - 0x8000;
      *samples++ = val;
    }
#endif
    buffer->sample_count = buffer->max_sample_count;
    give_audio_buffer(audio_pool, buffer);
  }
#endif
}
