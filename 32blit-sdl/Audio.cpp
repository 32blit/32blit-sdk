#include <algorithm>
#include <cstdio>
#include <iostream>
#include "SDL.h"

#include "Audio.hpp"
#include "engine/audio.hpp"

Audio::Audio() {
    SDL_AudioSpec *desired;

    desired = new SDL_AudioSpec();
    audio_spec = new SDL_AudioSpec();

    desired->freq = _sample_rate;
    desired->format = AUDIO_U16LSB;
    desired->channels = 1;

    desired->samples = 128;
    desired->callback = _audio_callback;

    audio_device = SDL_OpenAudioDevice(NULL, 0, desired, audio_spec, 0);

    delete desired;

    if(audio_device == 0){
        std::cout << "Audio Init Failed: " << SDL_GetError() << std::endl;
    }

    std::cout << "Audio Init Done" << std::endl;

    SDL_PauseAudioDevice(audio_device, 0);
}

Audio::~Audio() {
    SDL_PauseAudioDevice(audio_device, 1);
    SDL_CloseAudioDevice(audio_device);
}

void _audio_bufferfill(unsigned short *buffer, int buffer_size){
    memset(buffer, 0, buffer_size);

    for(auto sample = 0; sample < buffer_size; sample++){
        buffer[sample] = blit::audio::get_audio_frame();
    }
}

void _audio_callback(void *userdata, uint8_t *stream, int len){
    _audio_bufferfill((unsigned short *)stream, len / 2);
}