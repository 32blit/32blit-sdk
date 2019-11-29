#if defined(_WIN32) && !defined(WIN32)
#define WIN32
#endif
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
#include <map>

#include "engine/input.hpp" // Only needed for button definitions
#include "System.hpp"

#define WINDOW_TITLE "TinyDebug SDL"

#define SYSTEM_WIDTH 160
#define SYSTEM_HEIGHT 120

std::map<int, int> keys = {
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

std::map<int, int> gcbuttons = {
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
SDL_Renderer* renderer;

SDL_Texture* __fb_texture_RGB24;
SDL_Texture* __ltdc_texture_RGB565;

#ifndef NO_FFMPEG_CAPTURE
SDL_Texture* recorder_target;
uint8_t recorder_buffer[SYSTEM_WIDTH*2 * SYSTEM_HEIGHT*2 * 3];
#endif

typedef struct vector2d {
	double x;
	double y;
} vector2d;

vector2d mouse;
uint8_t mouse_button = 0;


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

void system_redraw(System *sys) {
	SDL_Texture *which = NULL;

	if (sys->mode() == SDL_PIXELFORMAT_RGB24) {
		which = __fb_texture_RGB24;
	}
	else
	{
		which = __ltdc_texture_RGB565;
	}

	sys->update_texture(which);
	sys->notify_redraw();

	SDL_SetRenderTarget(renderer, NULL);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, which, NULL, &renderer_dest);
	SDL_RenderPresent(renderer);

#ifndef NO_FFMPEG_CAPTURE
	if (recording) {
		SDL_SetRenderTarget(renderer, recorder_target);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, which, NULL, NULL);
		SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGB24, &recorder_buffer, 320*3);
		capture();
	}
#endif
}


void virtual_tilt(System *sys, int x, int y) {
	int z = 80;

	x = x - (current_width / 2);
	y = y - (current_height / 2);

	x /= current_pixel_size;
	y /= current_pixel_size;

	vec3 shadow_tilt = vec3(x, y, z);
	shadow_tilt.normalize();
	sys->set_tilt(0, shadow_tilt.x);
	sys->set_tilt(1, shadow_tilt.y);
	sys->set_tilt(2, shadow_tilt.z);
}

void virtual_analog(System *sys, int x, int y) {
	//printf("Joystick X/Y %d %d\n", x, y);

	float jx = (float)x / (current_width / 2);
	float jy = (float)y / (current_height / 2);

	//printf("Joystick X/Y %f %f\n", jx, jy);

	sys->set_joystick(0, jx);
	sys->set_joystick(1, jy);
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

	if (__fb_texture_RGB24) {
		SDL_DestroyTexture(__fb_texture_RGB24);
	}
	if (__ltdc_texture_RGB565) {
		SDL_DestroyTexture(__ltdc_texture_RGB565);
	}

	std::cout << "Textured destroyed" << std::endl;

	__fb_texture_RGB24 = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_TARGET, SYSTEM_WIDTH, SYSTEM_HEIGHT);
	__ltdc_texture_RGB565 = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_TARGET, SYSTEM_WIDTH * 2, SYSTEM_HEIGHT * 2);
#ifndef NO_FFMPEG_CAPTURE
	recorder_target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_TARGET, SYSTEM_WIDTH*2, SYSTEM_HEIGHT*2);
#endif

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

	resize_renderer(current_width, current_height);

	System *sys = new System();
	sys->run();

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
						virtual_tilt(sys, event.button.x, event.button.y);
					} else {
						int x = event.button.x;
						int y = event.button.y;
						x = x - (current_width / 2);
						y = y - (current_height / 2);
						virtual_analog(sys, x, y);
					}
				}
				break;

			case SDL_MOUSEBUTTONUP:
				if(event.button.button == SDL_BUTTON_LEFT){
					virtual_analog(sys, 0, 0);
				}
				break;

			case SDL_MOUSEMOTION:
				if(event.motion.state & SDL_MOUSEBUTTONDOWN){
					if(left_ctrl){
						virtual_tilt(sys, event.motion.x, event.motion.y);
					} else {
						int x = event.motion.x;
						int y = event.motion.y;
						x = x - (current_width / 2);
						y = y - (current_height / 2);
						virtual_analog(sys, x, y);
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
									filename << getTimeStamp().c_str();
									filename << ".mkv";
									open_stream(filename.str().c_str(), SYSTEM_WIDTH*2, SYSTEM_HEIGHT*2, AV_PIX_FMT_RGB24, recorder_buffer);
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
					} else {
						sys->set_button(iter->second, event.type == SDL_KEYDOWN);
					}
				}
				break;

			case SDL_CONTROLLERBUTTONDOWN:
			case SDL_CONTROLLERBUTTONUP:
				{
					auto iter = gcbuttons.find(event.cbutton.button);
					if (iter == gcbuttons.end()) break;
					sys->set_button(iter->second, event.type == SDL_CONTROLLERBUTTONDOWN);
				}
				break;

			case SDL_CONTROLLERAXISMOTION:
				switch(event.caxis.axis) {
					case SDL_CONTROLLER_AXIS_LEFTX:
						sys->set_joystick(0, event.caxis.value / 32768.0);
						break;
					case SDL_CONTROLLER_AXIS_LEFTY:
						sys->set_joystick(1, event.caxis.value / 32768.0);
						break;
					case SDL_CONTROLLER_AXIS_RIGHTX:
						sys->set_tilt(0, event.caxis.value / 32768.0);
						break;
					case SDL_CONTROLLER_AXIS_RIGHTY:
						sys->set_tilt(1, event.caxis.value / 32768.0);
						break;
				}
				break;

			case SDL_RENDER_TARGETS_RESET:
				std::cout << "Targets reset" << std::endl;
				break;

			case SDL_RENDER_DEVICE_RESET:
				std::cout << "Device reset" << std::endl;
				break;

			default:
				if(event.type == System::loop_event) {
					system_redraw(sys);
				} else if (event.type == System::timer_event) {
					switch(event.user.code) {
						case 0:
							SDL_SetWindowTitle(window, WINDOW_TITLE);
							break;
						case 1:
							SDL_SetWindowTitle(window, WINDOW_TITLE " [SLOW]");
							break;
						case 2:
							SDL_SetWindowTitle(window, WINDOW_TITLE " [FROZEN]");
							break;
					}
				}
				break;
		}
	}
	if (running) {
		fprintf(stderr, "Main loop exited with error: %s\n", SDL_GetError());
		running = false; // ensure timer thread quits
	}

	sys->stop();
	delete sys;


#ifndef NO_FFMPEG_CAPTURE
	if (recording) {
		recording = false;
		close_stream();
		std::cout << "Finished capture." << std::endl;
	}
	SDL_DestroyTexture(recorder_target);
#endif

	SDL_DestroyTexture(__ltdc_texture_RGB565);
	SDL_DestroyTexture(__fb_texture_RGB24);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
