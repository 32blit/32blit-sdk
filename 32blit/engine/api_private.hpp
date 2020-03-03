#pragma once
#include <cstdarg>
#include <cstdint>
#include <vector>

#include "engine.hpp"
#include "file.hpp"
#include "../audio/audio.hpp"
#include "../graphics/surface.hpp"
#include "../types/vec2.hpp"
#include "../types/vec3.hpp"

namespace blit {
  #pragma pack(push, 4)
  struct API {
    uint32_t buttons;
    float hack_left;
    float hack_right;
    float vibration;
    Vec2 joystick;
    Vec3 tilt;
    Pen LED;

    AudioChannel *channels;

    Surface     &(*set_screen_mode)  (ScreenMode new_mode);
    uint32_t (*now)();
    uint32_t (*random)();

    // serial debug
    void (*debug)(std::string message);
    int  (*debugf)(const char * psFormatString, va_list args);

    // files
    void *(*open_file)(std::string file, int mode);
    int32_t (*read_file)(void *fh, uint32_t offset, uint32_t length, char* buffer);
    int32_t (*write_file)(void *fh, uint32_t offset, uint32_t length, const char* buffer);
    int32_t (*close_file)(void *fh);
    uint32_t (*get_file_length)(void *fh);
    std::vector<FileInfo> (*list_files) (std::string path);
    bool (*file_exists) (std::string path);
    bool (*directory_exists) (std::string path);
    bool (*create_directory) (std::string path);

    // profiler
    void (*EnableUsTimer)(void);
    uint32_t (*GetUsTimer)(void);
    uint32_t (*GetMaxUsTimer)(void);
  
  };
  #pragma pack(pop)

  extern API api;
}