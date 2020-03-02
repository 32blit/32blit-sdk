#include "api.hpp"
#include "api_private.hpp"

namespace blit {
#ifdef TARGET_32BLIT_HW
  __attribute__((section(".api"))) API api;
#else
  API api;
#endif

  uint32_t &buttons = api.buttons;
  float &hack_left = api.hack_left;
  float &hack_right = api.hack_right;
  float &vibration = api.vibration;
  Vec2 &joystick = api.joystick;
  Vec3 &tilt = api.tilt;
  Pen &LED = api.LED;
}