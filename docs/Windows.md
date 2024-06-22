# Building & Running on Windows <!-- omit in toc -->

These instructions cover building 32blit on Windows (without WSL).

- [Prerequisites](#prerequisites)
- [Setting Up](#setting-up)
- [Building & Running on 32Blit](#building--running-on-32blit)
- [Building & Running Locally](#building--running-locally)
  - [Build Everything](#build-everything)

To build using Visual Studio [see here](Windows-VisualStudio.md). The rest of these instructions cover command-line builds.

## Prerequisites

- Windows Terminal - grab it from the Windows Store
- [Git for Windows](https://git-scm.com/download/win)
- [Python](https://www.python.org/downloads/) - eg: "python-3.9.6-amd64.exe"
- The [GNU Arm Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads) (For 32blit device builds, use the 9-2020-q2-update version). Make sure to select the "Add path to environment variable" at the end of the setup.
 - At least the [Visual Studio build tools](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022) (For local builds, select "Desktop development with C++"). This includes CMake and NMake.

If you have Visual Studio installed you can Modify the installation and select "MSVC vXXX - VS 2022 C++ x64/x86 build tools (Latest)" and "C++ CMake tools for Windows"

## Setting Up

Open "Developer Command Prompt for VS 2022." This is where you'll build the 32blit SDK and flash .blit files to your device.

Install the 32blit tools:

```
py -m pip install 32blit
```

This may result in a waning similar to:

```
WARNING: The script 32blit.exe is installed in 'C:\Users\[Name]\AppData\Local\Programs\Python\Python39\Scripts' which is not on PATH.
```

You will either need to add that to your PATH, or run the tools as `py -m ttblit ...` instead of `32blit ...`.

You should now be able to run `32blit.exe`, you can test your connection to your 32blit with:

```
32blit flash list
```

## Building & Running

### On 32Blit

If you want to run code on 32Blit, you should now refer to [Building & Running On 32Blit](32blit.md). On Windows you will need to add `-G"NMake Makefiles"` to your cmake commands, for example:

```
mkdir build.stm32
cd build.stm32
cmake -G"NMake Makefiles" -DCMAKE_TOOLCHAIN_FILE=../32blit.toolchain ..
```

Now to build an example, type:

```
cmake --build . --target raycaster
```

This will produce `examples/raycaster/raycaster.blit`.

You can build & flash to your 32blit with one command by adding ".flash" to the end of any target, for example:

```
cmake --build . --target raycaster.flash
```

### Locally (Windows .exe)

Set up the 32Blit Makefile from the root of the repository with the following commands:

```shell
mkdir build
cd build
cmake -G"NMake Makefiles" ..
```

Now to make any example, type:

```
cmake --build . --target raycaster
```

This will produce `examples\raycaster\raycaster.exe` which you should run with:

```
examples\raycaster\raycaster
```

### List Targets

For an exhaustive list of all the examples you can build, type:

```
cmake --build . --target help
```

### Build Everything

Whether you're building locally or for device you can build the SDK and all examples by typing:

```
cmake --build .
```

When the build completes you should be able to run/install any example.
