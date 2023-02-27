# Building & Running on macOS <!-- omit in toc -->

These instructions cover building 32blit on macOS.

- [Prerequisites](#prerequisites)
  - [Python3](#python3)
    - [Installing python3](#installing-python3)
    - [Installing pip3 dependecies](#installing-pip3-dependecies)
    - [Verifying install](#verifying-install)
  - [Installing `gcc-arm-none-eabi`](#installing-gcc-arm-none-eabi)
- [Building \& Running on 32Blit](#building--running-on-32blit)
- [Building \& Running Locally](#building--running-locally)
  - [Build Everything](#build-everything)
- [Troubleshooting](#troubleshooting)

## Prerequisites

You will need build tools and CMake. Assuming you have [homebrew](https://docs.brew.sh/Installation) installed:

``` shell
xcode-select --install
brew install cmake
```

### Python3

Before trying to install python3, it's worth checking if you already have it installed (and if so, which version), by jumping to 'Verifying install', below. If you do already have it installed, skip this section.

#### Installing python3

Installing `python3` can be done with homebrew with a simple `brew install python` which installs both `python3` and `pip3`.

####  Installing pip3 dependecies

before installing 32blit tools, some binary requirements are needed:

```
brew install libjpeg
brew install freetype
````

Then install 32blit tools itself.

```
pip3 install 32blit
```

TODO: Document install of `construct` and `bitstring` for Python 3 (probably need a requirements.txt for the tools directory)


####  Verifying install

``` shell
python3 --version
```
(expected output `Python 3.7.x`)

and
``` shell
pip3 --version
```
(expected output `pip x.x.x from /usr/local/lib/python3.7/site-packages/pip (python 3.7)` or similar)

<a name="gcc"/></a>
### Installing `gcc-arm-none-eabi`

Once this is done, you'll need to install `gcc-arm-none-eabi`. The easiest way to install this tool is via homebrew with the following source:

``` shell
brew tap ArmMbed/homebrew-formulae
brew install arm-none-eabi-gcc
```

Note:
If you do not want to/ are unable to use homebrew to do this, you should be able to find the manual install instruction in the `arm-none-eabi-gcc.rb` file in [this repository](https://github.com/ARMmbed/homebrew-formulae).

______

## Building & Running on 32Blit

If you want to run code on 32Blit, you should now refer to [Building & Running On 32Blit](32blit.md).

## Building & Running Locally

You'll need to install `SDL2`, `SDL2 Image` and `SDL2 Net`

``` shell
brew install sdl2 sdl2_image sdl2_net
```

Clone the examples repository:
```
git clone https://github.com/32blit/32blit-examples

cd 32blit-examples
```

Then, set up the 32Blit Makefile from the root of the repository with the following commands:

```shell
mkdir build
cd build
cmake -D32BLIT_DIR=../../32blit-sdk ..
```
(Assuming 32blit-sdk is cloned next to 32blit-examples)

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

## Troubleshooting

If you see `cannot create target because another target with the same name already exists` you've probably run `cmake ..` in the wrong directory (the project directory rather than the build directory), you should remove all but your project files and `cmake ..` again from the build directory.
