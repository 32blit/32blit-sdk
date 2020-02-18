#include "MenusDataSource.hpp"
#include <vector>
#include "persistence.h"
#include <cmath>

/*std::string menu_name (MenuItem item) {
  switch (item) {
    case BACKLIGHT: return "Backlight";
    case VOLUME: return "Volume";
    case DFU: return "DFU Mode";
    case SHIPPING: return "Power Off";
#if EXTERNAL_LOAD_ADDRESS == 0x90000000
    case SWITCH_EXE: return "Launch Game";
#else
    case SWITCH_EXE: return "Exit Game";
#endif
    case LAST_COUNT: return "";
  };
  return "";
}*/

__attribute__((section(".persist"))) Persist persist;

std::vector<MenuItem> items;
std::vector<MenuItem> MenusDataSource::menuItems() {
    return items;
};

void createMenuItems () {

    items.push_back(
        MenuItem("Backlight", [](float value) {
            persist.backlight += value;
            persist.backlight = std::fmin(1.0f, std::fmax(0.0f, persist.backlight));
        },
        [](){
            return 75 * persist.backlight; 
        }, 
        -1.0f / 256.0f,  // left adjustment
        1.0f / 256.0f)   // right adjustment
    );

    items.push_back(
        MenuItem("Vibrate", "Press A")
    );

    items.push_back(
        MenuItem("Blah", "Press A")
    );

}

MenusDataSource::MenusDataSource () { createMenuItems (); }  