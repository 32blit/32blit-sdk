#include "audio.hpp"
#include "config.h"

#include "pico/audio_pwm.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/pio.h"

#define AUDIO_SAMPLE_FREQ 22050

#include "audio/audio.hpp"

#define audio_pio __CONCAT(pio, PICO_AUDIO_I2S_PIO)

static audio_buffer_pool *audio_pool = nullptr;

static struct audio_buffer *cur_buffer = nullptr;

void init_audio() {
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

  uint8_t dma_channel = dma_claim_unused_channel(true);
  uint8_t pio_sm = pio_claim_unused_sm(audio_pio, true);

  // audio_i2s_setup claims
  dma_channel_unclaim(dma_channel);
  pio_sm_unclaim(audio_pio, pio_sm);

  struct audio_pwm_channel_config audio_pwm_config = {
    .core = {
      .base_pin = PICO_AUDIO_PWM_MONO_PIN,
      .dma_channel = dma_channel,
      .pio_sm = pio_sm,
    },
    .pattern = 3,
  };
  output_format = audio_pwm_setup(&audio_format, -1, &audio_pwm_config);
  if (!output_format) {
      panic("PicoAudio: Unable to open audio device.\n");
  }

  // PWM PIO program assumes 48MHz
  pio_sm_set_clkdiv(pio1, 1, clock_get_hz(clk_sys) / 48000000.0f);

  [[maybe_unused]] bool ok = audio_pwm_default_connect(producer_pool, false);
  assert(ok);
  audio_pwm_set_enabled(true);
  gpio_set_drive_strength(PICO_AUDIO_PWM_MONO_PIN, GPIO_DRIVE_STRENGTH_4MA);
  gpio_set_slew_rate(PICO_AUDIO_PWM_MONO_PIN, GPIO_SLEW_RATE_FAST);

  audio_pool = producer_pool;
}

void update_audio(uint32_t time) {
  // attempt to get new buffer
  if(!cur_buffer) {
    cur_buffer = take_audio_buffer(audio_pool, false);
    if(cur_buffer)
      cur_buffer->sample_count = 0;
  }

  if(cur_buffer) {
    auto samples = ((int16_t *)cur_buffer->buffer->bytes) + cur_buffer->sample_count;

    auto max_samples = cur_buffer->max_sample_count - cur_buffer->sample_count;

    for(uint32_t i = 0; i < max_samples; i++) {
      int val = (int)blit::get_audio_frame() - 0x8000;
      *samples++ = val;
    }

    cur_buffer->sample_count += max_samples;

    if(cur_buffer->sample_count == cur_buffer->max_sample_count) {
      give_audio_buffer(audio_pool, cur_buffer);
      cur_buffer = nullptr;
    }
  }
}
