#include <chrono>
#include <iostream>
#include <random>
#include "SDL.h"

#include "File.hpp"
#include "System.hpp"
#include "Input.hpp"
#include "32blit.hpp"
#include "UserCode.hpp"
#include "JPEG.hpp"
#include "Multiplayer.hpp"

#include "engine/api_private.hpp"

extern Input *blit_input;

// blit API
namespace blit {
  API real_api;
  API &api = real_api;
}

int System::width = System::max_width;
int System::height = System::max_height;

// blit framebuffer memory
static uint8_t framebuffer[System::max_width * System::max_height * 3];
static blit::Pen palette[256];

// blit debug callback
void blit_debug(const char *message) {
	std::cout << message;
}

// blit screenmode callback
blit::ScreenMode _mode = blit::ScreenMode::lores;
static blit::ScreenMode requested_mode = blit::ScreenMode::lores;
static blit::PixelFormat cur_format = blit::PixelFormat::RGB;
static blit::PixelFormat requested_format = blit::PixelFormat::RGB;

blit::SurfaceInfo cur_surf_info;

static void set_screen_palette(const blit::Pen *colours, int num_cols) {
	memcpy(palette, colours, num_cols * sizeof(blit::Pen));
}

static bool set_screen_mode_format(blit::ScreenMode new_mode, blit::SurfaceTemplate &new_surf_template) {
  new_surf_template.data = framebuffer;

  if(new_surf_template.format == (blit::PixelFormat)-1)
    new_surf_template.format = blit::PixelFormat::RGB;

  blit::Size default_bounds(System::width, System::height);

  if(new_surf_template.bounds.empty())
    new_surf_template.bounds = default_bounds;

  switch(new_mode) {
    case blit::ScreenMode::lores:
      new_surf_template.bounds /= 2;
      break;
    case blit::ScreenMode::hires:
    case blit::ScreenMode::hires_palette:
      break;
  }

  if(new_surf_template.bounds != default_bounds && new_surf_template.bounds != default_bounds / 2)
    return false;

  switch(new_surf_template.format) {
    case blit::PixelFormat::RGB:
    case blit::PixelFormat::RGB565:
      break;
    case blit::PixelFormat::P:
      new_surf_template.palette = palette;
      break;

    default:
      return false;
  }

  requested_mode = new_mode;
  requested_format = new_surf_template.format;

  return true;
}

blit::SurfaceInfo &set_screen_mode(blit::ScreenMode new_mode) {
  blit::SurfaceTemplate temp{nullptr, {0, 0}, new_mode == blit::ScreenMode::hires_palette ? blit::PixelFormat::P : blit::PixelFormat::RGB};

  // won't fail for the modes used here
  set_screen_mode_format(new_mode, temp);

  cur_surf_info.data = temp.data;
  cur_surf_info.bounds = temp.bounds;
  cur_surf_info.format = temp.format;
  cur_surf_info.palette = temp.palette;

  return cur_surf_info;
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

#ifndef __EMSCRIPTEN__
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
#endif

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

	blit::api.now = ::now;
	blit::api.random = ::blit_random;
	blit::api.debug = ::blit_debug;
	blit::api.set_screen_mode = ::set_screen_mode;
	blit::api.set_screen_palette = ::set_screen_palette;
  blit::api.set_screen_mode_format = ::set_screen_mode_format;
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

	blit::set_screen_mode(blit::lores);

#ifdef __EMSCRIPTEN__
	::init();
#else
	t_system_loop = SDL_CreateThread(system_loop_thread, "Loop", (void *)this);
	t_system_timer = SDL_CreateThread(system_timer_thread, "Timer", (void *)this);
#endif
}

int System::timer_thread() {
	// Signal the system loop every 10 msec.
	int dropped = 0;
	SDL_Event event = {};
	event.type = timer_event;

	while (SDL_SemWaitTimeout(s_timer_stop, 10)) {
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

void System::loop() {
  SDL_LockMutex(m_input);
  blit::buttons = shadow_buttons;
  blit::tilt.x = shadow_tilt[0];
  blit::tilt.y = shadow_tilt[1];
  blit::tilt.z = shadow_tilt[2];
  blit::joystick.x = shadow_joystick[0];
  blit::joystick.y = shadow_joystick[1];
  SDL_UnlockMutex(m_input);

  // only render at 50Hz (main loop runs at 100Hz)
  // however, the emscripten loop (usually) runs at the display refresh rate
  auto time_now = ::now();
#ifndef __EMSCRIPTEN__
  if(time_now - last_render_time >= 20)
#endif
  {
    blit::render(time_now);
    last_render_time = time_now;

    if(_mode != requested_mode || cur_format != requested_format) {
      _mode = requested_mode;
      cur_format = requested_format;
    }
  }

  blit::tick(::now());
  blit_input->rumble_controllers(blit::vibration);

  blit_multiplayer->update();
}

Uint32 System::mode() {
	return _mode;
}

Uint32 System::format() {
	return Uint32(cur_format);
}

void System::update_texture(SDL_Texture *texture) {
  bool is_lores = _mode == blit::ScreenMode::lores;

  SDL_Rect dest_rect{0, 0, is_lores ? width / 2 : width, is_lores ? height / 2 : height};
  auto stride = dest_rect.w * blit::pixel_format_stride[int(cur_format)];

  if(cur_format == blit::PixelFormat::P) {
    uint8_t col_fb[max_width * max_height * 3];

    auto in = framebuffer, out = col_fb;
    auto size = dest_rect.w * dest_rect.h;

    for(int i = 0; i < size; i++) {
      uint8_t index = *(in++);
      (*out++) = palette[index].r;
      (*out++) = palette[index].g;
      (*out++) = palette[index].b;
    }

    SDL_UpdateTexture(texture, &dest_rect, col_fb, stride * 3);
  } else
    SDL_UpdateTexture(texture, &dest_rect, framebuffer, stride);
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
