#pragma once
#include <cstdint>
#include <vector>

#include "engine.hpp"
#include "file.hpp"
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

    Surface     &(*set_screen_mode)  (ScreenMode new_mode);

    // files
    std::vector<FileInfo> (*list_files) (std::string path);
  
  };
  #pragma pack(pop)

  extern API api;
}