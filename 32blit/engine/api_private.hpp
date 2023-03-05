#pragma once
#include <cstdarg>
#include <cstdint>
#include <vector>

#include "api_version.h"
#include "engine.hpp"
#include "file.hpp"
#include "../audio/audio.hpp"
#include "../engine/input.hpp"
#include "../engine/version.hpp"
#include "../graphics/jpeg.hpp"
#include "../graphics/surface.hpp"
#include "../types/vec2.hpp"
#include "../types/vec3.hpp"

// compatibility with API layout before const/data split
#ifdef BLIT_API_SPLIT_COMPAT
#define COMPAT_PAD(type, name, size) type name[size]
#else
#define COMPAT_PAD(type, name, size)
#endif

namespace blit {

  using AllocateCallback = uint8_t *(*)(size_t);

  constexpr uint16_t api_version_major = BLIT_API_VERSION_MAJOR, api_version_minor = BLIT_API_VERSION_MINOR;

  // template for screen modes
  struct SurfaceTemplate {
    uint8_t *data = nullptr;
    Size bounds;
    PixelFormat format;
    Pen *palette = nullptr;

    PenBlendFunc pen_blend = nullptr;
    BlitBlendFunc blit_blend = nullptr;
    PenGetFunc pen_get = nullptr;
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

  enum class CanLaunchResult {
    Success = 0,
    UnknownType,       /// no known handler for this file
    InvalidFile,      /// file is not valid/doesn't exist
    IncompatibleBlit, /// file is incompatible with this device
  };

  #pragma pack(push, 4)
  struct APIConst {
    uint16_t version_major;
    uint16_t version_minor;

    COMPAT_PAD(uint8_t, pad, 48); // was data

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
    COMPAT_PAD(uintptr_t, pad2, 1); // was message_recieved

    const uint8_t *(*flash_to_tmp)(const std::string &filename, uint32_t &size);
    void (*tmp_file_closed)(const uint8_t *ptr);

    GameMetadata (*get_metadata)();

    COMPAT_PAD(uint8_t, pad3, 1); // was data

    bool (*set_screen_mode_format)(ScreenMode new_mode, SurfaceTemplate &new_surf_template);

    // raw i2c access
    bool (*i2c_send)(uint8_t address, uint8_t reg, const uint8_t *data, uint16_t len);
    bool (*i2c_receive)(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len);
    COMPAT_PAD(uintptr_t, pad4, 1); // was i2c_completed

    // raw cdc
    bool (*set_raw_cdc_enabled)(bool enabled);
    void (*cdc_write)(const uint8_t *data, uint16_t len);
    uint16_t (*cdc_read)(uint8_t *data, uint16_t len);

    // another launcher API
    void (*list_installed_games)(std::function<void(const uint8_t *, uint32_t, uint32_t)> callback);
    // if launch is expected to succeed on this file
    // files this returns success for should be .blit files or have a registered handler (get_type_handler_metadata should return valid metadata)
    CanLaunchResult (*can_launch)(const char *path);
  };

  struct APIData {
    uint16_t pad[2];

    ButtonState buttons;
    float hack_left;
    float hack_right;
    float vibration;
    Vec2 joystick;
    Vec3 tilt;
    Pen LED;

    COMPAT_PAD(uintptr_t, pad2, 32);

    // multiplayer
    void (*message_received)(const uint8_t *data, uint16_t len); // set by user

    COMPAT_PAD(uintptr_t, pad3, 3);

    bool tick_function_changed;

    COMPAT_PAD(uintptr_t, pad4, 3);

    // raw i2c access
    void (*i2c_completed)(uint8_t address, uint8_t reg, const uint8_t *data, uint16_t len); // callback when done

    COMPAT_PAD(uintptr_t, pad5, 5);
  };

  #pragma pack(pop)

#ifdef BLIT_API_SPLIT_COMPAT
  static_assert(sizeof(APIConst) == sizeof(APIData));
#else
  static_assert(sizeof(APIData) <= 256);
#endif

  extern const APIConst &api;
  extern APIData &api_data;
}
