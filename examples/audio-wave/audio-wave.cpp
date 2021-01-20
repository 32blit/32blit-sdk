#include <string>
#include <cstring>
#include <memory>
#include <cstdlib>

#include "audio-wave.hpp"

#include "assets.hpp"

/*
    Wave example:

    An example of an arbitrary waveform being played through the blit speaker.

    This example, a runthrough:

      Audio data:
      The audio file has been converted to 22050Hz sample rate, then exported without its header.
      (If a headered file is used, just read past that first. Try skipping the first 44 bytes).
      You would preferably store files separate because uncompressed audio will use all your flash!
      Here though, the raw wave to a c header using 'xxd -i glass.raw glass.h'

      As the data could be long, or even infinite, we can fill the audio buffer via a callback.

      Calling channels[n].trigger_attack() will run the playback and callback continuously until
      either channels[n].trigger_release() or channels[n].off() is called.


*/



using namespace blit;

void buff_callback(AudioChannel &);    //Declare our callback here instead of putting the whole thing here.

/* setup */
void init() {

  // Setup channel
  channels[0].waveforms            = Waveform::WAVE; // Set type to WAVE
  channels[0].wave_buffer_callback = &buff_callback;  // Set callback address

  screen.pen = Pen(0, 0, 0, 255);
  screen.clear();
}


// Static wave config
static uint32_t wav_size = 0;
static uint16_t wav_pos = 0;
static uint16_t wav_sample_rate = 0;
static const uint8_t *wav_sample;


// Called everytime audio buffer ends
void buff_callback(AudioChannel &channel) {

  // Copy 64 bytes to the channel audio buffer
  for (int x = 0; x < 64; x++) {
    // If current sample position is greater than the sample length, fill the rest of the buffer with zeros.
    // Note: The sample used here has an offset, so we adjust by 0x7f.
    channel.wave_buffer[x] = (wav_pos < wav_size) ? (wav_sample[wav_pos] << 8) - 0x7f00 : 0;

    // As the engine is 22050Hz, we can timestretch to match by incrementing our sample every other step (every even 'x')
    if (wav_sample_rate == 11025) {
      if (x % 2) wav_pos++;
    } else {
      wav_pos++;
    }
  }

  // For this example, clear the values
  if (wav_pos >= wav_size) {
    channel.off();        // Stop playback of this channel.
    //Clear buffer
    wav_sample = nullptr;
    wav_size = 0;
    wav_pos = 0;
    wav_sample_rate = 0;
  }
}

void render(uint32_t time_ms) {
  screen.pen = Pen(0, 0, 0);
	screen.clear();

	screen.alpha = 255;
	screen.pen = Pen(255, 255, 255);
	screen.rectangle(Rect(0, 0, 320, 14));
	screen.pen = Pen(0, 0, 0);
	screen.text("Wave Example", minimal_font, Point(5, 4));

  screen.pen = Pen(64, 64, 64);
	screen.text("Press A to break screen.", minimal_font, Point(20, 60));
}

void update(uint32_t time_ms) {
  bool button_a = blit::buttons & blit::Button::A;

  // If 'A' button pushed
  if(button_a){
    wav_sample = glass_wav;        // Set sample to the array in assets.hpp
    wav_size = glass_wav_length;      // Set the array length to the value in assets.hpp
    channels[0].trigger_attack(); // Start the playback.
  }
}

