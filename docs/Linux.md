# Building & Running on Linux

First install the required tools:

```
sudo apt install git gcc g++ gcc-arm-none-eabi cmake make python3 python3-pip python3-setuptools libsdl2-dev libsdl2-image-dev
sudo pip3 install construct bitstring
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
