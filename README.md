# 32blit Beta Testing Release

The code in this repository is intended for 32blit beta testers only. It's not intended for general release or for end users. If you expect to be able to get started quickly and easily- you're going to have a bad time.

The 32blit API itself isn't quite finalised yet and may be prone to sudden and sweeping rewrites- that's to say that feedback is appreciated, but your PRs might be in vain.

That said, we're really keen to hear feedback about the setup process, our documentation, ideal tooling, and generally how best get medium to advanced developers up and running with 32blit and C++. 

While the 32blit API is not finalised, this repository represents an overview of how our C++ tooling will work and how we expect to separate user projects from API code andt he SDL/STM32 front-ends.

# You Will Need

1. Some experience writing/compiling C/C++ software
2. `gcc` for compiling test builds
3. `arm-gcc` for compiling STM32 builds
4. A DFU upload tool, on Windows it's easiest to just use "DfuSe Demonstration"
5. Ubuntu on Windows + Xming if you're running on Windows 10, or a Linux VM if you prefer

# Overview

## 32blit

The `32blit` directory contains the API/engine for developing your game. This engine contains all the graphics, sound and game logic functionality you'll need for building a 32blit game.

## 32blit-sdl

The `32blit-sdl` directory contains an SDL2 HAL for 32blit compatible with Linux and Linux-on-Windows using Xming. You can use it to run your 32blit projects on your computer and test/debug them a little quicker.

You should be careful relying upon it, however, since `32blit-sdl` is *not an emulator* you may run into memory or performance problems when deploying your game to a 32blit console.

You'll need to make sure you:

1. `sudo apt install libsdl2-dev`

Then, to build your project for testing, go into the *32blit-sdl* directory 

```
cd 32blit-sdl
```

and compile the application:

```
make ../examples/<project_name>
```

To run the application on your computer, use the following command (from within the same directory):

```
DISPLAY=:0.0 ./build/<project_name>/<project_name>
```

For example, to run the Shmup game, do this:

```
DISPLAY=:0.0 ./build/shmup/shmup
```

### Video Capture

`32blit-sdl` supports optional FFMPEG video capture, which you can enable by grabbing a FFMPEG snapshot, building it, and turning on `ENABLE_FFMPEG` when building your project.


1. `sudo apt install liblzma-dev`
2. `wget https://github.com/FFmpeg/FFmpeg/archive/n4.1.4.zip`
3. `unzip n4.1.4.zip`
4. `cd FFmpeg-n4.1.4`
5. `./configure --prefix=$(pwd)/build`
6. `make && make install`

Then build your 32blit project with:

```
make ../example/<project_name> ENABLE_FFMPEG=true FFMPEG=../FFmpeg-n4.1.4/build
```

When running, you can now hit `r` to start and stop recording.

## 32blit-stm32

The `32blit-stm32` directory contains the STM32 HAL for 32blit, compatible with the STM32H750. Once you're ready to get your project running on a 32blit console you can use this to build a firmware by running:

```
make USER_CODE=../examples/<project_name>/<project_name>.cpp
```

## Examples

The `examples` directory contains example projects, these can be built into both SDL or STM32 binaries and cover a range of techniques from simple concepts to complete games.
