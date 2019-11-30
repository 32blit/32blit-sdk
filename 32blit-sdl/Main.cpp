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

#include "Input.hpp"
#include "System.hpp"

#include "engine/input.hpp"

#define WINDOW_TITLE "TinyDebug SDL"

#define SYSTEM_WIDTH 160
#define SYSTEM_HEIGHT 120

static bool running = true;
static bool recording = false;

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

	Input *inp = new Input(window);
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
					inp->resize(event.window.data1, event.window.data2);
				}
				break;

			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEBUTTONDOWN:
				inp->handle_mouse(sys, event.button.button, event.type == SDL_MOUSEBUTTONDOWN, event.button.x, event.button.y);
				break;

			case SDL_MOUSEMOTION:
				if (event.motion.state & SDL_BUTTON_LMASK) {
					inp->handle_mouse(sys, SDL_BUTTON_LEFT, event.motion.state & SDL_MOUSEBUTTONDOWN, event.motion.x, event.motion.y);
				}
				break;

			case SDL_KEYDOWN: // fall-though
			case SDL_KEYUP:
				if (!inp->handle_keyboard(sys, event.key.keysym.sym, event.type == SDL_KEYDOWN)) {
					switch (event.key.keysym.sym) {
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
				}
				break;

			case SDL_CONTROLLERBUTTONDOWN:
			case SDL_CONTROLLERBUTTONUP:
				inp->handle_controller_button(sys, event.cbutton.button, event.type == SDL_CONTROLLERBUTTONDOWN);
				break;

			case SDL_CONTROLLERAXISMOTION:
				inp->handle_controller_motion(sys, event.caxis.axis, event.caxis.value);
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
