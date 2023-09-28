#pragma once

#ifndef ALLOW_HIRES
#define ALLOW_HIRES 0 // disable by default, mode switching isn't supported
#endif

#define AUDIO_MAX_SAMPLE_UPDATE 64

// spi
#define SD_SCK   5
#define SD_MOSI 18
#define SD_MISO 19
#define SD_CS   22
