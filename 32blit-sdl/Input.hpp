#include <map>
#include <vector>
#include <algorithm>
class System;

struct GameController
{
  SDL_GameController* gc_id;
  bool can_rumble;
};

class Input {
	public:
		static std::map<int, int> keys;
		static std::map<int, int> buttons;

    static int find_key(int key);
		static int find_button(int button);

		explicit Input(System *system);
    ~Input();

		// Input handlers return true if they handled the event
		bool handle_mouse(int button, bool state, int x, int y);
		bool handle_keyboard(int key, bool state);
		bool handle_controller_button(int button, bool state);
		bool handle_controller_motion(int axis, int value);
    bool handle_controller_accel(float data[3]);

    // controller specific functions
    void handle_controller_added(Sint32 joystick_index);
    void handle_controller_removed(Sint32 joystick_index);

    void rumble_controllers(const float& volume);

private:
		System *target;

    std::vector<GameController> game_controllers;

    void _virtual_analog(int, int, int, int);
		void _virtual_tilt(int, int, int, int);

    void _add_controller(SDL_GameController* gc);
    void _remove_controller(SDL_GameController* gc);

		bool left_ctrl = false;
};
