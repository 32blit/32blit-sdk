#include "api.hpp"
#include "api_private.hpp"

#ifdef TARGET_32BLIT_HW
extern char __api_start;
#endif

namespace blit {
#ifdef TARGET_32BLIT_HW
  API &api = *(API *)&__api_start;
#else
  API real_api;
  API &api = real_api;
#endif

  uint32_t &buttons = api.buttons;
  float &hack_left = api.hack_left;
  float &hack_right = api.hack_right;
  float &vibration = api.vibration;
  Vec2 &joystick = api.joystick;
  Vec3 &tilt = api.tilt;
  Pen &LED = api.LED;

  AudioChannel *&channels = api.channels;
}