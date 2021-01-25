#include "waveform-demo.hpp"

using namespace blit;

enum CurrentSound {
  NO_SOUND,
  A_SOUND,
  B_SOUND,
  X_SOUND,
  Y_SOUND,
  UP_SOUND,
  DOWN_SOUND,
  LEFT_SOUND,
  RIGHT_SOUND
};

// current_sound determines which sound is currently playing.
// -1 indicates no sound.
CurrentSound current_sound = NO_SOUND;

// sound_sweep is used when changing the frequency of the waveform during playback
float sound_sweep = 0.0f;

// start_time is used to calculate how long a waveform has been playing for (e.g. to do frequency jumps)
uint32_t start_time = 0;

// frequency_jump_done is used to check if we've jumped the frequency yet
bool frequency_jump_done = false;

// sound_time is used to store how long a waveform has been playing for
uint32_t sound_time = 0;

///////////////////////////////////////////////////////////////////////////
//
// init()
//
// setup your game here
//
void init() {
  set_screen_mode(ScreenMode::lores);

  // Set up waveform presets.

  channels[0].waveforms = Waveform::SQUARE;
  channels[0].frequency = 0;
  channels[0].attack_ms = 5;
  channels[0].decay_ms = 400;
  channels[0].sustain = 0;
  channels[0].release_ms = 5;

  channels[1].waveforms = Waveform::TRIANGLE;
  channels[1].frequency = 0;
  channels[1].attack_ms = 5;
  channels[1].decay_ms = 300;
  channels[1].sustain = 0;
  channels[1].release_ms = 5;

  channels[2].waveforms = Waveform::TRIANGLE;
  channels[2].frequency = 800;
  channels[2].attack_ms = 5;
  channels[2].decay_ms = 500;
  channels[2].sustain = 0;
  channels[2].release_ms = 5;

  channels[3].waveforms = Waveform::TRIANGLE;
  channels[3].frequency = 0;
  channels[3].attack_ms = 5;
  channels[3].decay_ms = 400;
  channels[3].sustain = 0;
  channels[3].release_ms = 5;

  channels[4].waveforms = Waveform::NOISE;
  channels[4].frequency = 7700;
  channels[4].attack_ms = 5;
  channels[4].decay_ms = 350;
  channels[4].sustain = 0;
  channels[4].release_ms = 5;

  channels[5].waveforms = Waveform::SINE;
  channels[5].frequency = 0;
  channels[5].attack_ms = 5;
  channels[5].decay_ms = 400;
  channels[5].sustain = 0;
  channels[5].release_ms = 5;

  channels[6].waveforms = Waveform::TRIANGLE;
  channels[6].frequency = 1400;
  channels[6].attack_ms = 5;
  channels[6].decay_ms = 100;
  channels[6].sustain = 0;
  channels[6].release_ms = 5;

  channels[7].waveforms = Waveform::SINE;
  channels[7].frequency = 0;
  channels[7].attack_ms = 5;
  channels[7].decay_ms = 500;
  channels[7].sustain = 0;
  channels[7].release_ms = 5;
}

///////////////////////////////////////////////////////////////////////////
//
// render(time)
//
// This function is called to perform rendering of the game. time is the 
// amount if milliseconds elapsed since the start of your game
//
void render(uint32_t time) {

  // Clear the screen.
  screen.clear();

  // Draw some text ont the screen.
  screen.alpha = 255;
  screen.mask = nullptr;
  screen.pen = Pen(255, 255, 255);
  screen.rectangle(Rect(0, 0, 160, 14));
  screen.text("Press a button to", minimal_font, Point(5, 24));
  screen.text("play a sound", minimal_font, Point(5, 32));

  std::string text = "Current sound: ";

  if (current_sound != NO_SOUND) {
    // A sound is playing currently.
    // Display text indicating which sound.

    text.append(std::to_string(current_sound));
  }

  screen.text(text, minimal_font, Point(5, 48));

  screen.pen = Pen(0, 0, 0);
  screen.text("Waveform Demo", minimal_font, Point(5, 4));
}

