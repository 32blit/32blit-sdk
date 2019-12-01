#if defined(_WIN32) && !defined(WIN32)
#define WIN32
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
#include "Renderer.hpp"

#ifndef NO_FFMPEG_CAPTURE
#include "VideoCapture.hpp"
#endif

#define WINDOW_TITLE "TinyDebug SDL"


int main(int argc, char *argv[]) {
	static bool running = true;

	SDL_Window* window = NULL;

	std::cout << "Hello World" << std::endl;

	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_GAMECONTROLLER) < 0) {
		fprintf(stderr, "could not initialize SDL2: %s\n", SDL_GetError());
		return 1;
	}

	window = SDL_CreateWindow(
		WINDOW_TITLE,
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		System::width*2, System::height*2,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
	);

	if (window == NULL) {
		fprintf(stderr, "could not create window: %s\n", SDL_GetError());
		return 1;
	}
	SDL_SetWindowMinimumSize(window, System::width, System::height);

	// Open all joysticks as game controllers
	for(int n=0; n<SDL_NumJoysticks(); n++) {
		SDL_GameControllerOpen(n);
	}

	System *sys = new System();
	Input *inp = new Input(window, sys);
	Renderer *ren = new Renderer(window, System::width, System::height);

#ifndef NO_FFMPEG_CAPTURE
	VideoCapture *cap = new VideoCapture(argv[0]);
	unsigned int last_record_startstop = 0;
#endif

	sys->run();

	SDL_Event event;

	while (running && SDL_WaitEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				running = false;
				break;

			case SDL_WINDOWEVENT:
				if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
					inp->resize(event.window.data1, event.window.data2);
					ren->resize(event.window.data1, event.window.data2);
				}
				break;

			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEBUTTONDOWN:
				inp->handle_mouse(event.button.button, event.type == SDL_MOUSEBUTTONDOWN, event.button.x, event.button.y);
				break;

			case SDL_MOUSEMOTION:
				if (event.motion.state & SDL_BUTTON_LMASK) {
					inp->handle_mouse(SDL_BUTTON_LEFT, event.motion.state & SDL_MOUSEBUTTONDOWN, event.motion.x, event.motion.y);
				}
				break;

			case SDL_KEYDOWN: // fall-though
			case SDL_KEYUP:
				if (!inp->handle_keyboard(event.key.keysym.sym, event.type == SDL_KEYDOWN)) {
					switch (event.key.keysym.sym) {
#ifndef NO_FFMPEG_CAPTURE
					case SDLK_r:
						if (event.type == SDL_KEYDOWN && SDL_GetTicks() - last_record_startstop > 1000) {
							if (cap->recording()) cap->stop();
							else cap->start();
							last_record_startstop = SDL_GetTicks();
						}
#endif
					}
				}
				break;

			case SDL_CONTROLLERBUTTONDOWN:
			case SDL_CONTROLLERBUTTONUP:
				inp->handle_controller_button(event.cbutton.button, event.type == SDL_CONTROLLERBUTTONDOWN);
				break;

			case SDL_CONTROLLERAXISMOTION:
				inp->handle_controller_motion(event.caxis.axis, event.caxis.value);
				break;

			case SDL_RENDER_TARGETS_RESET:
				std::cout << "Targets reset" << std::endl;
				break;

			case SDL_RENDER_DEVICE_RESET:
				std::cout << "Device reset" << std::endl;
				break;

			default:
				if(event.type == System::loop_event) {
					ren->update(sys);
					sys->notify_redraw();
					ren->present();
#ifndef NO_FFMPEG_CAPTURE
					if (cap->recording()) cap->capture(ren);
#endif
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

#ifndef NO_FFMPEG_CAPTURE
	if (cap->recording()) cap->stop();
	delete cap;
#endif

	sys->stop();
	delete sys;
	delete ren;

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
