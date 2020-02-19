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
void createMenuItems ();
std::vector<MenuItem> items;
std::vector<MenuItem> MenusDataSource::menuItems() {
    createMenuItems();
    return items;
};

void createMenuItems () {

    // BACKLIGHT

    items.push_back(
        MenuItem("Backlight", 
        [](float value) {
            // slider value has changed
            persist.backlight += value;
            persist.backlight = std::fmin(1.0f, std::fmax(0.0f, persist.backlight));
        },
        [](){
            return 75 * persist.backlight; // get value to update the UI with
        }, 
        -1.0f / 256.0f,  // left adjustment
        1.0f / 256.0f)   // right adjustment
    );

    // VOLUME

    items.push_back(
        MenuItem("Volume",
         [] (float value) {

            // slider value has changed
            persist.volume -= value;
            persist.volume = std::fmin(1.0f, std::fmax(0.0f, persist.volume));

            float volume_log_base = 2.0f;
            blit::volume = (uint16_t)(65535.0f * log(1.0f + (volume_log_base - 1.0f) * persist.volume) / log(volume_log_base));
        },
        [](){
            return 75 * persist.volume; // get current value to update the UI with
        },
        1.0f / 256.0f,  // left adjustment
        -1.0f / 256.0f)   // right adjustment
    );

    items.push_back(
        MenuItem("DFU Mode", "Press A",
        []() {

            // Select Action

            // Set the special magic word value that's checked by the assembly entry Point upon boot
            // This will trigger a jump into DFU mode upon reboot
            *((uint32_t *)0x2001FFFC) = 0xCAFEBABE; // Special Key to End-of-RAM

            SCB_CleanDCache();
            NVIC_SystemReset();
        })
    );

    
    std::string switchExecutionTitle = "Exit Game";
    #if EXTERNAL_LOAD_ADDRESS == 0x90000000
        switchExecutionTitle = "Launch Game";
    #endif

    items.push_back(
        MenuItem(switchExecutionTitle,
        "Press A",
        [](){
            blit::switch_execution();
        })
    );

}

MenusDataSource::MenusDataSource () { }