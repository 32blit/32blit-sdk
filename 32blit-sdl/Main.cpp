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

SDL_Thread *t_system_timer;

static bool running = true;
static bool recording = false;
bool left_ctrl = false;

int current_pixel_size = 4;
int current_width = SYSTEM_WIDTH * current_pixel_size;
int current_height = SYSTEM_HEIGHT * current_pixel_size;

static unsigned int need_resize = 0;
static unsigned int last_resize;

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

void system_tick() {
	// it's time for updating the game
	uint32_t ticks_now = SDL_GetTicks();
	ticks_passed = ticks_now - ticks_last_update;

	//std::cout << "Tick! " << ticks_now << std::endl;

	// TICK HERE
	blit::tick(::now());

	ticks_last_update = ticks_now;

	SDL_Rect dest = {};
	dest.x = 0; dest.y = 0; dest.w = current_width; dest.h = current_height;
	SDL_SetRenderTarget(renderer, NULL);
	SDL_RenderClear(renderer);


	if (mode == blit::screen_mode::lores) {
		SDL_UpdateTexture(__fb_texture_RGB24, NULL, (uint8_t *)__fb.data, 160 * 3);
		SDL_RenderCopy(renderer, __fb_texture_RGB24, NULL, &dest);
	}
	else
	{
		SDL_UpdateTexture(__ltdc_texture_RGB565, NULL, (uint8_t *)__ltdc.data, 320 * sizeof(uint16_t));
		SDL_RenderCopy(renderer, __ltdc_texture_RGB565, NULL, &dest);
	}

	SDL_RenderPresent(renderer);

#ifndef NO_FFMPEG_CAPTURE
	if (recording) {
		capture();
	}
#endif
}

static int system_timer(void *ptr) {
	while (running) {
		SDL_Event event;
		event.type = SDL_USEREVENT;
		SDL_PushEvent(&event);
		SDL_Delay(20);
	}
	return 0;
}

void virtual_tilt(int x, int y) {
	int z = 80;

	x = x - (current_width / 2);
	y = y - (current_height / 2);

	x /= current_pixel_size;
	y /= current_pixel_size;

	blit::tilt = vec3(x, y, z);
	blit::tilt.normalize();
}

void virtual_analog(int x, int y) {
	if(x == 0 && y == 0){
		blit::joystick = vec2(0, 0);
		return;
	}

	//printf("Joystick X/Y %d %d\n", x, y);

	float jx = (float)x / (current_width / 2);
	float jy = (float)y / (current_height / 2);

	//printf("Joystick X/Y %f %f\n", jx, jy);

	blit::joystick = vec2(jx, jy);
}

void resize_renderer() {

	int sizeX, sizeY;

	SDL_GetWindowSize(window, &sizeX, &sizeY);

	sizeX = (int)round((double)sizeX / SYSTEM_WIDTH) * SYSTEM_WIDTH;
	sizeY = (int)round((double)sizeY / SYSTEM_HEIGHT) * SYSTEM_HEIGHT;

	current_pixel_size = sizeX / SYSTEM_WIDTH;

	if (sizeX / SYSTEM_WIDTH != sizeY / SYSTEM_HEIGHT) {
		sizeY = (sizeX / SYSTEM_WIDTH) * SYSTEM_HEIGHT;
	}

	std::cout << "Resized to: " << sizeX << "x" << sizeY << std::endl;

	if (__fb_texture_RGB24) {
		SDL_SetRenderTarget(renderer, NULL);
		SDL_DestroyTexture(__fb_texture_RGB24);
	}
	if (__ltdc_texture_RGB565) {
		SDL_SetRenderTarget(renderer, NULL);
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

	SDL_SetRenderTarget(renderer, NULL);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);

	std::cout << "Device reset" << std::endl;

}

int main(int argc, char *argv[]) {
	std::cout << "Hello World" << std::endl;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "could not initialize SDL2: %s\n", SDL_GetError());
		return 1;
	}

	window = SDL_CreateWindow(
		"TinyDebug SDL",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		current_width, current_height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
	);

	if (window == NULL) {
		fprintf(stderr, "could not create window: %s\n", SDL_GetError());
		return 1;
	}
	SDL_SetWindowMinimumSize(window, SYSTEM_WIDTH, SYSTEM_HEIGHT);

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

	t_system_timer = SDL_CreateThread(system_timer, "Run", (void *)NULL);

	while (running) {
		SDL_Event event;

		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = false;
				break;
			}
			if (event.type == SDL_WINDOWEVENT) {
				if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
					need_resize = 1;
					last_resize = SDL_GetTicks();
				}
				break;
			}
			if (event.type == SDL_MOUSEBUTTONDOWN) {
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
			}
			if (event.type == SDL_MOUSEBUTTONUP) {
				if(event.button.button == SDL_BUTTON_LEFT){
					virtual_analog(0, 0);
				}
			}
			if (event.type == SDL_MOUSEMOTION) {
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
			}
			if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
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

				if (event.type == SDL_KEYDOWN) {
					blit::buttons |= iter->second;
				}
				else
				{
					blit::buttons &= ~iter->second;
				}
			}
			if (event.type == SDL_RENDER_TARGETS_RESET) {
				std::cout << "Targets reset" << std::endl;
			}
			if (event.type == SDL_RENDER_DEVICE_RESET) {
				std::cout << "Device reset" << std::endl;
			}
			if (event.type == SDL_USEREVENT) {
				system_tick();
			}
		}

		if (need_resize && (SDL_GetTicks() > last_resize + 50)) {

			resize_renderer();

			need_resize = 0;
		}

	}

	int returnValue;
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
