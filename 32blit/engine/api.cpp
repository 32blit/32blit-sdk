#include "api.hpp"
#include "api_private.hpp"

namespace blit {
  static_assert(sizeof(API) < 2048);

  ButtonState &buttons = api.buttons;
  float &hack_left = api.hack_left;
  float &hack_right = api.hack_right;
  float &vibration = api.vibration;
  Vec2 &joystick = api.joystick;
  Vec3 &tilt = api.tilt;
  Pen &LED = api.LED;

  AudioChannel *&channels = api.channels;
}
