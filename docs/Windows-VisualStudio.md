# Visual Studio 2019

You can use Visual Studio 2019 to examine the samples (compile them and run them in an SDL window) or to build your own apps for the 32blit API.

See [Building & Running On 32Blit](32blit.md) if you want to compile examples/projects to run on 32Blit.

## Requirements

You will need Visual Studio 2019 (preferably version 16.4). 

Make sure you install C++ desktop development support.

You will also need to download SDL2 development libraries from the [SDL homepage](https://www.libsdl.org/download-2.0.php). Here find the latest version of the VC development libraries (at the time of this writing SDL2-devel-2.0.10-VC.zip). Additionally, download SDL2_image from [here](https://www.libsdl.org/projects/SDL_image/) (SDL2_image-devel-2.0.5-VC.zip).

Place these in the `vs\sdl\` folder. You will need to merge the include/lib directories.

There are two methods of building with Visual Studio:

## Option 1: Use the solution file

This should be the most familiar option for existing Visual Studio users.

The solutions and projects are made to use toolset version c142.

The solution file is located at `vs\32blit.sln`. It contains two static linked libraries, _32blit_ and _32blit-sdl_ and all the examples that will compile to .EXE. 

### Get started with your own game

There is also a skeleton game created for you in `vs\game.sln`. This is an empty skeleton with some comments to get you started with your own game (if you do not want to start tweaking one of the examples).

## Option 2: Use Visual Studio's built-in CMake support

This has the advantage of being closer to the build for the device.

1. Open Visual Studio

2. `File` > `Open` > `Folder` and open the folder where you cloned this repo. (Alternatively, if you haven't cloned the repo yet, use `File` -> `Clone or check out code`)

3. Build!

To find the built files use `Project` > `CMake Cache` > `Open in Explorer`.

[More info about using CMake with Visual Studio](https://docs.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=vs-2019)

## Troubleshooting

If you see errors such as `Cannot open include file: 'SDL.h': No such file or directory` and `cannot open file 'SDL2.lib'` you've probably extracted the SDL development libraries wrong. Inside your sdl folder you should have the folders docs, include and lib not SDL2-2.0.10.
