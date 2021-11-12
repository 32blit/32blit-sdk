#include "SDL.h"

#include "Input.hpp"
#include "engine/input.hpp"
#include "System.hpp"

std::map<int, int> Input::keys = {
	// arrow keys
	{SDLK_DOWN,   blit::Button::DPAD_DOWN},
	{SDLK_UP,     blit::Button::DPAD_UP},
	{SDLK_LEFT,   blit::Button::DPAD_LEFT},
	{SDLK_RIGHT,  blit::Button::DPAD_RIGHT},

	// wasd
	{SDLK_w,       blit::Button::DPAD_UP},
	{SDLK_a,       blit::Button::DPAD_LEFT},
	{SDLK_s,       blit::Button::DPAD_DOWN},
	{SDLK_d,       blit::Button::DPAD_RIGHT},

	// action buttons
	{SDLK_z,       blit::Button::A},
	{SDLK_x,       blit::Button::B},
	{SDLK_c,       blit::Button::X},
	{SDLK_v,       blit::Button::Y},

	{SDLK_u,       blit::Button::A},
	{SDLK_i,       blit::Button::B},
	{SDLK_o,       blit::Button::X},
	{SDLK_p,       blit::Button::Y},

	// system buttons
	{SDLK_1,       blit::Button::HOME},
	{SDLK_2,       blit::Button::MENU},
	{SDLK_3,       blit::Button::JOYSTICK},
  
  {SDLK_ESCAPE,  blit::Button::MENU},
};

std::map<int, int> Input::buttons = {
	// dpad
	{SDL_CONTROLLER_BUTTON_DPAD_DOWN,   blit::Button::DPAD_DOWN},
	{SDL_CONTROLLER_BUTTON_DPAD_UP,     blit::Button::DPAD_UP},
	{SDL_CONTROLLER_BUTTON_DPAD_LEFT,   blit::Button::DPAD_LEFT},
	{SDL_CONTROLLER_BUTTON_DPAD_RIGHT,  blit::Button::DPAD_RIGHT},

	// action buttons
	{SDL_CONTROLLER_BUTTON_A,           blit::Button::A},
	{SDL_CONTROLLER_BUTTON_B,           blit::Button::B},
	{SDL_CONTROLLER_BUTTON_X,           blit::Button::X},
	{SDL_CONTROLLER_BUTTON_Y,           blit::Button::Y},

	// system buttons
	{SDL_CONTROLLER_BUTTON_BACK,        blit::Button::HOME},
	{SDL_CONTROLLER_BUTTON_START,       blit::Button::MENU},
	{SDL_CONTROLLER_BUTTON_LEFTSTICK,   blit::Button::JOYSTICK},
};

int Input::find_key(int key) {
	auto iter = keys.find(key);
	if (iter == keys.end()) return 0;
	else return iter->second;
}

int Input::find_button(int button) {
	auto iter = buttons.find(button);
	if (iter == buttons.end()) return 0;
	else return iter->second;
}

Input::Input(System *target) : target(target) {
  // Open all joysticks as game controllers
  for(int n = 0; n < SDL_NumJoysticks(); n++) {

    if(SDL_IsGameController(n)) {

      auto gc = SDL_GameControllerOpen(n);
      if(gc != nullptr) {
        _add_controller(gc);
      }
    }
  }
}

Input::~Input() {
  for (auto gc : game_controllers)
  {
    _remove_controller(gc.gc_id);
  }
  game_controllers.clear();
}

bool Input::handle_mouse(int button, bool state, int x, int y) {

  int half_w = target->mode() ? target->width / 2 : target->width / 4;
  int half_h = target->mode() ? target->height / 2 : target->height / 4;

	if (button == SDL_BUTTON_LEFT) {
		if (state) {
			if(left_ctrl){
				_virtual_tilt(x, y, half_w, half_h);
			} else {
				x = x - half_w;
				y = y - half_h;
				_virtual_analog(x, y, half_w, half_h);
			}
		} else {
			_virtual_analog(0, 0, half_w, half_h);
		}
		return true;
	}
	return false;
}

bool Input::handle_keyboard(int key, bool state) {
	if (int blit_button = find_key(key)) {
		target->set_button(blit_button, state);
		return true;
	} else if (key == SDLK_LCTRL) {
		left_ctrl = state;
		return true;
	}
	return false;
}

bool Input::handle_controller_button(int button, bool state) {
	if (int blit_button = find_button(button)) {
		target->set_button(blit_button, state);
		return true;
	}
	return false;
}

bool Input::handle_controller_motion(int axis, int value) {
	float fvalue = value / 32768.0f;
	switch(axis) {
		case SDL_CONTROLLER_AXIS_LEFTX:
			target->set_joystick(0, fvalue);
			return true;
		case SDL_CONTROLLER_AXIS_LEFTY:
			target->set_joystick(1, fvalue);
			return true;
		case SDL_CONTROLLER_AXIS_RIGHTX:
			target->set_tilt(0, fvalue);
			return true;
		case SDL_CONTROLLER_AXIS_RIGHTY:
			target->set_tilt(1, fvalue);
			return true;
		default:
			break;
	}
	return false;
}

void Input::handle_controller_added(Sint32 joystick_index) {

  SDL_GameController* gc = SDL_GameControllerOpen(joystick_index);
  if (gc != nullptr) {
    _add_controller(gc);
  }
}

void Input::handle_controller_removed(Sint32 joystick_index) {

  auto gc_removed = SDL_GameControllerFromInstanceID(joystick_index);
  if(gc_removed != nullptr) {
    _remove_controller(gc_removed);
  }
}

void Input::rumble_controllers(const float& volume) {

  if(volume > 0) {
    for (auto gc : game_controllers) {
      if(gc.can_rumble) {
        auto frequency = volume * 0xFFFF;
        SDL_GameControllerRumble(gc.gc_id, frequency, frequency, 50);
      }
    }
  }
}

void Input::_virtual_tilt(int x, int y, int half_w, int half_h) {
	float z = 80.0f;
	x = x - half_w;
	y = y - half_h;
	blit::Vec3 shadow_tilt(x, y, z);
	shadow_tilt.normalize();
	target->set_tilt(0, shadow_tilt.x);
	target->set_tilt(1, shadow_tilt.y);
	target->set_tilt(2, shadow_tilt.z);
}

void Input::_virtual_analog(int x, int y, int half_w, int half_h) {
	float jx = (float)x / half_w;
	float jy = (float)y / half_h;
	target->set_joystick(0, jx);
	target->set_joystick(1, jy);
}

void Input::_add_controller(SDL_GameController* gc) {
  // welcome rumble to test if it can rumble
  auto can_rumble = SDL_GameControllerRumble(gc, 0xFFFF, 0xFFFF, 200);

  GameController gcs = {gc, can_rumble == 0};
  game_controllers.push_back(gcs);
}

void Input::_remove_controller(SDL_GameController* gc) {
  game_controllers.erase(
    std::remove_if(game_controllers.begin(), game_controllers.end(), [&](GameController const & controller) {
        return controller.gc_id == gc;
    }),
    game_controllers.end());

  SDL_GameControllerClose(gc);
}
