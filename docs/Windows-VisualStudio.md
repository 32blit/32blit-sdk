# Visual Studio 2019 <!-- omit in toc -->

You can use Visual Studio 2019 to examine the examples (compile and run them on Windows using the SDL HAL) or to build your own apps for 32blit.

See [Building & Running On 32Blit](32blit.md) if you want to compile examples/projects to run on 32Blit.

- [Requirements](#requirements)
- [Option 1: Use the solution file](#option-1-use-the-solution-file)
- [Option 2: Use Visual Studio's built-in CMake support (Recommended)](#option-2-use-visual-studios-built-in-cmake-support-recommended)
  - [Get started with your own game](#get-started-with-your-own-game)
  - [Building your own game](#building-your-own-game)
  - [Building for 32Blit](#building-for-32blit)
- [Troubleshooting](#troubleshooting)
  - [Cannot open include file: 'SDL.h': No such file or directory](#cannot-open-include-file-sdlh-no-such-file-or-directory)
  - [`cmd.exe` not recognised](#cmdexe-not-recognised)

## Requirements

You will need Visual Studio 2019 (preferably version 16.4).

Make sure you install "Desktop development with C++". If you plan to build for 32blit, you'll also need "Linux development with C++", making sure "C++ CMake tools for Linux" and "Embedded and IoT development tools" are selected in the "Optional" section.

There are two methods of building with Visual Studio:

## Option 1: Use the solution file

This should be the most familiar option for existing Visual Studio users.

The solutions and projects are made to use toolset version c142.

The solution file is located at `vs\32blit.sln`. It contains two static linked libraries, _32blit_ and _32blit-sdl_ and all the examples that will compile to .EXE.

For the solution file to work, you will need to clone the examples repo as `examples` inside the SDK.

```shell
cd 32blit-sdk
git clone https://github.com/32blit/32blit-examples examples
```

You will also need to download SDL2 development libraries from the [SDL releases](https://github.com/libsdl-org/SDL/releases/latest). Here find the latest version of the VC development libraries (at the time of this writing SDL2-devel-2.24.0-VC.zip). Additionally, download SDL2_image from [here](https://github.com/libsdl-org/SDL_image/releases/latest) (SDL2_image-devel-2.6.1-VC.zip) and SDL2_net from [here](https://github.com/libsdl-org/SDL_net/releases/latest) (SDL2_net-devel-2.2.0-VC.zip).

Place these in the `vs\sdl\` folder. You will need to merge the include/lib folders. If you are using CMake/Open Folder, these are downloaded automatically.

## Option 2: Use Visual Studio's built-in CMake support (Recommended)

This has the advantage of being closer to the build for the device.

First you need something to build. The [32blit-examples](https://github.com/32blit/32blit-examples) repository includes a series of demos showcasing various 32blit SDK features. These instructions will assume you're building those and have cloned or extracted that repository alongside 32blit-sdk:

```shell
git clone https://github.com/32blit/32blit-examples
```

Your directory tree should look something like:

- root_dir
    - 32blit-sdk
    - 32blit-examples

1. Open Visual Studio

2. `File` > `Open` > `Folder` and open the folder where you cloned the examples repo. (Alternatively, if you haven't cloned the repo yet, use `File` -> `Clone or check out code`)

3. Build!

To find the built files use `Project` > `CMake Cache` > `Open in Explorer`.

SDL libraries will be downloaded automatically by the CMake build system.

### Get started with your own game

There is also a skeleton game project created for you at https://github.com/32blit/32blit-boilerplate . This is an empty skeleton with some comments to get you started with your own game (if you do not want to start tweaking one of the examples).

### Building your own game

1. Open Visual Studio

2. `File` > `Open` > `Folder` and open the folder containing your game.

3. Open the CMake Settings: `Project` > `CMake Settings`.

4. Scroll down to the CMake variables and wait for the list to load.

5. Press the "Browse..." button next to `32BLIT_DIR`.

6. Browse to the folder containing the 32blit SDK.

7. To add a release configuration, press "Add a new configuration..." (the plus button under "Configurations"), select "x64-Release" and repeat steps 4-6 on the new configuration.

8. Save. It should configure successfully.

9. Build!

[More info about using CMake with Visual Studio](https://docs.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=vs-2019)

### Building for 32Blit

1. Make sure the "Embedded and IoT development tools" component is installed from the `Linux development with C++` VS Workload and you have [an Arm toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads) installed.

2. Open the CMake Settings: `Project` > `CMake Settings`..

3. Press "Edit JSON" and add a new entry to the "configurations" list like this (make sure to set the path to the toolchain file):
```jsonc
    // other configs...
    {
      "name": "32Blit-Release",
      "generator": "Ninja",
      "configurationType": "Release",
      "buildRoot": "${projectDir}\\out\\build\\${name}",
      "installRoot": "${projectDir}\\out\\install\\${name}",
      "cmakeCommandArgs": "",
      "buildCommandArgs": "",
      "ctestCommandArgs": "",
      "inheritEnvironments": [ "gcc-arm" ],
      "variables": [],
      "intelliSenseMode": "linux-gcc-arm",
      "cmakeToolchain": "[path...]/32blit-sdk/32blit.toolchain",
      "environments": [ { "PATH": "${env.PATH}" } ] // this is a workaround for finding the toolchain
    }
    //...
```

4. Save.

5. Select the new config and build!

## Troubleshooting

### Cannot open include file: 'SDL.h': No such file or directory

If you see errors such as `Cannot open include file: 'SDL.h': No such file or directory` and `cannot open file 'SDL2.lib'` you've probably extracted the SDL development libraries into "SDL2-2.24.0" rather than extracting the individual folders inside. Inside your "sdl" folder you should have the folders "docs", "include" and "lib" not "SDL2-2.24.0".
