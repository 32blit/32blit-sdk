#include "FirmwareMenusDataSource.hpp"
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

std::vector<MenuItem> FirmwareMenusDataSource::menuItems() {
    createSystemMenuItems();
    return _items;
}

static vector<MenuItem>aboutItems () {
    return {
        {"Version Number", "number"},
        {"Build Number","number"},
        {"Build Time","datetime"}
    };
}

void FirmwareMenusDataSource::createSystemMenuItems () {

    // BACKLIGHT

    _items.emplace_back(
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
    
    _items.emplace_back(
        MenuItem("Volume",
        [] (float value) {

            // slider value has changed
            persist.volume += value;
            persist.volume = fmin(1.0f, fmax(0.0f, persist.volume));

            float volume_log_base = 2.0f;
            blit::volume = (uint16_t)(65535.0f * log(1.0f + (volume_log_base - 1.0f) * persist.volume) / log(volume_log_base));
        },
        [](){
            return 75.0f * persist.volume; // get current value to update the UI with
        },
        -1.0f / 256.0f,   // left adjustment
        1.0f / 256.0f)  // right adjustment
    );

    _items.emplace_back(
        MenuItem("DFU Mode", 
        "Press A",
        []() {

            // Select Action

            // Set the special magic word value that's checked by the assembly entry Point upon boot
            // This will trigger a jump into DFU mode upon reboot
            *((uint32_t *)0x2001FFFC) = 0xCAFEBABE; // Special Key to End-of-RAM

            SCB_CleanDCache();
            NVIC_SystemReset();
        })
    );


    _items.emplace_back(
        MenuItem("Power off",
        "Press A",
        [](){
            bq24295_enable_shipping_mode(&hi2c4);
        })
    );

    _items.emplace_back(
        MenuItem("Launch Game",
        "Press A",
        [](){
            blit::switch_execution();
        })
    );

    _items.emplace_back(MenuItem("About",aboutItems()));
}

FirmwareMenusDataSource::FirmwareMenusDataSource () {}