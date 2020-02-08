# Building & Running On 32Blit

These instructions assume a basic familiarity with the Linux command-line and with compiling software from source.

Make sure you've prepared your 32Blit by following the instructions in:

* [Building & Flashing The 32Blit Firmware](32Blit-Firmware.md#building--flashing-the-32blit-firmware)
* [Building The 32Blit Loader Tool](32Blit-Loader.md#building-the-32blit-loader-tool)

You should also make sure you have a cross-compile environment set up on your computer, refer to the relevant documentation below:

* [Windows](Windows-WSL.md)
* [Linux](Linux.md)
* [macOS](macOS.md)
* [ChromeOS](ChromeOS.md)

## Examples

### Building An Example

To build an example for 32blit using `arm-none-eabi-gcc` you must prepare the Makefile with CMake using the provided toolchain file.

From the root of the repository:

```
mkdir build.stm32
cd build.stm32
cmake .. -DCMAKE_TOOLCHAIN_FILE=../32blit.toolchain
```

And then run `make examplename` to build an example.

The result of the build will be a `.bin`, `.hex` and `.elf` file in the relevant example directory.

For example you might type `make raycaster` which will give you `examples/raycaster/raycaster.bin`.

### Uploading An Example

This requires the [flash loader tool](32Blit-Loader.md) to be in your PATH or built in an adjacent `build` or `build.mingw` directory from a local build (Run the build for your platform in the top level).

With the tool available, and your device out of DFU mode, you can now run:

```
make [example-name].flash
```

For example you can:

```
make logo.flash
```

To build, flash and run the `logo` example.

## Your Own Projects

You can build a project based on the template by running:

```
mkdir build.stm32
cd build.stm32
cmake .. -D32BLIT_PATH="/path/to/32blit/repo" -DCMAKE_TOOLCHAIN_FILE=/path/to/32blit/repo/32blit.toolchain
make
```

# Troubleshooting

### Flasher Can't Find 32Blit Port

If `make example.flash` fails to find the correct port, re-run `cmake` with `-DFLASH_PORT=[PORT PATH]`.  

Port-detection does not work if your device is in DFU mode. Either reset it to get it out, or it it's stuck in DFU mode (or just boots into a black screen), you may need to reflash the firmware.

### Cmake Errors

If you see `cannot create target because another target with the same name already exists` you've probably run `cmake ..` in the wrong directory (the project directory rather than the build directory), you should remove all but your project files and `cmake ..` again from the build directory.
