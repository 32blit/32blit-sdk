#include "audio.hpp"

#ifdef AUDIO_I2S
#include "pico/audio_i2s.h"
#define HAVE_AUDIO
#define AUDIO_SAMPLE_FREQ 44100
#endif

#ifdef AUDIO_PWM
#include "pico/audio_pwm.h"
#include "hardware/pio.h"
#define HAVE_AUDIO
#define AUDIO_SAMPLE_FREQ 22050
#endif

#include "audio/audio.hpp"

#ifdef HAVE_AUDIO
static audio_buffer_pool *audio_pool = nullptr;
#endif

void init_audio() {
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
  pio_sm_set_clkdiv(pio1, 1, 2.0f); //!
  bool ok = audio_pwm_default_connect(producer_pool, false);
  assert(ok);
  audio_pwm_set_enabled(true);
  gpio_set_drive_strength(PICO_AUDIO_PWM_MONO_PIN, GPIO_DRIVE_STRENGTH_4MA);
  gpio_set_slew_rate(PICO_AUDIO_PWM_MONO_PIN, GPIO_SLEW_RATE_FAST);
#endif

  audio_pool = producer_pool;
#endif
}

void update_audio() {
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