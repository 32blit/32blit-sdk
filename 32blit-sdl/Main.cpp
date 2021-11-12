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

#include "contrib.hpp"

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

const char *launch_path = nullptr;

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
      blit_input->handle_controller_added(event.cdevice.which);
			break;

		case SDL_CONTROLLERDEVICEREMOVED:
      blit_input->handle_controller_removed(event.cdevice.which);
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
  int x, y;
  bool custom_window_position = false;

  std::cout << metadata_title << " " << metadata_version << std::endl;
  std::cout << "Powered by 32Blit SDL2 runtime - github.com/32blit/32blit-sdk" << std::endl << std::endl;

	auto mp_mode = Multiplayer::Mode::Auto;
	std::string mp_address = "localhost";

	for(int i = 1; i < argc; i++) {
		std::string arg_str(argv[i]);
		if(arg_str == "--connect" && i + 1 < argc) {
			mp_mode = Multiplayer::Mode::Connect;
			mp_address = std::string(argv[i + 1]);
			i++;
		}
		else if(arg_str == "--launch_path" && i + 1 < argc) {
			launch_path = argv[++i];
		}
		else if(arg_str == "--listen")
			mp_mode = Multiplayer::Mode::Listen;
		else if(arg_str == "--position") {
			if(SDL_sscanf(argv[i+1], "%d,%d", &x, &y) == 2) {
			    custom_window_position = true;
			}
		}
		else if(arg_str == "--credits") {
			std::cout << "32Blit was made possible by:" << std::endl;
			std::cout << std::endl;
			for(auto name : contributors) {
				if(name != nullptr) {
					std::cout << " * " << name << std::endl;
				}
			}
			std::cout << std::endl;
			std::cout << "Special thanks to:" << std::endl;
			std::cout << std::endl;
			for(auto name : special_thanks) {
				if(name != nullptr) {
					std::cout << " * " << name << std::endl;
				}
			}
			std::cout << std::endl;
			SDL_Quit();
			return 0;
		}
		else if(arg_str == "--info") {
			std::cout << metadata_description << std::endl << std::endl;
			std::cout << " Category: " << metadata_category << std::endl;
			std::cout << " Author:   " << metadata_author << std::endl;
			std::cout << " URL:      " << metadata_url << std::endl << std::endl;
			SDL_Quit();
			return 0;
		}
		else if(arg_str == "--help") {
			std::cout << "Usage: " << argv[0] << " <options>" << std::endl << std::endl;
			std::cout << " --connect <addr>     -- Connect to a listening game instance." << std::endl;
			std::cout << " --listen             -- Listen for incoming connections." << std::endl;
			std::cout << " --position x,y       -- Set window position." << std::endl;
			std::cout << " --launch_path <file> -- Emulates the file associations on the console." << std::endl;
			std::cout << " --credits            -- Print contributor credits and exit." << std::endl;
			std::cout << " --info               -- Print metadata info and exit." << std::endl << std::endl;
			SDL_DestroyWindow(window);
			SDL_Quit();
			return 0;
		}
	}

	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_GAMECONTROLLER|SDL_INIT_AUDIO) < 0) {
		std::cerr << "could not initialize SDL2: " << SDL_GetError() << std::endl;
		return 1;
	}

	window = SDL_CreateWindow(
		metadata_title,
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		System::width*2, System::height*2,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
	);

	if (window == nullptr) {
		std::cerr << "could not create window: " << SDL_GetError() << std::endl;
		return 1;
	}
	SDL_SetWindowMinimumSize(window, System::width, System::height);

	if(custom_window_position) {
		SDL_SetWindowPosition(window, x, y);
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
		std::cerr << "Main loop exited with error: " << SDL_GetError() << std::endl;
		running = false; // ensure timer thread quits
	}

#ifdef VIDEO_CAPTURE
	if (blit_capture->recording()) blit_capture->stop();
	delete blit_capture;
#endif

	blit_system->stop();
	delete blit_system;
  delete blit_input;
	delete blit_multiplayer;
	delete blit_renderer;

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
