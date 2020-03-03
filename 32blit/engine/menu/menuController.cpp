#include "../../32blit.hpp"

MenuController* MenuController::instance = 0;

MenuController* MenuController::shared() {
    if (instance == 0){ 
        instance = new MenuController();
        }
    return instance;
}

MenuController::MenuController() {}

bool MenuController::is_menu_visible() {
    return _current_menu != nullptr;
}

void MenuController::set_current_menu (Menu *menu) {

    if (menu != nullptr && _current_menu != menu) {
    
        _current_menu = menu;
    } else if (_current_menu != nullptr) {
        _current_menu->menu_hiding(); // reset any confirmations
        _current_menu = nullptr;
    }
}

void MenuController::set_system_menu(Menu *menu) {
    _system_menu = menu;
}

void MenuController::set_game_menu(Menu *menu) {
    _game_menu = menu;
    _current_menu = nullptr; // clear down any other menu so it's not still visible when game launches
}

void MenuController::render(uint32_t time) {
    if (_current_menu != nullptr) {
        _current_menu->render(time);
    }
}

void MenuController::update () {

    static uint32_t last_buttons = 0;
    uint32_t changed_buttons = blit::buttons ^ last_buttons;

    if (blit::buttons & changed_buttons & blit::Button::MENU) {
        // game menu
        set_current_menu(_game_menu);
    } else if (blit::buttons & changed_buttons & blit::Button::HOME) {
        // system menu
        set_current_menu(_system_menu);
    }

    if (_current_menu == nullptr) { return; }

    if (blit::buttons & changed_buttons & blit::Button::DPAD_UP) {
        _current_menu->decrement_selection();
    } else if (blit::buttons & changed_buttons & blit::Button::DPAD_DOWN) {
        _current_menu->increment_selection();
    } 

    if (blit::buttons & changed_buttons & blit::Button::DPAD_LEFT) {
        _current_menu->pressed_left();
    } else if (blit::buttons & changed_buttons & blit::Button::DPAD_RIGHT) {
        _current_menu->pressed_right();
    } else  if (blit::buttons & blit::Button::DPAD_LEFT) {
        _current_menu->held_left();
    } else if (blit::buttons & blit::Button::DPAD_RIGHT) {
        _current_menu->held_right();
    }

    if (blit::buttons & changed_buttons & blit::Button::A) {
        _current_menu->selected();
    }

    if (blit::buttons & changed_buttons & blit::Button::B) {
        _current_menu->back_pressed();
    }

    last_buttons = blit::buttons;
}

