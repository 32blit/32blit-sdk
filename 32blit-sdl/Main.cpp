#if defined(_WIN32) && !defined(WIN32)
#define WIN32
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#include "SDL.h"
#include <iostream>

#include "Input.hpp"
#include "System.hpp"
#include "Renderer.hpp"

#ifdef VIDEO_CAPTURE
#include "VideoCapture.hpp"
#endif

#ifndef WINDOW_TITLE
#define WINDOW_TITLE "TinyDebug SDL"
#endif

static bool running = true;

SDL_Window* window = NULL;

System *blit_system;
Input *blit_input;
Renderer *blit_renderer;

void handle_event(SDL_Event &event) {
	switch (event.type) {
		case SDL_QUIT:
			running = false;
			break;

		case SDL_WINDOWEVENT:
			if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
				blit_input->resize(event.window.data1, event.window.data2);
				blit_renderer->resize(event.window.data1, event.window.data2);
			}
			break;

		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEBUTTONDOWN:
			blit_input->handle_mouse(event.button.button, event.type == SDL_MOUSEBUTTONDOWN, event.button.x, event.button.y);
			break;

		case SDL_MOUSEMOTION:
			if (event.motion.state & SDL_BUTTON_LMASK) {
				blit_input->handle_mouse(SDL_BUTTON_LEFT, event.motion.state & SDL_MOUSEBUTTONDOWN, event.motion.x, event.motion.y);
			}
			break;

		case SDL_KEYDOWN: // fall-though
		case SDL_KEYUP:
			if (!blit_input->handle_keyboard(event.key.keysym.sym, event.type == SDL_KEYDOWN)) {
				switch (event.key.keysym.sym) {
#ifdef VIDEO_CAPTURE
				case SDLK_r:
					if (event.type == SDL_KEYDOWN && SDL_GetTicks() - last_record_startstop > 1000) {
						if (blit_capture->recording()) blit_capture->stop();
						else blit_capture->start();
						last_record_startstop = SDL_GetTicks();
					}
#endif
				}
			}
			break;

		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
			blit_input->handle_controller_button(event.cbutton.button, event.type == SDL_CONTROLLERBUTTONDOWN);
			break;

		case SDL_CONTROLLERAXISMOTION:
			blit_input->handle_controller_motion(event.caxis.axis, event.caxis.value);
			break;

		case SDL_RENDER_TARGETS_RESET:
			std::cout << "Targets reset" << std::endl;
			break;

		case SDL_RENDER_DEVICE_RESET:
			std::cout << "Device reset" << std::endl;
			break;

		default:
			if(event.type == System::loop_event) {
				blit_renderer->update(blit_system);
				blit_system->notify_redraw();
				blit_renderer->present();
#ifdef VIDEO_CAPTURE
				if (blit_capture->recording()) blit_capture->capture(blit_renderer);
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

#ifdef __EMSCRIPTEN__
void em_loop() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		handle_event(event);
	}

	blit_system->loop();
	blit_renderer->update(blit_system);
	blit_renderer->present();
}
#endif

#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char *argv[]) {

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

	blit_system = new System();
	blit_input = new Input(window, blit_system);
	blit_renderer = new Renderer(window, System::width, System::height);

#ifdef VIDEO_CAPTURE
	VideoCapture *blit_capture = new VideoCapture(argv[0]);
	unsigned int last_record_startstop = 0;
#endif

	blit_system->run();

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(em_loop, 0, 1);
#else
	SDL_Event event;

	while (running && SDL_WaitEvent(&event)) {
		handle_event(event);
	}
#endif

	if (running) {
		fprintf(stderr, "Main loop exited with error: %s\n", SDL_GetError());
		running = false; // ensure timer thread quits
	}

#ifdef VIDEO_CAPTURE
	if (blit_capture->recording()) blit_capture->stop();
	delete blit_capture;
#endif

	blit_system->stop();
	delete blit_system;
	delete blit_renderer;

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
