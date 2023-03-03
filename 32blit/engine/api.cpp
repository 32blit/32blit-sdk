#include "api.hpp"
#include "api_private.hpp"

namespace blit {
  static_assert(sizeof(APIConst) < 2048);

  ButtonState &buttons = api_data.buttons;
  float &hack_left = api_data.hack_left;
  float &hack_right = api_data.hack_right;
  float &vibration = api_data.vibration;
  Vec2 &joystick = api_data.joystick;
  Vec3 &tilt = api_data.tilt;
  Pen &LED = api_data.LED;

  AudioChannel * const &channels = api.channels;
}
