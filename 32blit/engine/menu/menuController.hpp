#include "menu.hpp"

class MenuController {

    private:        

        Menu *_currentMenu;
        Menu *_systemMenu;
        Menu *_gameMenu;
        
        void set_current_menu (Menu *menu);

        static MenuController* instance;

        // Private constructor to prevent instancing.
        MenuController();

    public:

        bool is_menu_visible ();

        void render(uint32_t time);
        void update();

        void set_system_menu(Menu *menu);
        void set_game_menu(Menu *menu);

        static MenuController* shared();
};
