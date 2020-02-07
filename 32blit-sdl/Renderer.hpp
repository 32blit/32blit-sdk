class System;

class Renderer {
	public:
		Renderer(SDL_Window *window, int width, int height);
		~Renderer();

		enum Mode {Stretch, KeepAspect, KeepPixels, KeepPixelsLores};

		void resize(int width, int height);
		void update(System *sys);
		void present();
		void set_mode(Mode mode);
		void read_pixels(int width, int height, Uint32 format, Uint8 *buffer);

	private:
		void _render(SDL_Texture *target, SDL_Rect *destination);

		int sys_width, sys_height;
		int win_width, win_height;

		Mode mode = KeepPixels;
		SDL_Renderer *renderer = NULL;
		SDL_Rect dest = {0, 0, 0, 0};

		SDL_Texture *fb_lores_texture = NULL;
		SDL_Texture *fb_hires_texture = NULL;
		SDL_Texture *current = NULL;
};
