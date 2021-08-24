# 32Blit + Pico SDK <!-- omit in toc -->

Mashing two SDKs together to run 32Blit games on even less powerful hardware!

- [Prerequisites](#prerequisites)
- [32Blit API Support](#32blit-api-support)
  - [Board-specific details](#board-specific-details)
- [Building](#building)
  - [Your Projects](#your-projects)
  - [Extra configuration](#extra-configuration)

## Prerequisites

This requires a working Pico SDK setup, the 32blit tools and a copy of the 32Blit SDK (this repository).

## 32Blit API Support

These features of the 32Blit API are currently unsupported on any pico-based device:

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
| Devce/board                        | Display                            | Sound | Input | LED |
| -----------                        | -------                            | ----- | ----- | --- |
| PicoSystem (`pimoroni_picosystem`) | `lores`: 120x120, `hires`: 240x240 | TODO  | GPIO  | Yes |
| VGA Board (`vgaboard`)             | `lores`: 160x120                   | I2S   | TODO  | No  |

## Building

To build for a pico-based device you need to specify both the 32Blit and Pico SDK paths, and the device/board you are building for.

To build the examples for a PicoSystem:
```
mkdir build.pico
cd build.pico
cmake .. -D32BLIT_DIR=.. -DPICO_SDK_PATH=/path/to/pico-sdk -DPICO_BOARD=pimoroni_picosystem
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
