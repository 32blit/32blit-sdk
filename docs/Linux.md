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
 - Arm Embedded GCC (`gcc-arm-none-eabi`, versions 8.x-12.x should work)

New enough versions of these exist in at least Debian "buster" and Ubuntu 20.04.

Install them with apt like so:

```
sudo apt install git gcc g++ gcc-arm-none-eabi cmake make \
python3 python3-pip python3-setuptools \
libsdl2-dev libsdl2-image-dev libsdl2-net-dev unzip
```

And install the 32blit tools with pip3:

```
pip3 install 32blit
```

## Building & Running on 32Blit

If you want to run code on 32Blit, you should now refer to [Building & Running On 32Blit](32blit.md).

## Building & Running Locally

First you need something to build. The [32blit-examples](https://github.com/32blit/32blit-examples) repository includes a series of demos showcasing various 32blit SDK features. These instructions will assume you're building those and have cloned or extracted that repository alongside 32blit-sdk:

```shell
git clone https://github.com/32blit/32blit-examples
```

Your directory tree should look something like:

- root_dir
    - 32blit-sdk
    - 32blit-examples

Create a build directory and configure the examples. The 32blit-sdk should be automatically detected:

```shell
cd 32blit-examples
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

# Troubleshooting

## pip3 warns about /home/user/.local/bin not on PATH

If you're running Ubuntu, Pop!_OS or similar you should be able to fix this for your current session by running:

```
source ~/.profile
```

Since `$HOME/.local/bin` is included in `$PATH` if it exists.

Otherwise, add something like the following to `~/.profile`:

```bash
# set PATH so it includes user's private bin if it exists
if [ -d "$HOME/.local/bin" ] ; then
    PATH="$HOME/.local/bin:$PATH"
fi
```

And make sure to "source" it to update your current session.

Now invoking `32blit` should work.
