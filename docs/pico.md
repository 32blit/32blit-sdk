# 32blit + Pico SDK <!-- omit in toc -->

Mashing two SDKs together to run 32blit games on even less powerful hardware!

- [Prerequisites](#prerequisites)
- [Building](#building)
  - [Your Projects](#your-projects)
  - [Extra configuration](#extra-configuration)
- [32blit API Support](#32blit-api-support)
  - [Board-specific details](#board-specific-details)

## Prerequisites

This requires a working Pico SDK setup ([Getting started with Raspberry Pi Pico](https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf)), the 32blit tools and a copy of the 32blit SDK (this repository).

## Building

To build for a pico-based device you need to specify both the 32blit and Pico SDK paths, and the device/board you are building for.

To build the examples for a PicoSystem:

```
mkdir build.pico
cd build.pico
cmake .. -D32BLIT_DIR=.. -DPICO_SDK_PATH=/path/to/pico-sdk -DPICO_BOARD=pimoroni_picosystem
```

You can use Pico SDK's fetch-from-git feature and build like so:

```
mkdir build.pico
cd build.pico
cmake .. -D32BLIT_DIR=`pwd`/../ -DPICO_SDK_FETCH_FROM_GIT=true -DPICO_EXTRAS_FETCH_FROM_GIT=true -DPICO_BOARD=pimoroni_picosystem
```

And then run `make` as usual.

### Your Projects

Building a boilerplate project is similar, but first you need to import the Pico SDK before the `project` line in your CMakeLists.txt.

```cmake
cmake_minimum_required(VERSION 3.9)

# this is a wrapper for the pico sdk import files, which are only included if PICO_BOARD is set
include(${32BLIT_DIR}/32blit-pico/sdk_import.cmake OPTIONAL)

project(my-amazing-gane)
...
```

Then configure with something like:
```
cmake .. -D32BLIT_DIR=/path/to/32blit-sdk -DPICO_SDK_PATH=/path/to/pico-sdk -DPICO_BOARD=pimoroni_picosystem
```

### Extra configuration

If you're not using `hires` mode and need some more RAM, it can be disabled:
```cmake
...

blit_executable(amazing-lores-game ...)

...

target_compile_definitions(amazing-lores-game PRIVATE ALLOW_HIRES=0)
```

## 32blit API Support

These features of the 32blit API are currently unsupported on any pico-based device:

- Joystick
- `HOME` and `MENU` buttons
- Accelerometer
- Vibration
- Paletted screen mode
- JPEG decoding
- `OpenMode::cached`

Additionally some supported features have limitations:

- The `screen` surface is RGB565 instead of RGB888
- `hires` screen mode is not double-buffered, usually resulting in a lower framerate
- `get_metadata` is missing the `author` and `category` fields
- `blit::random` is not a hardware generator
- Multiplayer has no host support
- Using the MP3 decoder is probably not a good idea

### Board-specific details

|           | 32blit                       | PicoSystem                  | VGA Board + Pico
|-----------|------------------------------|-----------------------------|-----------
| CPU       | 480MHz                       | 250MHz †                    | 250MHz †
| RAM       | 611K §                       | 151K to 207K (lores only ¶) | ~207K
| FPU       | Yes                          | No                          | No
| Buttons   | 6 + reset                    | 4 + power                   | USB HID Gamepad
| Joystick  | Yes                          | No                          | No
| Tilt      | Yes                          | No                          | No
| Sound     | 8CH mini speaker             | 1CH piezo booper ‡          | 8CH 3.5mm jack (i2s DAC)
| Storage   | 32MB XiP QSPI + 128K Flash   | 12MB XiP QSPI               | 1.5MB XiP QSPI
|           | SD card for data             | 4MB QSPI for data (FAT)     | 512K QSPI for data (FAT)
| Screen    | 320x240 (160x120 lores)      | 240x240 (120x120 lores)     | 160x120 only
| Firmware  | Yes                          | No                          | No
| Launcher  | Browse + Launch Games        | No                          | No
| LED       | Yes                          | Yes                         | No


* † - technically 2 cores overclocked from 133MHz to 250MHz but the 32Blit SDK uses only one
* ‡ - makes a best-effort attempt to play any `SQUARE` waveforms (single-channel)
* § - 362K main RAM, 64K D3 RAM, 127K DTCMRAM, 58K ITCMRAM
* ¶ - setting `ALLOW_HIRES=0` allocates a doubled buffered 120x120 16bit framebuffer (56.25k) and disables the hires screen mode.
