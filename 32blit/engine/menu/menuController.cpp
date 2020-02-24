#include "32blit.hpp"

MenuController* MenuController::instance = 0;

MenuController* MenuController::shared() {
    if (instance == 0){ 
        instance = new MenuController();
        }
    return instance;
}

MenuController::MenuController() {}

bool is_in_game () {
    #if EXTERNAL_LOAD_ADDRESS == 0x90000000
        return false;
    #else
        return true;
    #endif
}

bool MenuController::is_menu_visible() {
    return _currentMenu != nullptr;
}

void MenuController::set_current_menu (Menu *menu) {
    if (_currentMenu != menu) {
        _currentMenu = menu;
    } else {
        _currentMenu = nullptr;
    }
}

void MenuController::set_system_menu(Menu *menu) {
    _systemMenu = menu;
}

void MenuController::set_game_menu(Menu *menu) {
    _gameMenu = menu;
    _currentMenu = nullptr;
}

void MenuController::render(uint32_t time) {
    if (_currentMenu != nullptr) {
        _currentMenu->render(time);
    }
}
void MenuController::update () {

    static uint32_t last_buttons = 0;
    uint32_t changed_buttons = blit::buttons ^ last_buttons;

    if (blit::buttons & changed_buttons & blit::Button::MENU) {
        // game menu
        set_current_menu(_gameMenu);
    } else if (blit::buttons & changed_buttons & blit::Button::HOME) {
        // system menu
        set_current_menu(_systemMenu);
    }

    if (_currentMenu == nullptr) { return; }

    if (blit::buttons & changed_buttons & blit::Button::DPAD_UP) {
        _currentMenu->decrement_selection();
    } else if (blit::buttons & changed_buttons & blit::Button::DPAD_DOWN) {
        _currentMenu->increment_selection();
    } 

    if (blit::buttons & changed_buttons & blit::Button::DPAD_LEFT) {
        _currentMenu->pressed_left();
    } else if (blit::buttons & changed_buttons & blit::Button::DPAD_RIGHT) {
        _currentMenu->pressed_right();
    } else  if (blit::buttons & blit::Button::DPAD_LEFT) {
        _currentMenu->held_left();
    } else if (blit::buttons & blit::Button::DPAD_RIGHT) {
        _currentMenu->held_right();
    }

    if (pressed(blit::A)) {
        _currentMenu->selected();
    }

    if (pressed(blit::B)) {
        _currentMenu->back_pressed();
    }

    last_buttons = blit::buttons;
}

