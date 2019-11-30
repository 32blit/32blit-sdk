#include <SDL.h>

#include "Input.hpp"
#include "engine/input.hpp"

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
