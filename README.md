# 32blit SDK

This is the software developer kit for 32blit. It includes the 32blit game engine, plus tooling and instructions to compile your 32blit projects on Linux, Windows and macOS.

Finished 32blit games can be released simultaneously for Linux, Windows, macOS and the 32blit using the GitHub actions in the 32blit boilerplate - https://github.com/32blit/32blit-boilerplate/

# You Will Need

Some experience writing/compiling C/C++ software is required.

OS specific docs, these cover setup and command-line builds:

* [Linux](docs/Linux.md)
* [Linux - ChromeOS](docs/ChromeOS.md)
* [Windows](docs/Windows.md)
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

To get started writing your own game, you should use the 32blit boilerplate - https://github.com/32blit/32blit-boilerplate/

## Help & Support

Join the 32blit community on:

* Discord (Chat) - https://discord.gg/7qM9ftC
* Discourse (Forum) - https://forums.pimoroni.com/c/32blit/21

## Troubleshooting

If you see `cannot create target because another target with the same name already exists` you've probably run `cmake ..` in the wrong directory (the project directory rather than the build directory), you should remove all but your project files and `cmake ..` again from the build directory.

If you are getting complaints on WSL / Linux about python modules missing, you may have accidentally installed them with sudo. When using pip3 to install modules do not use sudo, this will make sure that modules are installed for the current user.

