#if defined(_WIN32) && !defined(WIN32)
#define WIN32
#endif
#include "32blit.hpp"
#include "UserCode.hpp"
#ifndef NO_FFMPEG_CAPTURE
#include "VideoCapture.hpp"
#endif
#include <SDL.h>
#include <random>
#include <cmath>
#include <iostream>
#include <chrono>
#include <ctime>
#include <string>
#include <sstream>

#define WINDOW_TITLE "TinyDebug SDL"

#define SYSTEM_WIDTH 160
#define SYSTEM_HEIGHT 120

std::map<int, int> keys = {
	// arrow keys
	{SDLK_DOWN,   button::DPAD_DOWN},
	{SDLK_UP,     button::DPAD_UP},
	{SDLK_LEFT,   button::DPAD_LEFT},
	{SDLK_RIGHT,  button::DPAD_RIGHT},

	// wasd
	{SDLK_w,       button::DPAD_UP},
	{SDLK_a,       button::DPAD_LEFT},
	{SDLK_s,       button::DPAD_DOWN},
	{SDLK_d,       button::DPAD_RIGHT},

	// action buttons
	{SDLK_z,       button::A},
	{SDLK_x,       button::B},
	{SDLK_c,       button::X},
	{SDLK_y,       button::Y},

	// system buttons
	{SDLK_1,       button::HOME},
	{SDLK_2,       button::MENU}
};

std::map<int, int> gcbuttons = {
	// dpad
	{SDL_CONTROLLER_BUTTON_DPAD_DOWN,   button::DPAD_DOWN},
	{SDL_CONTROLLER_BUTTON_DPAD_UP,     button::DPAD_UP},
	{SDL_CONTROLLER_BUTTON_DPAD_LEFT,   button::DPAD_LEFT},
	{SDL_CONTROLLER_BUTTON_DPAD_RIGHT,  button::DPAD_RIGHT},

	// action buttons
	{SDL_CONTROLLER_BUTTON_A,           button::A},
	{SDL_CONTROLLER_BUTTON_B,           button::B},
	{SDL_CONTROLLER_BUTTON_X,           button::X},
	{SDL_CONTROLLER_BUTTON_Y,           button::Y},

	// system buttons
	{SDL_CONTROLLER_BUTTON_BACK,        button::HOME},
	{SDL_CONTROLLER_BUTTON_START,       button::MENU}
};

Uint32 shadow_buttons = 0;
vec3 shadow_tilt = {0, 0, 0};
vec2 shadow_joystick = {0, 0};
SDL_mutex *shadow_mutex = SDL_CreateMutex();

SDL_Thread *t_system_timer;
SDL_Thread *t_system_loop;
SDL_sem *system_timer_stop = SDL_CreateSemaphore(0);
SDL_sem *system_loop_update = SDL_CreateSemaphore(0);
SDL_sem *system_loop_redraw = SDL_CreateSemaphore(0);
SDL_sem *system_loop_ended = SDL_CreateSemaphore(0);

Uint32 USEREVENT_REDRAW = SDL_RegisterEvents(4);
Uint32 USEREVENT_OKAY = USEREVENT_REDRAW + 1;
Uint32 USEREVENT_SLOW = USEREVENT_REDRAW + 2;
Uint32 USEREVENT_FROZEN = USEREVENT_REDRAW + 3;

static bool running = true;
static bool recording = false;
bool left_ctrl = false;

float current_pixel_size = 4;
int current_width = SYSTEM_WIDTH * current_pixel_size;
int current_height = SYSTEM_HEIGHT * current_pixel_size;
bool keep_aspect = true;
bool keep_pixels = true;
SDL_Rect renderer_dest = {0, 0, current_width, current_height};


static unsigned int last_record_startstop = 0;

uint32_t ticks_passed;
uint32_t ticks_last_update;

SDL_Window* window = NULL;
SDL_Surface* screenSurface = NULL;
SDL_Renderer* renderer;

SDL_Texture* __fb_texture_RGB24;
SDL_Texture* __ltdc_texture_RGB565;

rgb565 __ltdc_buffer[320 * 240 * 2];
surface __ltdc((uint8_t *)__ltdc_buffer, pixel_format::RGB565, size(320, 240));
surface __fb((uint8_t *)__ltdc_buffer + (320 * 240 * 2), pixel_format::RGB, size(160, 120));

typedef struct vector2d {
	double x;
	double y;
} vector2d;

vector2d mouse;
uint8_t mouse_button = 0;

std::chrono::steady_clock::time_point start;

void debug(std::string message) {
	//OutputDebugStringA(message.c_str());
	//OutputDebugStringA("\n");
}

