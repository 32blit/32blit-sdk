// #include "MenuItem.hpp"
// #include "persistence.h"

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

struct MenusDataSource {

    // __attribute__((section(".persist"))) Persist persist;

    std::vector<MenuItem> menuItems;

    void createMenuItems () {


    }

    MenusDataSource () {
        createMenuItems();
    }    
}