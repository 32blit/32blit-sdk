# 32blit Beta Testing Release

The code in this repository is intended for 32blit beta testers only. It's not intended for general release or for end users. If you expect to be able to get started quickly and easily- you're going to have a bad time.

The 32blit API itself isn't quite finalised yet and may be prone to sudden and sweeping rewrites- that's to say that feedback is appreciated, but your PRs might be in vain.

That said, we're really keen to hear feedback about the setup process, our documentation, ideal tooling, and generally how best get medium to advanced developers up and running with 32blit and C++. 

While the 32blit API is not finalised, this repository represents an overview of how our C++ tooling will work and how we expect to separate user projects from API code and the SDL/STM32 front-ends.

# You Will Need

1. Some experience writing/compiling C/C++ software
2. `gcc` for compiling test builds
3. `arm-gcc` for compiling STM32 builds
4. `cmake` and `make` for building 32blit libraries and examples
5. A DFU upload tool, on Windows it's easiest to just use "DfuSe Demonstration"
6. Ubuntu on Windows 10, or a Linux VM if you prefer.

On Windows 10 you can either build for win32 or use XMing and run your builds in WSL by prefixing with `DISPLAY=:0.0`.

# Overview

## 32blit

The `32blit` directory contains the API/engine for developing your game. This engine contains all the graphics, sound and game logic functionality you'll need for building a 32blit game.

## 32blit-sdl

The `32blit-sdl` directory contains an SDL2 HAL for 32blit compatible with Linux and Linux-on-Windows using Xming. You can use it to run your 32blit projects on your computer and test/debug them a little quicker.

You should be careful relying upon it, however, since `32blit-sdl` is *not an emulator* you may run into memory or performance problems when deploying your game to a 32blit console.

If you're planning to use the SDL HAL you'll need to make sure you:

```
sudo apt install libsdl2-dev
```

## 32blit-stm32

The `32blit-stm32` directory contains the STM32 HAL for 32blit, compatible with the STM32H750. Once you're ready to get your project running on a 32blit console you will need to ensure you have the `arm-gcc` toolchain installed and then follow the instructions below for "Building & Running On 32Blit"

## Examples / Projects

### Building & Running Locally Using The SDL HAL (Linux / WSL + XMing)

To build your project for testing, go into the relevant example directory. We'll use `palette-cycle` to demonstrate:

```
cd examples/palette-cycle
```

prepare the Makefile with CMake:

```
mkdir build
cd build
cmake ..
```

and compile the example:

```
make
```

To run the application on your computer, use the following command (from within the same directory):

```
./palette-cycle
```

If you're using WSL and XMing you will need to add a `DISPLAY` variable like so:

```
DISPLAY=:0.0 ./palette-cycle
```

### Building & Running on Win32 (WSL + MinGW)

TODO: Investigate why this outputs `cannot create target because another target with the same name already exists` errors.

To build your project for Win32 you'll need `i686-w64-mingw32-gcc` and `i686-w64-mingw32-g++`.

You'll also need to cross-compile SDL2 and install it wherever you like to keep your cross-compile libraries.

```
wget https://www.libsdl.org/release/SDL2-2.0.10.zip
unzip SDL2-2.0.10.zip
cd SDL2-2.0.10.zip
mkdir build.mingw
cd build.mingw
../configure --target=x86_64-w64-mingw32 --host=x86_64-w64-mingw32 --build=x86_64--linux --prefix=/usr/local/cross-tools/x86_64-w64-mingw32/
make
sudo make install
```

Finally, from the root directory of your project:

```
mkdir build.mingw
cd build.mingw
cmake .. -DCMAKE_TOOLCHAIN_FILE=../../../mingww.toolchain -DSDL2_DIR=/usr/local/cross-tools/x86_64-w64-mingw32/lib/cmake/SDL2
```

### Building & Running on OSX

You will need build tools and CMAKE. Assuming you have [homebrew](https://docs.brew.sh/Installation) installed

``` shell
xcode-select --install
brew install cmake
```

You'll also need to build and install SDL2...

``` shell
wget https://www.libsdl.org/release/SDL2-2.0.10.zip
unzip SDL2-2.0.10.zip
cd SDL2-2.0.10
mkdir build
cd build
../configure
make
sudo make install
```

Finally, from the root directory of this repo

``` shell
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../darwin.toolchain
make
```

When the build completes you should find all the examples within the build directory

### Building & Running On 32blit

To build your project for 32blit using arm-none-eabi-gcc you should prepare the Makefile with CMake using the provided toolchain file.

From the root of your project:

```
mkdir build.stm32
cd build.stm32
cmake .. -DCMAKE_TOOLCHAIN_FILE=../../../32blit.toolchain
```

And then `make` as normal.

The result of your build will be a `.bin`, `.hex` and `.elf` file. You can turn the `.bin` into a DfuSe-compatible DFU file using th provided `dfu` tool:

```
../../../tools/dfu build --out palette-cycle.dfu palette-cycle.bin
```

### Video Capture

`32blit-sdl` supports optional FFMPEG video capture, which you can enable by grabbing a FFMPEG snapshot, building it, and turning on `ENABLE_FFMPEG` when building your project.


1. `sudo apt install liblzma-dev`
2. `wget https://github.com/FFmpeg/FFmpeg/archive/n4.1.4.zip`
3. `unzip n4.1.4.zip`
4. `cd FFmpeg-n4.1.4`
5. `./configure --prefix=$(pwd)/build`
6. `make && make install`

Then configure your 32blit project with:

```
mkdir build
cd build
cmake .. -DVIDEO_CAPTURE=true
```

When running your game, you can now hit `r` to start and stop recording.

## Examples

The `examples` directory contains example projects, these can be built into both SDL or STM32 binaries and cover a range of techniques from simple concepts to complete games.
