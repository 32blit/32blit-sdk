#include <SDL.h>

#include "Input.hpp"
#include "engine/input.hpp"
#include "System.hpp"

std::map<int, int> Input::keys = {
	// arrow keys
	{SDLK_DOWN,   blit::button::DPAD_DOWN},
	{SDLK_UP,     blit::button::DPAD_UP},
	{SDLK_LEFT,   blit::button::DPAD_LEFT},
	{SDLK_RIGHT,  blit::button::DPAD_RIGHT},

	// wasd
	{SDLK_w,       blit::button::DPAD_UP},
	{SDLK_a,       blit::button::DPAD_LEFT},
	{SDLK_s,       blit::button::DPAD_DOWN},
	{SDLK_d,       blit::button::DPAD_RIGHT},

	// action buttons
	{SDLK_z,       blit::button::A},
	{SDLK_x,       blit::button::B},
	{SDLK_c,       blit::button::X},
	{SDLK_y,       blit::button::Y},

	// system buttons
	{SDLK_1,       blit::button::HOME},
	{SDLK_2,       blit::button::MENU},
	{SDLK_3,       blit::button::JOYSTICK},
};

std::map<int, int> Input::buttons = {
	// dpad
	{SDL_CONTROLLER_BUTTON_DPAD_DOWN,   blit::button::DPAD_DOWN},
	{SDL_CONTROLLER_BUTTON_DPAD_UP,     blit::button::DPAD_UP},
	{SDL_CONTROLLER_BUTTON_DPAD_LEFT,   blit::button::DPAD_LEFT},
	{SDL_CONTROLLER_BUTTON_DPAD_RIGHT,  blit::button::DPAD_RIGHT},

	// action buttons
	{SDL_CONTROLLER_BUTTON_A,           blit::button::A},
	{SDL_CONTROLLER_BUTTON_B,           blit::button::B},
	{SDL_CONTROLLER_BUTTON_X,           blit::button::X},
	{SDL_CONTROLLER_BUTTON_Y,           blit::button::Y},

	// system buttons
	{SDL_CONTROLLER_BUTTON_BACK,        blit::button::HOME},
	{SDL_CONTROLLER_BUTTON_START,       blit::button::MENU},
	{SDL_CONTROLLER_BUTTON_LEFTSTICK,   blit::button::JOYSTICK},
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

Input::Input(SDL_Window *window) {
	int w, h;
	SDL_GetWindowSize(window, &w, &h);
	resize(w, h);
}

void Input::resize(int width, int height) {
	win_width = width;
	win_height = height;
}

bool Input::handle_mouse(System *sys, int button, bool state, int x, int y) {
	if (button == SDL_BUTTON_LEFT) {
		if (state) {
			if(left_ctrl){
				_virtual_tilt(sys, x, y);
			} else {
				x = x - (win_width / 2);
				y = y - (win_height / 2);
				_virtual_analog(sys, x, y);
			}
		} else {
			_virtual_analog(sys, 0, 0);
		}
		return true;
	}
	return false;
}

bool Input::handle_keyboard(System *sys, int key, bool state) {
	if (int blit_button = find_key(key)) {
		sys->set_button(blit_button, state);
		return true;
	} else if (key == SDLK_LCTRL) {
		left_ctrl = state;
		return true;
	}
	return false;
}

bool Input::handle_controller_button(System *sys, int button, bool state) {
	if (int blit_button = find_button(button)) {
		sys->set_button(blit_button, state);
		return true;
	}
	return false;
}

bool Input::handle_controller_motion(System *sys, int axis, int value) {
	float fvalue = value / 32768.0;
	switch(axis) {
		case SDL_CONTROLLER_AXIS_LEFTX:
			sys->set_joystick(0, fvalue);
			return true;
		case SDL_CONTROLLER_AXIS_LEFTY:
			sys->set_joystick(1, fvalue);
			return true;
		case SDL_CONTROLLER_AXIS_RIGHTX:
			sys->set_tilt(0, fvalue);
			return true;
		case SDL_CONTROLLER_AXIS_RIGHTY:
			sys->set_tilt(1, fvalue);
			return true;
		default:
			break;
	}
	return false;
}

void Input::_virtual_tilt(System *sys, int x, int y) {
	int z = 80;
	x = x - (win_width / 2);
	y = y - (win_height / 2);
	vec3 shadow_tilt = vec3(x, y, z);
	shadow_tilt.normalize();
	sys->set_tilt(0, shadow_tilt.x);
	sys->set_tilt(1, shadow_tilt.y);
	sys->set_tilt(2, shadow_tilt.z);
}

void Input::_virtual_analog(System *sys, int x, int y) {
	float jx = (float)x / (win_width / 2);
	float jy = (float)y / (win_height / 2);
	sys->set_joystick(0, jx);
	sys->set_joystick(1, jy);
}
