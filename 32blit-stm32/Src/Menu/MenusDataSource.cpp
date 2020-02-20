#include "MenusDataSource.hpp"
#include <vector>
#include "persistence.h"
#include <cmath>

#include "gpio.hpp"
#include "file.hpp"

#include "adc.h"
#include "tim.h"
#include "rng.h"
#include "spi.h"
#include "i2c.h"
#include "i2c-msa301.h"
#include "i2c-bq24295.h"
#include "fatfs.h"
#include "quadspi.h"
#include "usbd_core.h"

using namespace std;

__attribute__((section(".persist"))) Persist persist;
void createMenuItems ();
vector<MenuItem> items;
vector<MenuItem> MenusDataSource::menuItems() {
    createMenuItems();
    return items;
};

vector<MenuItem>aboutMenuItems () {
    vector<MenuItem> about;

    about.push_back(MenuItem("Version", "number"));
    about.push_back(MenuItem("Build number", "number"));
    about.push_back(MenuItem("Build time", "date"));

    return about;
}

void createMenuItems () {

    // BACKLIGHT

    items.push_back(
        MenuItem("Backlight", 
        [](float value) {
            // slider value has changed
            persist.backlight += value;
            persist.backlight = fmin(1.0f, fmax(0.0f, persist.backlight));
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
            persist.volume = fmin(1.0f, fmax(0.0f, persist.volume));

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


    items.push_back(
        MenuItem("Power off",
        "Press A",
        [](){
            bq24295_enable_shipping_mode(&hi2c4);
        })
    );
    
    string switchExecutionTitle = "Exit Game";
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

    items.push_back(MenuItem("About",aboutMenuItems()));
}



MenusDataSource::MenusDataSource () { }