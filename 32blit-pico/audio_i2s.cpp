#include "audio.hpp"
#include "config.h"

#include "hardware/dma.h"
#include "hardware/pio.h"
#include "pico/audio_i2s.h"
#define AUDIO_SAMPLE_FREQ 44100

#include "audio/audio.hpp"

#define audio_pio __CONCAT(pio, PICO_AUDIO_I2S_PIO)

static audio_buffer_pool *audio_pool = nullptr;

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

  struct audio_i2s_config config = {
    .data_pin = PICO_AUDIO_I2S_DATA_PIN,
    .clock_pin_base = PICO_AUDIO_I2S_CLOCK_PIN_BASE,
    .dma_channel = dma_channel,
    .pio_sm = pio_sm,
  };

  output_format = audio_i2s_setup(&audio_format, &config);
  if (!output_format) {
    panic("PicoAudio: Unable to open audio device.\n");
  }

  [[maybe_unused]] bool ok = audio_i2s_connect(producer_pool);
  assert(ok);
  audio_i2s_set_enabled(true);

  audio_pool = producer_pool;
}

void update_audio(uint32_t time) {
  // audio
  struct audio_buffer *buffer = take_audio_buffer(audio_pool, false);
  if(buffer) {
    auto samples = (int16_t *) buffer->buffer->bytes;

    for(uint32_t i = 0; i < buffer->max_sample_count; i += 2) {
      int val = (int)blit::get_audio_frame() - 0x8000;
      *samples++ = val;
      *samples++ = val;
    }

    buffer->sample_count = buffer->max_sample_count;
    give_audio_buffer(audio_pool, buffer);
  }
}
