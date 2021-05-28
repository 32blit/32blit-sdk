# Building & Running on Linux <!-- omit in toc -->

These instructions cover building 32blit on Linux.

- [Prerequisites](#prerequisites)
- [Building & Running on 32Blit](#building--running-on-32blit)
- [Building & Running Locally](#building--running-locally)
  - [Build Everything](#build-everything)

## Prerequisites

You'll need to install:
 - Git
 - CMake (at least 3.9)
 - Make (or another build tool like Ninja)
 - Python (at least 3.6) + pip
 - [The 32blit tools](https://github.com/32blit/32blit-tools)

For local builds:
 - GCC
 - SDL2 + SDL2_image + SDL2_net

For 32Blit device builds:
 - Arm Embedded GCC (`gcc-arm-none-eabi`, versions 7.x-9.x should work)

New enough versions of these exist in at least Debian "buster" and Ubuntu 20.04.

```
sudo apt install git gcc g++ gcc-arm-none-eabi cmake make python3 python3-pip python3-setuptools libsdl2-dev libsdl2-image-dev libsdl2-net-dev unzip

pip3 install 32blit
```

Optionally, for building the firmware as a .DFU file (usually not needed on Linux):

```
pip3 install construct bitstring
```

## Building & Running on 32Blit

If you want to run code on 32Blit, you should now refer to [Building & Running On 32Blit](32blit.md).

## Building & Running Locally

Set up the 32Blit Makefile from the root of the repository with the following commands:

```shell
mkdir build
cd build
cmake ..
```

Now to make any example, type:

```shell
make example-name
```

For example:

```shell
make raycaster
```

This will produce `examples/raycaster/raycaster` which you should run with:

```shell
./examples/raycaster/raycaster
```

### Build Everything

Alternatively you can build everything by just typing:

```shell
make
```

When the build completes you should be able to run any example.
