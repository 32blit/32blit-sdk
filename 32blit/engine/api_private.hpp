#pragma once
#include <cstdarg>
#include <cstdint>
#include <vector>

#include "engine.hpp"
#include "file.hpp"
#include "../audio/audio.hpp"
#include "../engine/input.hpp"
#include "../graphics/jpeg.hpp"
#include "../graphics/surface.hpp"
#include "../types/vec2.hpp"
#include "../types/vec3.hpp"

namespace blit {

  using AllocateCallback = uint8_t *(*)(size_t);

  constexpr uint32_t api_version = 0;

  #pragma pack(push, 4)
  struct API {
    uint32_t version = api_version;

    ButtonState buttons;
    float hack_left;
    float hack_right;
    float vibration;
    Vec2 joystick;
    Vec3 tilt;
    Pen LED;

    AudioChannel *channels;

    Surface &(*set_screen_mode)  (ScreenMode new_mode);
    void (*set_screen_palette)  (const Pen *colours, int num_cols);
    uint32_t (*now)();
    uint32_t (*random)();
    void (*exit)(bool is_error);

    // serial debug
    void (*debug)(std::string message);
    int  (*debugf)(const char * psFormatString, va_list args);

    // files
    void *(*open_file)(const std::string &file, int mode);
    int32_t (*read_file)(void *fh, uint32_t offset, uint32_t length, char* buffer);
    int32_t (*write_file)(void *fh, uint32_t offset, uint32_t length, const char* buffer);
    int32_t (*close_file)(void *fh);
    uint32_t (*get_file_length)(void *fh);
    std::vector<FileInfo> (*list_files) (const std::string &path);
    bool (*file_exists) (const std::string &path);
    bool (*directory_exists) (const std::string &path);
    bool (*create_directory) (const std::string &path);
    bool (*rename_file) (const std::string &old_name, const std::string &new_name);
    bool (*remove_file) (const std::string &path);
    std::string (*get_save_path)();

    // profiler
    void (*enable_us_timer)();
    uint32_t (*get_us_timer)();
    uint32_t (*get_max_us_timer)();

    // jepg
    JPEGImage (*decode_jpeg_buffer)(const uint8_t *ptr, uint32_t len, AllocateCallback alloc);
    JPEGImage (*decode_jpeg_file)(std::string filename, AllocateCallback alloc);

  };
  #pragma pack(pop)

  extern API &api;
}