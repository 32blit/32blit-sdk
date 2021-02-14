#if defined(_WIN32) && !defined(WIN32)
#define WIN32
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#include "SDL.h"
#include <iostream>

#include "Input.hpp"
#include "Multiplayer.hpp"
#include "System.hpp"
#include "Renderer.hpp"
#include "Audio.hpp"
#include "UserCode.hpp"

#ifdef VIDEO_CAPTURE
#include "VideoCapture.hpp"
#endif

static bool running = true;

SDL_Window* window = nullptr;

System *blit_system;
Input *blit_input;
Multiplayer *blit_multiplayer;
Renderer *blit_renderer;
Audio *blit_audio;

#ifdef VIDEO_CAPTURE
VideoCapture *blit_capture;
unsigned int last_record_startstop = 0;
#endif

void handle_event(SDL_Event &event) {
	switch (event.type) {
		case SDL_QUIT:
			running = false;
			break;

		case SDL_WINDOWEVENT:
			if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
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
#ifdef VIDEO_CAPTURE
				switch (event.key.keysym.sym) {
				case SDLK_r:
					if (event.type == SDL_KEYDOWN && SDL_GetTicks() - last_record_startstop > 1000) {
						if (blit_capture->recording()) blit_capture->stop();
						else blit_capture->start();
						last_record_startstop = SDL_GetTicks();
					}
				}
#endif
			}
			break;

		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
			blit_input->handle_controller_button(event.cbutton.button, event.type == SDL_CONTROLLERBUTTONDOWN);
			break;

		case SDL_CONTROLLERAXISMOTION:
			blit_input->handle_controller_motion(event.caxis.axis, event.caxis.value);
			break;

		case SDL_CONTROLLERDEVICEADDED:
			SDL_GameControllerOpen(event.cdevice.which);
			break;

		case SDL_CONTROLLERDEVICEREMOVED:
			SDL_GameControllerClose(SDL_GameControllerFromInstanceID(event.cdevice.which));
			break;

		case SDL_RENDER_TARGETS_RESET:
			std::cout << "Targets reset" << std::endl;
			break;

		case SDL_RENDER_DEVICE_RESET:
			std::cout << "Device reset" << std::endl;
			break;

		default:
			if(event.type == System::loop_event) {
				blit_multiplayer->update();
				blit_renderer->update(blit_system);
				blit_system->notify_redraw();
				blit_renderer->present();
#ifdef VIDEO_CAPTURE
				if (blit_capture->recording()) blit_capture->capture(blit_renderer);
#endif
			} else if (event.type == System::timer_event) {
				switch(event.user.code) {
					case 0:
						SDL_SetWindowTitle(window, metadata_title);
						break;
					case 1:
						SDL_SetWindowTitle(window, (std::string(metadata_title) + " [SLOW]").c_str());
						break;
					case 2:
						SDL_SetWindowTitle(window, (std::string(metadata_title) + " [FROZEN]").c_str());
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

	blit_multiplayer->update();
	blit_system->loop();
	blit_renderer->update(blit_system);
	blit_renderer->present();
}
#endif

int main(int argc, char *argv[]) {

  std::cout << "32Blit SDL2 runtime" << std::endl;
  std::cout << "(c) Pimoroni et.al. 2019-2020" << std::endl;

	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_GAMECONTROLLER|SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "could not initialize SDL2: %s\n", SDL_GetError());
		return 1;
	}

	window = SDL_CreateWindow(
		metadata_title,
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		System::width*2, System::height*2,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
	);

	if (window == nullptr) {
		fprintf(stderr, "could not create window: %s\n", SDL_GetError());
		return 1;
	}
	SDL_SetWindowMinimumSize(window, System::width, System::height);

	// Open all joysticks as game controllers
	for(int n=0; n<SDL_NumJoysticks(); n++) {
		SDL_GameControllerOpen(n);
	}

	auto mp_mode = Multiplayer::Mode::Auto;
	std::string mp_address = "localhost";

	for(int i = 1; i < argc; i++) {
		std::string arg_str(argv[i]);
		if(arg_str == "--connect" && i + 1 < argc) {
			mp_mode = Multiplayer::Mode::Connect;
			mp_address = std::string(argv[i + 1]);
			i++;
		}
		else if(arg_str == "--listen")
			mp_mode = Multiplayer::Mode::Listen;
		else if(arg_str == "--position") {
			int x, y;
			if(SDL_sscanf(argv[i+1], "%d,%d", &x, &y) == 2) {
			    SDL_SetWindowPosition(window, x, y);
			}
		}
	}

	blit_system = new System();
	blit_input = new Input(blit_system);
	blit_multiplayer = new Multiplayer(mp_mode, mp_address);
	blit_renderer = new Renderer(window, System::width, System::height);
	blit_audio = new Audio();

#ifdef VIDEO_CAPTURE
	blit_capture = new VideoCapture(argv[0]);
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
	delete blit_multiplayer;
	delete blit_renderer;

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