///////////////////////////////////////////////////////////////////////////
//
// update(time)
//
// This is called to update your game state. time is the 
// amount if milliseconds elapsed since the start of your game
//
void update(uint32_t time) {
  if (current_sound == NO_SOUND) {
    // No sound is currently playing, let's see if we need to play a sound now...

    // Generate a sound if a button is pressed (and only if no other sounds are playing).

    // Only detect one button press at a time (by using else if) because reasons.

    if (buttons.pressed & Button::X) {
      current_sound = X_SOUND;

      channels[0].trigger_attack();
      sound_sweep = 1.0f;

      start_time = time;
    }
    else if (buttons.pressed & Button::A) {
      current_sound = A_SOUND;

      channels[1].trigger_attack();
      sound_sweep = 1.0f;

      start_time = time;
    }
    else if (buttons.pressed & Button::B) {
      current_sound = B_SOUND;

      channels[2].frequency = 800;
      channels[2].trigger_attack();

      frequency_jump_done = false;
      start_time = time;
    }
    else if (buttons.pressed & Button::Y) {
      current_sound = Y_SOUND;

      channels[3].trigger_attack();
      sound_sweep = 1.0f;

      start_time = time;
    }
    else if (buttons.pressed & Button::DPAD_UP) {
      current_sound = UP_SOUND;

      channels[4].trigger_attack();

      start_time = time;
    }
    else if (buttons.pressed & Button::DPAD_RIGHT) {
      current_sound = RIGHT_SOUND;

      channels[5].trigger_attack();
      sound_sweep = 1.0f;

      start_time = time;
    }
    else if (buttons.pressed & Button::DPAD_DOWN) {
      current_sound = DOWN_SOUND;

      channels[6].trigger_attack();

      start_time = time;
    }
    else if (buttons.pressed & Button::DPAD_LEFT) {
      current_sound = LEFT_SOUND;

      channels[7].trigger_attack();
      sound_sweep = 1.0f;

      start_time = time;
    }
  }

  if (current_sound == NO_SOUND) {
    sound_time = 0;
  }
  else {
    sound_time = time - start_time;
  }

  // Update the sounds which are playing.

  switch (current_sound) {
  case X_SOUND:
    if (sound_time > 400) {
      // Stop sound
      current_sound = NO_SOUND;
      channels[0].trigger_release();
    }
    else {
      // Update the sweep
      channels[0].frequency = 1000 - (400.0f * sound_sweep);
      sound_sweep -= 0.01f;
    }
    break;

  case A_SOUND:
    if (sound_time > 300) {
      // Stop sound
      current_sound = NO_SOUND;
      channels[1].trigger_release();
    }
    else {
      // Update the sweep
      channels[1].frequency = 1300 - (1000.0f * sound_sweep);
      sound_sweep -= 0.01f;
    }
    break;

  case B_SOUND:
    if (sound_time > 500) {
      // Stop sound
      current_sound = NO_SOUND;
      channels[2].trigger_release();
    }
    else if (sound_time > 200 && !frequency_jump_done) {
      // Jump the frequency by 60%
      channels[2].frequency *= 1.6;
      frequency_jump_done = true;
    }
    break;

  case Y_SOUND:
    if (sound_time > 400) {
      // Stop sound
      current_sound = NO_SOUND;
      channels[3].trigger_release();
    }
    else {
      // Update the sweep
      channels[3].frequency = 1300.0f * sound_sweep;
      sound_sweep -= 0.01f;
    }
    break;

  case UP_SOUND:
    if (sound_time > 350) {
      // Stop sound
      current_sound = NO_SOUND;
      channels[4].trigger_release();
    }
    break;

  case RIGHT_SOUND:
    if (sound_time > 400) {
      // Stop sound
      current_sound = NO_SOUND;
      channels[5].trigger_release();
    }
    else {
      // Update the sweep
      channels[5].frequency = 1800 - (1000.0f * sound_sweep);
      sound_sweep -= 0.05f;
    }
    break;

  case DOWN_SOUND:
    if (sound_time > 100) {
      // Stop sound
      current_sound = NO_SOUND;
      channels[6].trigger_release();
    }
    break;

  case LEFT_SOUND:
    if (sound_time > 500) {
      // Stop sound
      current_sound = NO_SOUND;
      channels[7].trigger_release();
    }
    else {
      // Update the sweep
      channels[7].frequency = (3000.0f * sound_sweep) - 800;
      sound_sweep -= 0.05f;
    }
    break;

  default:
    // No sound playing
    break;
  }
}
