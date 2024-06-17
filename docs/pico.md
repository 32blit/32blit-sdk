# 32blit + Pico SDK <!-- omit in toc -->

The Pico port brings the 32blit SDK to PicoSystem and other RP2040-based devices.

Since RP2040 is slower and less capable than 32blit's STM32H750 there are some limitations, but most of the 32blit SDK conceniences work well.

- [Why use 32blit SDK on PicoSystem?](#why-use-32blit-sdk-on-picosystem)
- [Building The SDK \& Examples](#building-the-sdk--examples)
  - [Fetch Pico SDK Automatically (Quick-Start)](#fetch-pico-sdk-automatically-quick-start)
    - [Building Examples](#building-examples)
  - [Existing Pico SDK (Advanced)](#existing-pico-sdk-advanced)
- [Starting Your Own 32blit SDK Project](#starting-your-own-32blit-sdk-project)
  - [Coniguring PicoSystem builds](#coniguring-picosystem-builds)
  - [Building](#building)
  - [Copying to your PicoSystem](#copying-to-your-picosystem)
  - [Extra configuration](#extra-configuration)
- [API Limitations \& Board Details](#api-limitations--board-details)
  - [Unsupported Features](#unsupported-features)
  - [Limitations](#limitations)
  - [Board-specific details](#board-specific-details)
- [Troubleshooting](#troubleshooting)
  - [fatal error: tusb.h: No such file or directory](#fatal-error-tusbh-no-such-file-or-directory)


## Why use 32blit SDK on PicoSystem?

The number 1 reason is portability! 32blit SDK will build for:

* Windows
* macOS
* Linux
* Emscripten (WebAssembly)
* PicoSystem
* 32blit

And is portable to any platform supporting SDL2.

This means you can ship your game to more people on more platforms, share it online to play, and reach a little further than the confines of PicoSystem!

Additionally the 32blit SDK has some conveniences:

* Tiled editor .tmx support for levels
* An asset pipeline for converting fonts & spritesheets for use on device
* A boilerplate project with GitHub Actions - https://github.com/32blit/picosystem-boilerplate

## Building The SDK & Examples

We recommend using Linux to work with PicoSystem/Pico SDK. It's the path of least resistance!

This guide was tested with Ubuntu 21.04, and most of these instructions will work in its WSL (Windows Subsystem for Linux) equivalent.

You'll need a compiler and a few other dependencies to get started building C++ for PicoSystem:

```
sudo apt install git gcc g++ gcc-arm-none-eabi cmake make \
python3 python3-pip python3-setuptools \
libsdl2-dev libsdl2-image-dev libsdl2-net-dev unzip
```

And the 32blit tools:

```
pip3 install 32blit
```

If pip gives you warnings about 32blit being installed in a directory not on PATH, make sure you add it, eg:

```
export PATH=$PATH:~/.local/bin
```

You might also want to add this to the bottom of your `~/.bashrc`.

And finally you should fetch the 32blit SDK and examples:

```
git clone https://github.com/32blit/32blit-sdk
git clone https://github.com/32blit/32blit-examples
```

### Fetch Pico SDK Automatically (Quick-Start)

You can use Pico SDK's fetch-from-git feature and build like so:

```
cd 32blit-sdk
mkdir build.pico
cd build.pico
cmake .. -D32BLIT_DIR=`pwd`/.. -DPICO_SDK_FETCH_FROM_GIT=true -DPICO_EXTRAS_FETCH_FROM_GIT=true -DPICO_BOARD=pimoroni_picosystem
```

The 32blit SDK includes only `picosystem-hardware-test` which you can make with:

```
make picosystem-hardware-test
```


#### Building Examples

The [32blit-examples](https://github.com/32blit/32blit-examples) repository includes a series of demos showcasing various 32blit SDK features.

In order to compile examples against pico-sdk, your directory tree should look something like:

- root_dir
    - 32blit-sdk
    - 32blit-examples

For example:

```
cd 32blit-examples
mkdir build.pico
cd build.pico
cmake .. -D32BLIT_DIR=`pwd`/.. -DPICO_SDK_FETCH_FROM_GIT=true -DPICO_EXTRAS_FETCH_FROM_GIT=true -DPICO_BOARD=pimoroni_picosystem
```

Now you can start hacking on an existing example, or skip to [Starting Your Own 32blit SDK Project](#starting-your-own-32blit-sdk-project).

### Existing Pico SDK (Advanced)

This requires a working Pico SDK setup ([Getting started with Raspberry Pi Pico](https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf)), the 32blit tools and a copy of the 32blit SDK (this repository).

To build for a pico-based device you need to specify both the 32blit and Pico SDK paths, and the device/board you are building for.

Your directory tree should look something like:

- root_dir
    - 32blit-sdk
    - 32blit-examples
    - pico-sdk
    - pico-extras

To build the examples for a PicoSystem:

```
cd 32blit-examples
mkdir build.pico
cd build.pico
cmake .. -D32BLIT_DIR=`pwd`/.. -DPICO_SDK_PATH=/path/to/pico-sdk -DPICO_BOARD=pimoroni_picosystem
```

And then run `make` as usual.

Now you can start hacking on an existing example, or skip to [Starting Your Own 32blit SDK Project](#starting-your-own-32blit-sdk-project).

## Starting Your Own 32blit SDK Project

We've created a boilerplate 32blit SDK project to get you started: https://github.com/32blit/picosystem-boilerplate/

Click the green "Use this template" button to start creating your new project.

### Coniguring PicoSystem builds

Clone your new GitHub project to your local machine alongside the "pico-sdk" directory.

Create a new build directory for PicoSystem, the name doesn't matter but we tend to use "build.pico":

```
cd your-repo-name
mkdir build.pico
cd build.pico
```

Then configure, like so:

```
cmake .. -DCMAKE_TOOLCHAIN_FILE=../../32blit-sdk/pico.toolchain -DPICO_BOARD=pimoroni_picosystem
```
This requires the Pico SDK, Pico Extras and 32blit SDK to be alongside your project directory.

Alternatively you can ask the Pico SDK to fetch itself from git:

```
cmake .. -DCMAKE_TOOLCHAIN_FILE=../../32blit-sdk/pico.toolchain -DPICO_BOARD=pimoroni_picosystem -DPICO_SDK_FETCH_FROM_GIT=true -DPICO_EXTRAS_FETCH_FROM_GIT=true
```

:warning: Note: you should probably grab local copies of `pico-sdk` and `pico-extras` somewhere memorable, since fetching them from git every time you configure will get tedious!

### Building

Finally type `make` to build your project into a `.uf2` file compatible with PicoSystem. This will output `your-project-name.uf2` which you must copy to your PicoSystem.

### Copying to your PicoSystem

Connect your PicoSystem to your computer using a USB Type-C cable.

From a power-off state, hold down X (the top face button) and press Power (the button at the top left, next to the USB Type-C port).

Your PicoSystem should mount as "RPI-RP2". On Linux this might be `/media/<username>/RPI-RP2`:

```
cp your-project-name.uf2 /media/`whoami`/RPI-RP2
```

The file should copy over, and your PicoSystem should automatically reboot into your game.

### Extra configuration

If you're not using `hires` mode and need some more RAM, it can be disabled:
```cmake
...

blit_executable(amazing-lores-game ...)

...

target_compile_definitions(amazing-lores-game PRIVATE ALLOW_HIRES=0)
```

## API Limitations & Board Details

### Unsupported Features

These features of the 32blit API are currently unsupported on any pico-based device:

- Accelerometer
- Vibration
- Paletted screen mode
- JPEG decoding
- `OpenMode::cached`

### Limitations

Additionally some supported features have limitations:

- The `screen` surface is RGB565 instead of RGB888
- `hires` screen mode is not double-buffered, usually resulting in a lower framerate
- `blit::random` is `pico_rand`
- Multiplayer has fixed host/device role
- Using the MP3 decoder is probably not a good idea

### Board-specific details

The RP2040/Pico port supports PicoSystem, PicoVision and VGA board. Below is a table showing which features are available on each platform, and which `PICO_BOARD` to use to target them:

|            | 32blit                       | PicoSystem                  | PicoVision               | VGA Board                
|------------|------------------------------|-----------------------------|--------------------------|--------------------------
| PICO_BOARD | N/A                          | pimoroni_picosystem         | pico_w                   | vgaboard                 
| PICO_ADDON | N/A                          | N/A                         | pimoroni_picovision      | N/A                      
| CPU        | 480MHz                       | 250MHz †                    | 250MHz †                 | 250MHz †                 
| RAM        | 583K §                       | 151K to 207K (lores only ¶) | ~250K                    | ~189K                    
| FPU        | Yes                          | No                          | No                       | No                       
| Buttons    | 6 + reset                    | 4 + power                   | HID Gamepad (up to 6)    | HID Gamepad (up to 6)    
| Joystick   | Yes                          | No                          | HID Gamepad              | HID Gamepad              
| Tilt       | Yes                          | No                          | No                       | No                       
| Sound      | 8CH mini speaker             | 1CH piezo booper ‡          | 8CH 3.5mm jack (i2s DAC) | 8CH 3.5mm jack (i2s DAC) 
| Storage    | 32MB XiP QSPI + 128K Flash   | 12MB XiP QSPI               | 2MB XiP QSPI             | 2MB XiP QSPI             
|            | SD card for data             | 4MB QSPI for data (FAT)     | SD card for data         | SD card for data         
| Screen     | 320x240 (160x120 lores)      | 240x240 (120x120 lores)     | 320x240 (160x120 lores)  | 160x120 only             
| Firmware   | Yes                          | No                          | No                       | No                       
| Launcher   | Browse + Launch Games        | No                          | No                       | No                       
| LED        | Yes                          | Yes                         | No                       | No                       

* † - technically 2 cores overclocked from 133MHz to 250MHz but the 32Blit SDK uses only one
* ‡ - makes a best-effort attempt to play any `SQUARE` waveforms (single-channel)
* § - 362K main RAM, 64K D3 RAM, 127K DTCMRAM, 30K ITCMRAM
* ¶ - setting `ALLOW_HIRES=0` allocates a doubled buffered 120x120 16bit framebuffer (56.25k) and disables the hires screen mode.

# Troubleshooting

## fatal error: tusb.h: No such file or directory

You forgot to `git submodule update --init` in your local copy of `pico-sdk`.