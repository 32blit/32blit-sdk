# 32blit + Pico SDK <!-- omit in toc -->

The Pico port brings the 32blit SDK to PicoSystem and other RP2040-based devices.

Since RP2040 is slower and less capable than 32blit's STM32H750 there are some limitations, but most of the 32blit SDK conceniences work well.

- [Wh y use 32blit SDK on PicoSystem?](#wh-y-use-32blit-sdk-on-picosystem)
- [Building](#building)
  - [Linux](#linux)
    - [Fetch Pico SDK Automatically (Quick-Start)](#fetch-pico-sdk-automatically-quick-start)
    - [Existing Pico SDK (Advanced)](#existing-pico-sdk-advanced)
- [Starting Your Own 32blit SDK Project](#starting-your-own-32blit-sdk-project)
  - [Enabling PicoSystem builds](#enabling-picosystem-builds)
  - [Enabling PicoSystem CI](#enabling-picosystem-ci)
  - [Extra configuration](#extra-configuration)
- [API Limitations & Board Details](#api-limitations--board-details)
  - [Unsupported Features](#unsupported-features)
  - [Limitations](#limitations)
  - [Board-specific details](#board-specific-details)

## Wh y use 32blit SDK on PicoSystem?

The number 1 reason is portability! 32blit SDK will build for:

1. Windows
2. macOS
3. Linux
4. Emscripten (Web assembly)
5. PicoSystem
6. 32blit

And is portable to any platform supporting SDL2.

This means you can ship your game to more people on more platforms, share it online to play, and reach a little further than the confines of PicoSystem!

Additionally the 32blit SDK has some conveniences:

1. Tiled editor .tmx support for levels
2. An asset pipeline for converting fonts & spritesheets for use on device
3. A boilerplate project with GitHub Actions

## Building

### Linux

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

#### Fetch Pico SDK Automatically (Quick-Start)

You can use Pico SDK's fetch-from-git feature and build like so:

```
git clone https://github.com/32blit/32blit-sdk
cd 32blit-sdk
mkdir build.pico
cd build.pico
cmake .. -D32BLIT_DIR=`pwd`/.. -DPICO_SDK_FETCH_FROM_GIT=true -DPICO_EXTRAS_FETCH_FROM_GIT=true -DPICO_BOARD=pimoroni_picosystem
```

And then run `make` as usual.

Now you can start hacking on an existing example, or skip to [Starting Your Own 32blit SDK Project](#starting-your-own-32blit-sdk-project).

#### Existing Pico SDK (Advanced)

This requires a working Pico SDK setup ([Getting started with Raspberry Pi Pico](https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf)), the 32blit tools and a copy of the 32blit SDK (this repository).

To build for a pico-based device you need to specify both the 32blit and Pico SDK paths, and the device/board you are building for.

To build the examples for a PicoSystem:

```
git clone https://github.com/32blit/32blit-sdk
cd 32blit-sdk
mkdir build.pico
cd build.pico
cmake .. -D32BLIT_DIR=`pwd`/.. -DPICO_SDK_PATH=/path/to/pico-sdk -DPICO_BOARD=pimoroni_picosystem
```

And then run `make` as usual.

Now you can start hacking on an existing example, or skip to [Starting Your Own 32blit SDK Project](#starting-your-own-32blit-sdk-project).

## Starting Your Own 32blit SDK Project

We've created a boilerplate 32blit SDK project to get you started: https://github.com/32blit/32blit-boilerplate/

If you use GitHub, just click the green "Use this template" button to start creating your new project.

Alternatively you can run `32blit setup` for a step-by-step boilerplate setup.

### Enabling PicoSystem builds

To set up the boilerplate project for PicoSystem builds, you must import the Pico SDK before the `project` line in your CMakeLists.txt.

```cmake
cmake_minimum_required(VERSION 3.9)

# this is a wrapper for the pico sdk import files, which are only included if PICO_BOARD is set
include(${32BLIT_DIR}/32blit-pico/sdk_import.cmake OPTIONAL)

project(my-amazing-gane)
...
```

Then configure (as above) with something like:
```
cmake .. -D32BLIT_DIR=/path/to/32blit-sdk -DPICO_SDK_PATH=/path/to/pico-sdk -DPICO_BOARD=pimoroni_picosystem
```

### Enabling PicoSystem CI

You can also enable automatic, GitHub actions builds of PicoSystem .uf2 files when you tag a release on your project. Make the following changes to `.github/workflows/build.yml`:

```diff
diff --git a/.github/workflows/build.yml b/.github/workflows/build.yml
index db26a2b..b37df91 100644
--- a/.github/workflows/build.yml
+++ b/.github/workflows/build.yml
@@ -38,6 +38,14 @@ jobs:
             cmake-args: -D32BLIT_DIR=$GITHUB_WORKSPACE/32blit-sdk -DCMAKE_TOOLCHAIN_FILE=$GITHUB_WORKSPACE/32blit-sdk/32blit.toolchain
             apt-packages: gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib python3-setuptools
 
+          - os: ubuntu-20.04
+            pico-sdk: true
+            name: PicoSystem
+            cache-key: picosystem
+            release-suffix: PicoSystem
+            cmake-args: -D32BLIT_DIR=$GITHUB_WORKSPACE/32blit-sdk -DPICO_SDK_PATH=$GITHUB_WORKSPACE/pico-sdk -DPICO_BOARD=pimoroni_picosystem
+            apt-packages: gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib python3-setuptools
+
           - os: ubuntu-20.04
             name: Emscripten
             release-suffix: WEB
@@ -75,6 +83,22 @@ jobs:
         repository: 32blit/32blit-sdk
         path: 32blit-sdk
 
+    # pico sdk/extras for some builds
+    - name: Checkout Pico SDK
+      if: matrix.pico-sdk
+      uses: actions/checkout@v2
+      with:
+        repository: raspberrypi/pico-sdk
+        path: pico-sdk
+        submodules: true
+
+    - name: Checkout Pico Extras
+      if: matrix.pico-sdk
+      uses: actions/checkout@v2
+      with:
+        repository: raspberrypi/pico-extras
+        path: pico-extras
+
     # Linux dependencies
     - name: Install Linux deps
       if: runner.os == 'Linux'

```

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

- Joystick
- `HOME` and `MENU` buttons
- Accelerometer
- Vibration
- Paletted screen mode
- JPEG decoding
- `OpenMode::cached`

### Limitations

Additionally some supported features have limitations:

- The `screen` surface is RGB565 instead of RGB888
- `hires` screen mode is not double-buffered, usually resulting in a lower framerate
- `get_metadata` is missing the `author` and `category` fields
- `blit::random` is not a hardware generator
- Multiplayer has no host support
- Using the MP3 decoder is probably not a good idea

### Board-specific details


The RP2040/Pico port supports PicoSystem and VGA board. Below is a table showing which features are available on each platform, and which `PICO_BOARD` to use to target them:

|            | 32blit                       | PicoSystem                  | VGA Board
|------------|------------------------------|-----------------------------|-----------
| PICO_BOARD | N/A                          | pimoroni_picosystem         | vgaboard
| CPU        | 480MHz                       | 250MHz †                    | 250MHz †
| RAM        | 611K §                       | 151K to 207K (lores only ¶) | ~207K
| FPU        | Yes                          | No                          | No
| Buttons    | 6 + reset                    | 4 + power                   | HID Gamepad
| Joystick   | Yes                          | No                          | No
| Tilt       | Yes                          | No                          | No
| Sound      | 8CH mini speaker             | 1CH piezo booper ‡          | 8CH 3.5mm jack (i2s DAC)
| Storage    | 32MB XiP QSPI + 128K Flash   | 12MB XiP QSPI               | 1.5MB XiP QSPI
|            | SD card for data             | 4MB QSPI for data (FAT)     | 512K QSPI for data (FAT)
| Screen     | 320x240 (160x120 lores)      | 240x240 (120x120 lores)     | 160x120 only
| Firmware   | Yes                          | No                          | No
| Launcher   | Browse + Launch Games        | No                          | No
| LED        | Yes                          | Yes                         | No

* † - technically 2 cores overclocked from 133MHz to 250MHz but the 32Blit SDK uses only one
* ‡ - makes a best-effort attempt to play any `SQUARE` waveforms (single-channel)
* § - 362K main RAM, 64K D3 RAM, 127K DTCMRAM, 58K ITCMRAM
* ¶ - setting `ALLOW_HIRES=0` allocates a doubled buffered 120x120 16bit framebuffer (56.25k) and disables the hires screen mode.
