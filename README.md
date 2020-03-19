# 32blit Beta Testing Release

The code in this repository is intended for 32blit beta testers only. It's not intended for general release or for end users. If you expect to be able to get started quickly and easily- you're going to have a bad time.

The 32blit API itself isn't quite finalised yet and may be prone to sudden and sweeping rewrites- that's to say that feedback is appreciated, but your PRs might be in vain.

That said, we're really keen to hear feedback about the setup process, our documentation, ideal tooling, and generally how best get medium to advanced developers up and running with 32blit and C++. 

While the 32blit API is not finalised, this repository represents an overview of how our C++ tooling will work and how we expect to separate user projects from API code and the SDL/STM32 front-ends.

# You Will Need

1. Some experience writing/compiling C/C++ software
2. `gcc` or Visual Studio for compiling test builds
3. `gcc-arm-none-eabi` for compiling STM32 builds
4. `cmake` and `make` for building 32blit libraries and examples
5. _Python3_ installed, along with _pip3_ (`sudo apt install python3 python3-pip` on Ubuntu/WSL) and the [32blit tools](https://github.com/pimoroni/32blit-tools) `pip3 install 32blit`
6. Ubuntu on Windows 10 WSL (_Windows Subsystem for Linux_), or a Linux VM if you prefer.
7. If you intend on building/uploading the device firmware, you will need a DFU upload tool, on Windows it's easiest to just use "DfuSe Demonstration" (available from [st.com](https://www.st.com/en/development-tools/stsw-stm32080.html)) and the following Python modules: `pip3 install construct bitstring` 

For more information about how to build on the various systems, refer to the platform specific docs in the `docs` folder!

# More docs!

Refer to the OS specific docs:

* [Linux](docs/Linux.md)
* [Linux - ChromeOS](docs/ChromeOS.md)
* [Windows - Visual Studio](docs/Windows-VisualStudio.md)
* [Windows - WSL (Advanced)](docs/Windows-WSL.md)
* [macOS](docs/macOS.md)

If you want to run in a browser, refer to the [Emscripten docs](docs/Emscripten.md)

If you want to run on device, refer to the [32blit docs](docs/32blit.md)

How to set up your editor:

* [Visual Studio Code](docs/VSCode.md)

# Overview of what is inside?

## 32blit

The `32blit` directory contains the API/engine for developing your game. This engine contains all the graphics, sound and game logic functionality you'll need for building a 32blit game.

## 32blit-sdl

The `32blit-sdl` directory contains an SDL2 HAL for 32blit compatible with Linux, macOS, Windows and Emscripten. You can use it to run your 32blit projects on your computer and test/debug them a little quicker.

You should be careful relying upon it, however, since `32blit-sdl` is *not an emulator* you may run into memory or performance problems when deploying your game to a 32blit console.

## 32blit-stm32

The `32blit-stm32` directory contains the STM32 HAL for 32blit, compatible with the STM32H750. Once you're ready to get your project running on a 32blit console you will need to ensure you have the `gcc-arm-none-eabi` toolchain installed and then follow the instructions below for [Building & Running On 32Blit](docs/32blit.md)

## Examples / Projects

The `examples` directory contains example projects, these can be built into both SDL or STM32 binaries and cover a range of techniques from simple concepts to complete games.

Refer to the OS/platform specific documentation files in the `docs/` folder for instructions on how to compile and run these examples.

The `template` directory contains a minimal game template you can copy to start your own project.

## Troubleshooting

If you see `cannot create target because another target with the same name already exists` you've probably run `cmake ..` in the wrong directory (the project directory rather than the build directory), you should remove all but your project files and `cmake ..` again from the build directory.

If you are getting complaints on WSL / Linux about python modules missing, you may have accidentally installed them with sudo. When using pip3 to install modules do not use sudo, this will make sure that modules are installed for the current user.