blit::screen_mode mode = blit::screen_mode::lores;
void set_screen_mode(blit::screen_mode new_mode) {
	mode = new_mode;
	if (mode == blit::screen_mode::hires) {
		blit::fb = __ltdc;
	}
	else {
		blit::fb = __fb;
	}
}

uint32_t now() {
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
	return (uint32_t)elapsed.count();
}

inline std::tm localtime_xp(std::time_t timer)
{
	// Don't ignore unsafe warnings - https://stackoverflow.com/questions/38034033/c-localtime-this-function-or-variable-may-be-unsafe
	std::tm bt{};
#if defined(__unix__)
	localtime_r(&timer, &bt);
#elif defined(_MSC_VER)
	localtime_s(&bt, &timer);
#else
	static std::mutex mtx;
	std::lock_guard<std::mutex> lock(mtx);
	bt = *std::localtime(&timer);
#endif
	return bt;
}

std::string getTimeStamp() {
	auto bt = localtime_xp(std::time(0));
	std::string s(30, '\0');
	std::strftime(&s[0], s.size(), "%Y-%m-%d-%H-%M-%S", &bt);
	return s;
}

static int system_timer(void *ptr) {
	// Signal the system loop every 20 msec.
	int dropped = 0;
	SDL_Event event_okay = {.type = USEREVENT_OKAY};
	SDL_Event event_slow = {.type = USEREVENT_SLOW};
	SDL_Event event_frozen = {.type = USEREVENT_FROZEN};

	while (SDL_SemWaitTimeout(system_timer_stop, 20)) {
		if (SDL_SemValue(system_loop_update)) {
			dropped++;
			if(dropped > 100) {
				dropped = 100;
				SDL_PushEvent(&event_frozen);
			} else {
				SDL_PushEvent(&event_slow);
			}
		} else {
			SDL_SemPost(system_loop_update);
			dropped = 0;
			SDL_PushEvent(&event_okay);
		}
	}
	return 0;
}

static int system_loop(void *ptr) {
	// Run the blit user code once every time we are signalled.
	SDL_Event event = {.type = USEREVENT_REDRAW};
	while (true) {
		SDL_SemWait(system_loop_update);
		if(!running) break;
		SDL_LockMutex(shadow_mutex);
		blit::buttons = shadow_buttons;
		blit::tilt = shadow_tilt;
		blit::joystick = shadow_joystick;
		SDL_UnlockMutex(shadow_mutex);
		blit::tick(::now());
		if(!running) break;
		SDL_PushEvent(&event);
		SDL_SemWait(system_loop_redraw);
	}
	SDL_SemPost(system_loop_ended);
	return 0;
}

void system_redraw() {

	SDL_SetRenderTarget(renderer, NULL);
	SDL_RenderClear(renderer);

	if (mode == blit::screen_mode::lores) {
		SDL_UpdateTexture(__fb_texture_RGB24, NULL, (uint8_t *)__fb.data, 160 * 3);
		SDL_RenderCopy(renderer, __fb_texture_RGB24, NULL, &renderer_dest);
	}
	else
	{
		SDL_UpdateTexture(__ltdc_texture_RGB565, NULL, (uint8_t *)__ltdc.data, 320 * sizeof(uint16_t));
		SDL_RenderCopy(renderer, __ltdc_texture_RGB565, NULL, &renderer_dest);
	}

	SDL_RenderPresent(renderer);

#ifndef NO_FFMPEG_CAPTURE
	if (recording) {
		capture();
	}
#endif

	SDL_SemPost(system_loop_redraw);
}


void virtual_tilt(int x, int y) {
	int z = 80;

	x = x - (current_width / 2);
	y = y - (current_height / 2);

	x /= current_pixel_size;
	y /= current_pixel_size;

	SDL_LockMutex(shadow_mutex);
	shadow_tilt = vec3(x, y, z);
	shadow_tilt.normalize();
	SDL_UnlockMutex(shadow_mutex);
}

void virtual_analog(int x, int y) {
	if(x == 0 && y == 0){
		shadow_joystick = vec2(0, 0);
		return;
	}

	//printf("Joystick X/Y %d %d\n", x, y);

	float jx = (float)x / (current_width / 2);
	float jy = (float)y / (current_height / 2);

	//printf("Joystick X/Y %f %f\n", jx, jy);

	SDL_LockMutex(shadow_mutex);
	shadow_joystick = vec2(jx, jy);
	SDL_UnlockMutex(shadow_mutex);
}

