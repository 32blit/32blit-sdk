#include <map>

class Input {
	public:
		static std::map<int, int> keys;
		static std::map<int, int> buttons;

		static int find_key(int key);
		static int find_button(int button);
};
