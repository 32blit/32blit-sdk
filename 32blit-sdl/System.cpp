#include <chrono>
#include <iostream>
#include <random>
#include "SDL.h"

#include "File.hpp"
#include "System.hpp"
#include "32blit.hpp"
#include "UserCode.hpp"
#include "JPEG.hpp"
#include "Multiplayer.hpp"

#include "engine/api_private.hpp"

// blit framebuffer memory
uint8_t framebuffer[320 * 240 * 3];
blit::Surface __fb_hires((uint8_t *)framebuffer, blit::PixelFormat::RGB, blit::Size(320, 240));
blit::Surface __fb_hires_pal((uint8_t *)framebuffer, blit::PixelFormat::P, blit::Size(320, 240));
blit::Surface __fb_lores((uint8_t *)framebuffer, blit::PixelFormat::RGB, blit::Size(160, 120));

static blit::Pen palette[256];

// blit debug callback
void blit_debug(const char *message) {
	std::cout << message;
}

// blit screenmode callback
blit::ScreenMode _mode = blit::ScreenMode::lores;
blit::Surface &set_screen_mode(blit::ScreenMode new_mode) {
	_mode = new_mode;
    switch(_mode) {
      case blit::ScreenMode::lores:
        blit::screen = __fb_lores;
        break;
      case blit::ScreenMode::hires:
        blit::screen = __fb_hires;
        break;
      case blit::ScreenMode::hires_palette:
        blit::screen = __fb_hires_pal;
        break;
    }

	return blit::screen;
}

static void set_screen_palette(const blit::Pen *colours, int num_cols) {
	memcpy(palette, colours, num_cols * sizeof(blit::Pen));
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


// us timer used by profiler

void enable_us_timer()
{
	// Enable/initialise timer
}

uint32_t get_us_timer()
{
	// get current time in us
	uint64_t ticksPerUs = SDL_GetPerformanceFrequency() / 1000000;
	return SDL_GetPerformanceCounter() / ticksPerUs;
}

uint32_t get_max_us_timer()
{
	// largest us value timer can produce for wrapping
	return UINT32_MAX;
}

/* Added a command line ability to specify a launch_path parameter. */
extern const char *launch_path;
static const char *get_launch_path() {
  return launch_path;
}

static blit::GameMetadata get_metadata() {
  blit::GameMetadata ret;

  ret.title = metadata_title;
  ret.author = metadata_author;
  ret.description = metadata_description;
  ret.version = metadata_version;
  ret.url = metadata_url;
  ret.category = metadata_category;

  return ret;
}

extern Multiplayer *blit_multiplayer;
bool blit_is_multiplayer_connected() {
	return blit_multiplayer->is_connected();
}

void blit_set_multiplayer_enabled(bool enabled) {
	blit_multiplayer->set_enabled(enabled);
}

void blit_send_message(const uint8_t *data, uint16_t length) {
	blit_multiplayer->send_message(data, length);
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

	__fb_hires_pal.palette = palette;
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

	blit::api.now = ::now;
	blit::api.random = ::blit_random;
	blit::api.debug = ::blit_debug;
	blit::api.set_screen_mode = ::set_screen_mode;
	blit::api.set_screen_palette = ::set_screen_palette;
	blit::update = ::update;
	blit::render = ::render;

	setup_base_path();

	blit::api.open_file = ::open_file;
	blit::api.read_file = ::read_file;
	blit::api.write_file = ::write_file;
	blit::api.close_file = ::close_file;
	blit::api.get_file_length = ::get_file_length;
	blit::api.list_files = ::list_files;
	blit::api.file_exists = ::file_exists;
	blit::api.directory_exists = ::directory_exists;
	blit::api.create_directory = ::create_directory;
	blit::api.rename_file = ::rename_file;
	blit::api.remove_file = ::remove_file;
	blit::api.get_save_path = ::get_save_path;
	blit::api.is_storage_available = ::is_storage_available;

	blit::api.enable_us_timer = ::enable_us_timer;
	blit::api.get_us_timer = ::get_us_timer;
	blit::api.get_max_us_timer = ::get_max_us_timer;

	blit::api.decode_jpeg_buffer = blit_decode_jpeg_buffer;
	blit::api.decode_jpeg_file = blit_decode_jpeg_file;

  blit::api.get_launch_path = ::get_launch_path;

	blit::api.is_multiplayer_connected = blit_is_multiplayer_connected;
	blit::api.set_multiplayer_enabled = blit_set_multiplayer_enabled;
	blit::api.send_message = blit_send_message;

  blit::api.get_metadata = ::get_metadata;

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
	blit_multiplayer->update();
}

Uint32 System::mode() {
	return _mode;
}

void System::update_texture(SDL_Texture *texture) {
	blit::render(::now());
	if (_mode == blit::ScreenMode::lores) {
		SDL_UpdateTexture(texture, nullptr, __fb_lores.data, 160 * 3);
	}
	else if(_mode == blit::ScreenMode::hires) {
		SDL_UpdateTexture(texture, nullptr, __fb_hires.data, 320 * 3);
	} else {
		uint8_t col_fb[320 * 240 * 3];

		auto in = __fb_hires_pal.data, out = col_fb;

		for(int i = 0; i < 320 * 240; i++) {
			uint8_t index = *(in++);
			(*out++) = palette[index].r;
			(*out++) = palette[index].g;
			(*out++) = palette[index].b;
		}

		SDL_UpdateTexture(texture, nullptr, col_fb, 320 * 3);
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
		std::cerr << "User code appears to have frozen. Detaching thread." << std::endl;
		SDL_DetachThread(t_system_loop);
	} else {
		SDL_WaitThread(t_system_loop, &returnValue);
	}

	SDL_SemPost(s_timer_stop);
	SDL_WaitThread(t_system_timer, &returnValue);
}