void resize_renderer(int sizeX, int sizeY) {

	current_pixel_size = std::min((float)sizeX / SYSTEM_WIDTH, (float)sizeY / SYSTEM_HEIGHT);

	if (keep_pixels) current_pixel_size = (int)current_pixel_size;

	current_width = sizeX;
	current_height = sizeY;

	if (keep_pixels || keep_aspect) {
		int w = 160 * current_pixel_size;
		int h = 120 * current_pixel_size;
		int xoffs = (current_width - w) / 2;
		int yoffs = (current_height - h) / 2;
		renderer_dest.x = xoffs; renderer_dest.y = yoffs;
		renderer_dest.w = w; renderer_dest.h = h;
	} else {
		renderer_dest.x = 0; renderer_dest.y = 0;
		renderer_dest.w = current_width; renderer_dest.h = current_height;
	}

	std::cout << "Resized to: " << sizeX << "x" << sizeY << std::endl;

	SDL_SetRenderTarget(renderer, NULL);

	if (__fb_texture_RGB24) {
		SDL_DestroyTexture(__fb_texture_RGB24);
	}
	if (__ltdc_texture_RGB565) {
		SDL_DestroyTexture(__ltdc_texture_RGB565);
	}

	std::cout << "Textured destroyed" << std::endl;

	current_width = sizeX;
	current_height = sizeY;
	SDL_SetWindowSize(window, sizeX, sizeY);

	std::cout << "Window size set" << std::endl;

	__fb_texture_RGB24 = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_TARGET, SYSTEM_WIDTH, SYSTEM_HEIGHT);
	__ltdc_texture_RGB565 = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_TARGET, SYSTEM_WIDTH * 2, SYSTEM_HEIGHT * 2);

	std::cout << "Textured recreated" << std::endl;

	std::cout << "Device reset" << std::endl;
}

