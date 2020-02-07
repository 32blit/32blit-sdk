#include <chrono>
#include <iostream>
#include <random>
#include "SDL.h"

#include "System.hpp"
#include "32blit.hpp"
#include "UserCode.hpp"


// blit framebuffer memory
uint8_t framebuffer[320 * 240 * 3];
blit::Surface __fb_hires((uint8_t *)framebuffer, blit::PixelFormat::RGB, blit::Size(320, 240));
blit::Surface __fb_lores((uint8_t *)framebuffer, blit::PixelFormat::RGB, blit::Size(160, 120));

// blit debug callback
void debug(std::string message) {
	std::cout << message << std::endl;
}

int blit_debugf(const char * psFormatString, ...)
{
	va_list args;
	va_start(args, psFormatString);
	int ret = vprintf(psFormatString, args);
	va_end(args);
	return ret;
}

// blit screenmode callback
blit::ScreenMode _mode = blit::ScreenMode::lores;
void set_screen_mode(blit::ScreenMode new_mode) {
	_mode = new_mode;
	if (_mode == blit::ScreenMode::hires) {
		blit::screen = __fb_hires;
	}
	else {
		blit::screen = __fb_lores;
	}
}

// blit timer callback
std::chrono::steady_clock::time_point start;
uint32_t now() {
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
	return (uint32_t)elapsed.count();
}

// blit random callback
#ifdef __MINGW32__
// Windows/MinGW doesn't support a non-deterministic source of randomness, so we fall back upon the age-old time seed once more
// Without this, random_device() will always return the same number and thus our mersenne twister will always produce the same sequence.
std::mt19937 random_generator(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
#else
std::random_device random_device;
std::mt19937 random_generator(random_device());
#endif
std::uniform_int_distribution<uint32_t> random_distribution;
uint32_t blit_random() {
	return random_distribution(random_generator);
}

// SDL events
const Uint32 System::timer_event = SDL_RegisterEvents(2);
const Uint32 System::loop_event = System::timer_event + 1;

// Thread bouncers
static int system_timer_thread(void *ptr) {
	// Bounce back in to the class.
	System *sys = (System *)ptr;
	return sys->timer_thread();
}

static int system_loop_thread(void *ptr) {
	// Bounce back in to the class.
	System *sys = (System *)ptr;
	return sys->update_thread();
}


System::System() {
	m_input = SDL_CreateMutex();
	s_timer_stop = SDL_CreateSemaphore(0);
	s_loop_update = SDL_CreateSemaphore(0);
	s_loop_redraw = SDL_CreateSemaphore(0);
	s_loop_ended = SDL_CreateSemaphore(0);
}

System::~System() {
	SDL_DestroyMutex(m_input);
	SDL_DestroySemaphore(s_timer_stop);
	SDL_DestroySemaphore(s_loop_update);
	SDL_DestroySemaphore(s_loop_redraw);
	SDL_DestroySemaphore(s_loop_ended);
}

void System::run() {
	running = true;

	start = std::chrono::steady_clock::now();

	blit::now = ::now;
	blit::random = ::blit_random;
	blit::debug = ::debug;
	blit::debugf = ::blit_debugf;
	blit::set_screen_mode = ::set_screen_mode;
	blit::update = ::update;
	blit::render = ::render;

	::set_screen_mode(blit::lores);

#ifdef __EMSCRIPTEN__
	::init();
#else
	t_system_loop = SDL_CreateThread(system_loop_thread, "Loop", (void *)this);
	t_system_timer = SDL_CreateThread(system_timer_thread, "Timer", (void *)this);
#endif
}

int System::timer_thread() {
	// Signal the system loop every 20 msec.
	int dropped = 0;
	SDL_Event event = {};
	event.type = timer_event;

	while (SDL_SemWaitTimeout(s_timer_stop, 20)) {
		if (SDL_SemValue(s_loop_update)) {
			dropped++;
			if(dropped > 100) {
				dropped = 100;
				event.user.code = 2;
				SDL_PushEvent(&event);
			} else {
				event.user.code = 1;
				SDL_PushEvent(&event);
			}
		} else {
			SDL_SemPost(s_loop_update);
			dropped = 0;
			event.user.code = 0;
			SDL_PushEvent(&event);
		}
	}
	return 0;
}

int System::update_thread() {
	// Run the blit user code once every time we are signalled.
	SDL_Event event = {};
	event.type = loop_event;

	::init(); // Run init here because the user can make it hang.

	while (true) {
		SDL_SemWait(s_loop_update);
		if(!running) break;
		loop();
		if(!running) break;
		SDL_PushEvent(&event);
		SDL_SemWait(s_loop_redraw);
	}
	SDL_SemPost(s_loop_ended);
	return 0;
}

void System::loop()
{
	SDL_LockMutex(m_input);
	blit::buttons = shadow_buttons;
	blit::tilt.x = shadow_tilt[0];
	blit::tilt.y = shadow_tilt[1];
	blit::tilt.z = shadow_tilt[2];
	blit::joystick.x = shadow_joystick[0];
	blit::joystick.y = shadow_joystick[1];
	SDL_UnlockMutex(m_input);
	blit::tick(::now());
}

Uint32 System::mode() {
	return _mode;
}

void System::update_texture(SDL_Texture *texture) {
	blit::render(::now());
	if (_mode == blit::ScreenMode::lores) {
		SDL_UpdateTexture(texture, NULL, __fb_lores.data, 160 * 3);
	}
	else
	{
		SDL_UpdateTexture(texture, NULL, __fb_hires.data, 320 * 3);
	}
}

void System::notify_redraw() {
	SDL_SemPost(s_loop_redraw);
}

void System::set_joystick(int axis, float value) {
	if (axis < 2) {
		SDL_LockMutex(m_input);
		shadow_joystick[axis] = value;
		SDL_UnlockMutex(m_input);
	}
}

void System::set_tilt(int axis, float value) {
	if (axis < 3) {
		SDL_LockMutex(m_input);
		shadow_tilt[axis] = value;
		SDL_UnlockMutex(m_input);
	}
}

void System::set_button(int button, bool state) {
	SDL_LockMutex(m_input);
	if (state) {
		shadow_buttons |= button;
	} else {
		shadow_buttons &= ~button;
	}
	SDL_UnlockMutex(m_input);
}

void System::stop() {
	int returnValue;
	running = false;

	if(SDL_SemWaitTimeout(s_loop_ended, 500)) {
		fprintf(stderr, "User code appears to have frozen. Detaching thread.\n");
		SDL_DetachThread(t_system_loop);
	} else {
		SDL_WaitThread(t_system_loop, &returnValue);
	}

	SDL_SemPost(s_timer_stop);
	SDL_WaitThread(t_system_timer, &returnValue);
}
