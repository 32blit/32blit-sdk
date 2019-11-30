struct SDL_Window;
struct SDL_Texture;

class System;

class Renderer {
	public:
		Renderer(SDL_Window *window, int width, int height);
		~Renderer();

		enum Mode {Stretch, KeepAspect, KeepPixels, KeepPixelsx2};

		void resize(int width, int height);
		void update(System *sys);
		void render(SDL_Texture *target);
		void render();
		void present();
		void set_mode(Mode mode);

	private:
		int sys_width, sys_height;
		int win_width, win_height;

		Mode mode = KeepPixels;
		SDL_Renderer *renderer = NULL;
		SDL_Rect dest = {0, 0, 0, 0};

		SDL_Texture *fb_texture_RGB24 = NULL;
		SDL_Texture *ltdc_texture_RGB565 = NULL;
		SDL_Texture *current = NULL;
};