int main(int argc, char *argv[]) {
	std::cout << "Hello World" << std::endl;

	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_GAMECONTROLLER) < 0) {
		fprintf(stderr, "could not initialize SDL2: %s\n", SDL_GetError());
		return 1;
	}

	window = SDL_CreateWindow(
		WINDOW_TITLE,
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		current_width, current_height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
	);

	if (window == NULL) {
		fprintf(stderr, "could not create window: %s\n", SDL_GetError());
		return 1;
	}
	SDL_SetWindowMinimumSize(window, SYSTEM_WIDTH, SYSTEM_HEIGHT);

	// Open all joysticks as game controllers
	for(int n=0; n<SDL_NumJoysticks(); n++) {
		SDL_GameControllerOpen(n);
	}

	//SDL_SetHint(SDL_HINT_RENDER_DRIVER, "openGL");
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		fprintf(stderr, "could not create renderer: %s\n", SDL_GetError());
		return 1;
	}

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);

	//textureBuffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SYSTEM_WIDTH, SYSTEM_HEIGHT);

	__fb_texture_RGB24 = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_TARGET, SYSTEM_WIDTH, SYSTEM_HEIGHT);
	__ltdc_texture_RGB565 = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_TARGET, SYSTEM_WIDTH * 2, SYSTEM_HEIGHT * 2);

	start = std::chrono::steady_clock::now();

	uint32_t last_tick_ms = ::now();

	blit::now = ::now;
	blit::debug = ::debug;
	blit::set_screen_mode = ::set_screen_mode;
	blit::update = ::update;
	blit::render = ::render;
	//engine:init = ::init;

	::set_screen_mode(blit::lores);
	::init();

	printf("Init Done\n");

	t_system_loop = SDL_CreateThread(system_loop, "Loop", (void *)NULL);
	t_system_timer = SDL_CreateThread(system_timer, "Timer", (void *)NULL);

	SDL_Event event;

	while (running && SDL_WaitEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				running = false;
				break;

			case SDL_WINDOWEVENT:
				if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
					resize_renderer(event.window.data1, event.window.data2);
				}
				break;

			case SDL_MOUSEBUTTONDOWN:
				if(event.button.button == SDL_BUTTON_LEFT){
					if(left_ctrl){
						virtual_tilt(event.button.x, event.button.y);
					} else {
						int x = event.button.x;
						int y = event.button.y;
						x = x - (current_width / 2);
						y = y - (current_height / 2);
						virtual_analog(x, y);
					}
				}
				break;

			case SDL_MOUSEBUTTONUP:
				if(event.button.button == SDL_BUTTON_LEFT){
					virtual_analog(0, 0);
				}
				break;

			case SDL_MOUSEMOTION:
				if(event.motion.state & SDL_MOUSEBUTTONDOWN){
					if(left_ctrl){
						virtual_tilt(event.motion.x, event.motion.y);
					} else {
						int x = event.motion.x;
						int y = event.motion.y;
						x = x - (current_width / 2);
						y = y - (current_height / 2);
						virtual_analog(x, y);
					}
				}
				break;

			case SDL_KEYDOWN: // fall-though
			case SDL_KEYUP:
				{
					auto iter = keys.find(event.key.keysym.sym);
					if (iter == keys.end()) {
						switch (event.key.keysym.sym) {
						case SDLK_LCTRL:
							left_ctrl = event.type == SDL_KEYDOWN;
							break;
#ifndef NO_FFMPEG_CAPTURE
						case SDLK_r:
							if (event.type == SDL_KEYDOWN && SDL_GetTicks() - last_record_startstop > 1000) {
								if (!recording) {
									std::stringstream filename;
									filename << argv[0];
									filename << "-";
									filename << "capture-";
									if (mode == blit::screen_mode::lores) {
										filename << "160x120-";
										filename << getTimeStamp().c_str();
										filename << ".mp4";
										open_stream(filename.str().c_str(), 160, 120, AV_PIX_FMT_RGB24, (uint8_t *)__fb.data);
									} else {
										filename << "320x240-";
										filename << getTimeStamp().c_str();
										filename << ".mp4";
										open_stream(filename.str().c_str(), 320, 240, AV_PIX_FMT_RGB565, (uint8_t *)__ltdc.data);
									}
									recording = true;
									std::cout << "Starting capture to " << filename.str() << std::endl;
								}
								else
								{
									recording = false;
									close_stream();
									std::cout << "Finished capture." << std::endl;
								}
								last_record_startstop = SDL_GetTicks();
							}
#endif
						}
						break;
					}

					SDL_LockMutex(shadow_mutex);
					if (event.type == SDL_KEYDOWN) {
						shadow_buttons |= iter->second;
					}
					else
					{
						shadow_buttons &= ~iter->second;
					}
					SDL_UnlockMutex(shadow_mutex);
				}
				break;

			case SDL_CONTROLLERBUTTONDOWN:
			case SDL_CONTROLLERBUTTONUP:
				{
					auto iter = gcbuttons.find(event.cbutton.button);
					if (iter == gcbuttons.end()) break;
					SDL_LockMutex(shadow_mutex);
					if (event.type == SDL_CONTROLLERBUTTONDOWN) {
						shadow_buttons |= iter->second;
					}
					else
					{
						shadow_buttons &= ~iter->second;
					}
					SDL_UnlockMutex(shadow_mutex);
				}
				break;

			case SDL_CONTROLLERAXISMOTION:
				SDL_LockMutex(shadow_mutex);
				switch(event.caxis.axis) {
					case SDL_CONTROLLER_AXIS_LEFTX:
						shadow_joystick.x = event.caxis.value / 32768.0;
						break;
					case SDL_CONTROLLER_AXIS_LEFTY:
						shadow_joystick.y = event.caxis.value / 32768.0;
						break;
					case SDL_CONTROLLER_AXIS_RIGHTX:
						shadow_tilt.x = event.caxis.value / 32768.0;
						break;
					case SDL_CONTROLLER_AXIS_RIGHTY:
						shadow_tilt.y = event.caxis.value / 32768.0;
						break;
				}
				SDL_UnlockMutex(shadow_mutex);
				break;

			case SDL_RENDER_TARGETS_RESET:
				std::cout << "Targets reset" << std::endl;
				break;

			case SDL_RENDER_DEVICE_RESET:
				std::cout << "Device reset" << std::endl;
				break;

			default:
				if(event.type == USEREVENT_REDRAW) {
					system_redraw();
				} else if (event.type == USEREVENT_OKAY) {
					SDL_SetWindowTitle(window, WINDOW_TITLE);
				} else if (event.type == USEREVENT_SLOW) {
					SDL_SetWindowTitle(window, WINDOW_TITLE " [SLOW]");
				} else if (event.type == USEREVENT_FROZEN) {
					SDL_SetWindowTitle(window, WINDOW_TITLE " [FROZEN]");
				}
				break;
		}
	}
	if (running) {
		fprintf(stderr, "Main loop exited with error: %s\n", SDL_GetError());
		running = false; // ensure timer thread quits
	}

	int returnValue;

	if(SDL_SemWaitTimeout(system_loop_ended, 500)) {
		fprintf(stderr, "User code appears to have frozen. Detaching thread.\n");
		SDL_DetachThread(t_system_loop);
	} else {
		SDL_WaitThread(t_system_loop, &returnValue);
	}

	SDL_SemPost(system_timer_stop);
	SDL_WaitThread(t_system_timer, &returnValue);


#ifndef NO_FFMPEG_CAPTURE
	if (recording) {
		recording = false;
		close_stream();
		std::cout << "Finished capture." << std::endl;
	}
#endif

	SDL_DestroyTexture(__ltdc_texture_RGB565);
	SDL_DestroyTexture(__fb_texture_RGB24);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
