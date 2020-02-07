#include <algorithm>
#include <cstdio>
#include "SDL.h"

#include "Renderer.hpp"
#include "System.hpp"

Renderer::Renderer(SDL_Window *window, int width, int height) : sys_width(width), sys_height(height) {
	//SDL_SetHint(SDL_HINT_RENDER_DRIVER, "openGL");
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		fprintf(stderr, "could not create renderer: %s\n", SDL_GetError());
	}

	current = fb_hires_texture;

	int w, h;
	SDL_GetWindowSize(window, &w, &h);
	resize(w, h);

	// Clear the window.
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);
}

Renderer::~Renderer() {
	SDL_DestroyTexture(fb_lores_texture);
	SDL_DestroyTexture(fb_hires_texture);
	SDL_DestroyRenderer(renderer);
}

void Renderer::set_mode(Mode new_mode) {
	mode = new_mode;
	if (mode == Stretch) {
		dest.x = 0; dest.y = 0;
		dest.w = win_width; dest.h = win_height;
	} else {
		float current_pixel_size = std::min((float)win_width / sys_width, (float)win_height / sys_height);
		if (mode == KeepPixels) current_pixel_size = (int)current_pixel_size;
		else if (mode == KeepPixelsLores) current_pixel_size = ((int)(2*current_pixel_size)) / 2.0;
		dest.w = sys_width * current_pixel_size;
		dest.h = sys_height * current_pixel_size;
		dest.x = (win_width - dest.w) / 2;
		dest.y = (win_height - dest.h) / 2;
	}
}

void Renderer::resize(int width, int height) {
	win_width = width;
	win_height = height;
	set_mode(mode);

	if (fb_lores_texture) {
		SDL_DestroyTexture(fb_lores_texture);
	}
	if (fb_hires_texture) {
		SDL_DestroyTexture(fb_hires_texture);
	}

	fb_lores_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, sys_width/2, sys_height/2);
	fb_hires_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, sys_width, sys_height);
}

void Renderer::update(System *sys) {
	if (sys->mode() == 0) {
		current = fb_lores_texture;
	} else {
		current = fb_hires_texture;
	}

	sys->update_texture(current);
}

void Renderer::_render(SDL_Texture *target, SDL_Rect *destination) {
	SDL_SetRenderTarget(renderer, target);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, current, NULL, destination);
}

void Renderer::present() {
	_render(NULL, &dest);
	SDL_RenderPresent(renderer);
}

void Renderer::read_pixels(int width, int height, Uint32 format, Uint8 *buffer) {
	SDL_Texture *record_target = SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_TARGET, width, height);
	_render(record_target, NULL);
	SDL_RenderReadPixels(renderer, NULL, format, buffer, width*SDL_BYTESPERPIXEL(format));
	SDL_DestroyTexture(record_target);
}

