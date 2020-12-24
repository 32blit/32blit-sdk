# Building & Running on Win32 (WSL and MinGW) <!-- omit in toc -->

These instructions cover setting up Windows Subsystem for Linux so that you can cross-compile Windows-compatible binaries with MinGW. This approach is included for completeness but not recommended, since MinGW binaries are statically linked and much larger than the Visual Studio output.

A basic knowledge of the Linux command-line, installing tools and compiling code from source is assumed.

If you're more familiar with Visual Studio then you should [follow the instructions in Windows-VisualStudio.md](Windows-VisualStudio.md)

- [Setting Up](#setting-up)
  - [Windows Subsystem for Linux (WSL)](#windows-subsystem-for-linux-wsl)
  - [Installing requirements inside WSL](#installing-requirements-inside-wsl)
- [Building & Running on 32Blit](#building--running-on-32blit)
- [Building & Running Locally](#building--running-locally)
  - [Installing SDL2 & SDL2_image](#installing-sdl2--sdl2_image)
    - [SDL2](#sdl2)
    - [SDL2_image](#sdl2_image)
  - [Building](#building)
    - [Single Example](#single-example)
    - [Build Everything](#build-everything)
  - [Troubleshooting](#troubleshooting)

## Setting Up

### Windows Subsystem for Linux (WSL)

To enable Windows Subsystem for Linux, you must access the _Turn Windows features on or off_ dialog. Find the entry for _Windows Subsystem for Linux_ and make sure it is enabled.

After that, proceed to the Microsoft Store to download Ubuntu for WSL.

### Installing requirements inside WSL

The following requirements enable cross-compile to 32Blit via ARM GCC and to Windows via MinGW:

```shell
# Ubuntu 18.04's gcc-arm-none-eabi is too old, this repo backports the one from 19.10
sudo add-apt-repository ppa:daft-freak/arm-gcc
sudo apt update

sudo apt install gcc gcc-arm-none-eabi gcc-mingw-w64 g++-mingw-w64 unzip cmake make python3 python3-pip
pip3 install 32blit construct bitstring
```

## Building & Running on 32Blit

If you want to run code on 32Blit, you should now refer to [Building & Running On 32Blit](32blit.md).

## Building & Running Locally

You can use WSL on Windows to cross-compile your project (or any 32Blit example) into a Windows .exe for testing locally.

You will need to cross-compile SDL2 for MinGW and install both it, and SDL2-image.

### Installing SDL2 & SDL2_image

This will install the SDL2 64bit mingw development headers and libraries into `/opt/local/x86_64-w64-mingw32/`.

Note: the `lib/cmake/SDL2/sdl2-config.cmake` shipped with these libraries expects them to be in `/opt/local`, if you change the install path you will have to modify this file.

First, make sure the `/opt/local/` directory exists:

```shell
sudo mkdir -p /opt/local/
```

#### SDL2

Grab and install the SDL2 mingw development package:

```shell
wget https://libsdl.org/release/SDL2-devel-2.0.14-mingw.tar.gz
tar xzf SDL2-devel-2.0.14-mingw.tar.gz
sudo cp -r SDL2-2.0.14/x86_64-w64-mingw32 /opt/local/
```

#### SDL2_image

Grab and install the SDL2_image mingw development package:

```shell
wget https://www.libsdl.org/projects/SDL_image/release/SDL2_image-devel-2.0.5-mingw.tar.gz
tar xzf SDL2_image-devel-2.0.5-mingw.tar.gz
sudo cp -r SDL2_image-2.0.5/x86_64-w64-mingw32 /opt/local/
```

### Building

Finally, set up the 32Blit Makefile from the root of the repository with the following commands:

```shell
mkdir build.mingw
cd build.mingw
cmake .. -DCMAKE_TOOLCHAIN_FILE=../mingw.toolchain
```

#### Single Example

Now to make any example, type:

```shell
make example-name
```

For example:

```shell
make raycaster
```

This will produce `examples/raycaster/raycaster.exe` which you should run with:

```shell
./examples/raycaster/raycaster.exe
```

WSL will launch the example in Windows, using the required `SDL2.dll` that will have been copied into the build root.

Don't forget to include `SDL2.dll` this if you want to redistribute a game/example.

#### Build Everything

Alternatively you can build everything by just typing:

```shell
make
```

When the build completes you should be able to run any example.

### Troubleshooting

If you see `cannot create target because another target with the same name already exists` you've probably run `cmake ..` in the wrong directory (the project directory rather than the build directory), you should remove all but your project files and `cmake ..` again from the build directory.
