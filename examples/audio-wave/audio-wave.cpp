#include <string>
#include <string.h>
#include <memory>
#include <cstdlib>

#include "audio-wave.hpp"

#include "glass.h"

/*
    Wave example:
    
    An example of an arbitrary waveform being played through the blit speaker.

    This example, a runthrough:

      Audio data:
      The audio file has been converted to 11025Hz sample rate, then exported as 16bit PCM, but without its header.
      (If a headered file is used, just read past that first. Try skipping the first 44 bytes).
      You would preferably store files separate because uncompressed audio will use all your flash!
      Here though, the raw wave to a c header using 'xxd -i glass.raw glass.h'

      As the data could be long, or even infinite, we can fill the audio buffer via a callback.

      Calling channels[n].trigger_attack() will run the playback and callback continuously until
      either channels[n].trigger_release() or channels[n].off() is called.


*/



using namespace blit;

void buffCallBack();    //Declare our callback here instead of putting the whole thing here.

/* setup */
void init() {

  // Setup channel
  channels[0].waveforms   = Waveform::WAVE;                // Set type to WAVE
  channels[0].sustain     = 0xffff;                        // Set sustain to max
  channels[0].callback_waveBufferRefresh = &buffCallBack;  // Set callback address

  screen.pen(RGBA(0, 0, 0, 255));
  screen.clear();
}


// Static wave config
static uint32_t wavSize = 0;
static uint16_t wavPos = 0;
static uint16_t wavSampleRate = 0;
static const uint16_t *wavSample;


// Called everytime audio buffer ends
void buffCallBack() {

  // Copy 64 bytes to the channel audio buffer
  for (int x = 0; x < 64; x++) {
    // If current sample position is greater than the sample length, fill the rest of the buffer with zeros.
    // Note: The sample used here has an offset, so we adjust by 0x7f. 
    channels[0].wave_buffer[x] = (wavPos < wavSize) ? wavSample[wavPos] - 0x7f : 0;

    // As the engine is 22050Hz, we can timestretch to match by incrementing our sample every other step (every even 'x')
    if (wavSampleRate == 11025) {
      if (x % 2) wavPos++;
    } else {
      wavPos++;
    }
  }
  
  // For this example, clear the values
  if (wavPos >= wavSize) {
    channels[0].off();        // Stop playback of this channel.
    //Clear buffer
    wavSample = 0;
    wavSize = 0;
    wavPos = 0;
    wavSampleRate = 0;
  }
}

void render(uint32_t time_ms) {
  screen.pen(RGBA(0, 0, 0));
	screen.clear();

	screen.alpha = 255;
	screen.pen(RGBA(255, 255, 255));
	screen.rectangle(Rect(0, 0, 320, 14));
	screen.pen(RGBA(0, 0, 0));
	screen.text("Wave Example", &minimal_font[0][0], Point(5, 4));

  screen.pen(RGBA(64, 64, 64));
	screen.text("Press A to break screen.", &minimal_font[0][0], Point(20, 60));


  bool button_a = blit::buttons & blit::Button::A;
  
  // If 'A' button pushed
  if(button_a){
    wavSample = glass_wav;        // Set sample to the array in glass.h
    wavSampleRate = 11025;        // Note the sample rate
    wavSize = glass_wav_len;      // Set the array length to the value in glass.h
    channels[0].trigger_attack(); // Start the playback.
  }
} 

void update(uint32_t time_ms) {

}
  
