#include "audio.hpp"

#include "pico/audio_i2s.h"

#include "audio/audio.hpp"

static audio_buffer_pool *audio_pool = nullptr;

void init_audio() {
#ifdef AUDIO_I2S
  static audio_format_t audio_format = {
    .sample_freq = 44100,
    .format = AUDIO_BUFFER_FORMAT_PCM_S16,
    .channel_count = 1
  };

  static struct audio_buffer_format producer_format = {
    .format = &audio_format,
    .sample_stride = 2
  };

  struct audio_buffer_pool *producer_pool = audio_new_producer_pool(&producer_format, 4, 441);
  const struct audio_format *output_format;

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
  audio_pool = producer_pool;
#else
  audio_pool = nullptr;
#endif
}

void update_audio() {
  // audio
  if(!audio_pool)
    return;

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
