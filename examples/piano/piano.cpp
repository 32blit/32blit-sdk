#include <string>
#include <cstring>
#include <memory>
#include <cstdlib>

#include "piano.hpp"

using namespace blit;

uint16_t music_tick = 0;
uint16_t beat = 0;

constexpr int step_count = 30;
constexpr int channel_count = 1;

int16_t notes[channel_count][step_count];
int16_t note_y[41];
int16_t selected_note = 0;
int16_t selected_channel = 0;
bool playing = false;

int note_to_freq(int note) {
  return powf(2.0f, (float(note) / 12.0f)) * 293.67f;
}

/* setup */
void init() {
  set_screen_mode(hires);

  for(int channel = 0; channel < channel_count; channel++) {
    for(int beat = 0; beat < step_count; beat++) {
      notes[channel][beat] = 12;
    }
  }

  // configure voices

  // melody track
  channels[0].waveforms   = Waveform::TRIANGLE;
  channels[0].attack_ms   = 10;
  channels[0].decay_ms    = 350;
  channels[0].sustain     = 0;
  channels[0].release_ms  = 168;

  // rhythm track
  channels[1].waveforms   = Waveform::SINE;
  channels[1].attack_ms   = 5;
  channels[1].decay_ms    = 100;
  channels[1].sustain     = 0;
  channels[1].release_ms  = 0;

  // drum track
  channels[2].waveforms   = 0; // Waveform::NOISE;
  channels[2].attack_ms   = 10;
  channels[2].decay_ms    = 750;
  channels[2].sustain     = 0;
  channels[2].release_ms  = 100;

  // set global volume
  // volume = 2048;
  
  screen.pen = Pen(0, 0, 0, 255);
  screen.clear();  
}

void render(uint32_t time_ms) {
  screen.pen = Pen(20, 30, 40);
  screen.clear();

  for(int pass = 0; pass < 2; pass++) {
    for(int note = 0; note < 41; note++) {
      // White: 0, 2, 4, 5, 7, 9, 11
      // Black: 1, 3, 6, 8, 10
      int octave = floor(note / 12);
      int y = 70 * octave;
      int y_offset = 0;
      int w = 20;
      int h = 9;
      switch(note % 12) {
        case 11:
          y += 10;
        case 9:
          y += 10;
        case 7:
          y += 10;
        case 5:
          y += 10;
        case 4:
          y += 10;
        case 2:
          y += 10;
        case 0:
          y_offset = 3;
          if (pass == 1) continue;
          screen.pen = Pen(255, 255, 255);
          break;
        case 3:
          y += 10;
        case 1:
          y += 4;
          w = 15;
          h = 7;
          y_offset = 2;
          if (pass == 0) continue;
          screen.pen = Pen(0, 0, 0);
          break;
        case 10:
          y += 10;
        case 8:
          y += 10;
        case 6:
          y += 34;
          w = 15;
          h = 7;
          y_offset = 2;
          if (pass == 0) continue;
          screen.pen = Pen(0, 0, 0);
          break;
      }
      y = screen.bounds.h - y - 10;
      note_y[note] = y + y_offset;
      screen.rectangle(Rect(
        0, y,
        w, h
      ));
      screen.rectangle(Rect(
        20, y,
        300, h
      ));
    }
  }

  for(int x = 0; x < 30; x++) {
    screen.pen = x & 1 ? Pen(20, 30, 40, 220) : Pen(30, 40, 50, 220);
    screen.rectangle(Rect(
      20 + x * 10, 0,
      10, 240
    ));
    int note = notes[selected_channel][x] & ~0x100;
    bool mute = notes[selected_channel][x] & 0x100;
    if(note > -1){
      screen.pen = mute ? Pen(255, 255, 255, 100) : Pen(255, 255, 255, 255);
      screen.rectangle(Rect(
        21 + x * 10, note_y[note],
        8, 3
      ));
    }
  }

  screen.pen = Pen(255, 200, 200);
  screen.rectangle(Rect(
    20 + beat * 10, note_y[notes[selected_channel][beat] & ~0x100] - 1,
    10, 5
  ));

  screen.pen = Pen(255, 0, 0);
  screen.rectangle(Rect(
    20 + selected_note * 10, note_y[notes[selected_channel][selected_note] & ~0x100] - 1,
    10, 5
  ));

  screen.watermark();  
}

void play_selected_note() {
  if(playing) return;
  int16_t hz = note_to_freq(notes[selected_channel][selected_note] & ~0x100);
  channels[selected_channel].frequency = hz;
  channels[selected_channel].trigger_attack();
}

void play() {
  if(!playing) return;
  static uint16_t prev_beat = 1;
  beat = (music_tick / 25) % step_count; // 200ms per beat
  music_tick++;

  if (beat == prev_beat) return;
  prev_beat = beat;

  for(uint8_t i = 0; i < channel_count; i++) {
    if(notes[i][beat] & 0x100) {
      channels[i].trigger_release();
    } else {
      int16_t hz = note_to_freq(notes[i][beat] & ~0x100);
      channels[i].frequency = hz;
      channels[i].trigger_attack();
    }
  }
}

void update(uint32_t time_ms) {
  play();

  if(buttons.pressed & Button::DPAD_LEFT) {
    if(selected_note > 0){
      selected_note--;
      play_selected_note();
    }
  }
  if(buttons.pressed & Button::DPAD_RIGHT) {
    if(selected_note < 30){
      selected_note++;
      play_selected_note();
    }
  }
  if(buttons.pressed & Button::DPAD_UP) {
    if(notes[selected_channel][selected_note] < 41) {
      notes[selected_channel][selected_note]++;
      play_selected_note();
    }
  }
  if(buttons.pressed & Button::DPAD_DOWN) {
    if(notes[selected_channel][selected_note] > 0) {
      notes[selected_channel][selected_note]--;
      play_selected_note();
    }
  }
  if(buttons.pressed & Button::A) {
    playing = !playing;
  }

  if(buttons.pressed & Button::B) {
    bool mute = notes[0][selected_note] & 0x100;
    notes[0][selected_note] &= ~0x100;
    if(!mute) {
      notes[selected_channel][selected_note] |= 0x100;
    }
  }

  if(buttons.pressed & Button::Y) {
    music_tick = 0;
    beat = 0;
  }
}
  
