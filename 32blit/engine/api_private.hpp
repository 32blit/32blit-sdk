#pragma once
#include <cstdarg>
#include <cstdint>
#include <vector>

#include "engine.hpp"
#include "file.hpp"
#include "../audio/audio.hpp"
#include "../engine/input.hpp"
#include "../engine/version.hpp"
#include "../graphics/jpeg.hpp"
#include "../graphics/surface.hpp"
#include "../types/vec2.hpp"
#include "../types/vec3.hpp"

namespace blit {

  using AllocateCallback = uint8_t *(*)(size_t);

  constexpr uint16_t api_version_major = 0, api_version_minor = 1;

  // template for screen modes
  struct SurfaceTemplate {
    uint8_t *data = nullptr;
    Size bounds;
    PixelFormat format;
    Pen *palette = nullptr;
  };

  // subset of Surface for API compat
  struct SurfaceInfo {
    SurfaceInfo() = default;
    SurfaceInfo(const Surface &surf): data(surf.data), bounds(surf.bounds), format(surf.format), palette(surf.palette) {}
    SurfaceInfo(const SurfaceTemplate &surf): data(surf.data), bounds(surf.bounds), format(surf.format), palette(surf.palette) {}

    uint8_t *data = nullptr;
    Size bounds;

    // unused, here for compat reasons
    Rect clip;
    uint8_t alpha;
    Pen pen;

    PixelFormat format;
    uint8_t pixel_stride; // unused
    uint16_t row_stride; // unused

    Surface *mask = nullptr; // unused
    Pen *palette = nullptr;
  };

  #pragma pack(push, 4)
  struct API {
    uint16_t version_major;
    uint16_t version_minor;

    ButtonState buttons;
    float hack_left;
    float hack_right;
    float vibration;
    Vec2 joystick;
    Vec3 tilt;
    Pen LED;

    AudioChannel *channels;

    SurfaceInfo &(*set_screen_mode)  (ScreenMode new_mode);
    void (*set_screen_palette)  (const Pen *colours, int num_cols);
    uint32_t (*now)();
    uint32_t (*random)();
    void (*exit)(bool is_error);

    // serial debug
    void (*debug)(const char *message);

    // files
    void *(*open_file)(const std::string &file, int mode);
    int32_t (*read_file)(void *fh, uint32_t offset, uint32_t length, char* buffer);
    int32_t (*write_file)(void *fh, uint32_t offset, uint32_t length, const char* buffer);
    int32_t (*close_file)(void *fh);
    uint32_t (*get_file_length)(void *fh);
    void (*list_files) (const std::string &path, std::function<void(FileInfo &)> callback);
    bool (*file_exists) (const std::string &path);
    bool (*directory_exists) (const std::string &path);
    bool (*create_directory) (const std::string &path);
    bool (*rename_file) (const std::string &old_name, const std::string &new_name);
    bool (*remove_file) (const std::string &path);
    const char *(*get_save_path)();
    bool (*is_storage_available)();

    // profiler
    void (*enable_us_timer)();
    uint32_t (*get_us_timer)();
    uint32_t (*get_max_us_timer)();

    // jepg
    JPEGImage (*decode_jpeg_buffer)(const uint8_t *ptr, uint32_t len, AllocateCallback alloc);
    JPEGImage (*decode_jpeg_file)(const std::string &filename, AllocateCallback alloc);

    // launcher APIs - only intended for use by launchers and only available on device
    bool (*launch)(const char *filename);
    void (*erase_game)(uint32_t offset);
    void *(*get_type_handler_metadata)(const char *filetype);

    const char *(*get_launch_path)();

    // multiplayer
    bool (*is_multiplayer_connected)();
    void (*set_multiplayer_enabled)(bool enabled);
    void (*send_message)(const uint8_t *data, uint16_t len);
    void (*message_received)(const uint8_t *data, uint16_t len); // set by user

    const uint8_t *(*flash_to_tmp)(const std::string &filename, uint32_t &size);
    void (*tmp_file_closed)(const uint8_t *ptr);

    GameMetadata (*get_metadata)();

    bool tick_function_changed;

    bool (*set_screen_mode_format)(ScreenMode new_mode, SurfaceTemplate &new_surf_template);

    // raw i2c access
    bool (*i2c_send)(uint8_t address, uint8_t reg, const uint8_t *data, uint16_t len);
    bool (*i2c_receive)(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len);
    void (*i2c_completed)(uint8_t address, uint8_t reg, const uint8_t *data, uint16_t len); // callback when done

    // raw cdc
    bool (*set_raw_cdc_enabled)(bool enabled);
    void (*cdc_write)(const uint8_t *data, uint16_t len);
    uint16_t (*cdc_read)(uint8_t *data, uint16_t len);

    // another launcher API
    void (*list_installed_games)(std::function<void(const uint8_t *, uint32_t, uint32_t)> callback);
  };
  #pragma pack(pop)

  extern API &api;
}
