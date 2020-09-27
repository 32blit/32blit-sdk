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

Input::Input(SDL_Window *window, System *target) : target(target) {
	int w, h;
	SDL_GetWindowSize(window, &w, &h);
	resize(w, h);
}

void Input::resize(int width, int height) {
	win_width = width;
	win_height = height;
}

bool Input::handle_mouse(int button, bool state, int x, int y) {
	if (button == SDL_BUTTON_LEFT) {
		if (state) {
			if(left_ctrl){
				_virtual_tilt(x, y);
			} else {
				x = x - (win_width / 2);
				y = y - (win_height / 2);
				_virtual_analog(x, y);
			}
		} else {
			_virtual_analog(0, 0);
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
	float fvalue = value / 32768.0;
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

void Input::_virtual_tilt(int x, int y) {
	int z = 80;
	x = x - (win_width / 2);
	y = y - (win_height / 2);
	blit::Vec3 shadow_tilt(x, y, z);
	shadow_tilt.normalize();
	target->set_tilt(0, shadow_tilt.x);
	target->set_tilt(1, shadow_tilt.y);
	target->set_tilt(2, shadow_tilt.z);
}

void Input::_virtual_analog(int x, int y) {
	float jx = (float)x / (win_width / 2);
	float jy = (float)y / (win_height / 2);
	target->set_joystick(0, jx);
	target->set_joystick(1, jy);
}
