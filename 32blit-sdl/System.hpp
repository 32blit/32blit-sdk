class System {
	public:
		static const Uint32 timer_event;
		static const Uint32 loop_event;

		static const int width = 320;
		static const int height = 240;

		System();
		~System();

		void run();
		void stop();

		int update_thread();
		int timer_thread();

		void loop();

		Uint32 mode();
		void update_texture(SDL_Texture *);
		void notify_redraw();

		void set_joystick(int axis, float value);
		void set_tilt(int axis, float value);
		void set_button(int button, bool state);

	private:

		SDL_Thread *t_system_timer = NULL;
		SDL_Thread *t_system_loop = NULL;

		SDL_mutex *m_input = NULL;

		SDL_sem *s_timer_stop = NULL;
		SDL_sem *s_loop_update = NULL;
		SDL_sem *s_loop_redraw = NULL;
		SDL_sem *s_loop_ended = NULL;

		bool running = false;

		// shadow input
		Uint32 shadow_buttons = 0;
		float shadow_joystick[2] = {0, 0};
		float shadow_tilt[3] = {0, 0, 0};
};
