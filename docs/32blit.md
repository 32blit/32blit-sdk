# Building & Running On 32blit

To build your project for 32blit using `arm-none-eabi-gcc` you should prepare the Makefile with CMake using the provided toolchain file.

From the root of your project:

```
mkdir build.stm32
cd build.stm32
cmake .. -DCMAKE_TOOLCHAIN_FILE=../../../32blit.toolchain
```

And then `make` as normal.

The result of your build will be a `.bin`, `.hex` and `.elf` file.

To run your project on 32Blit you'll need:

1. the firmware installed.
2. to build the flash loader tool in `tools/src`.

See the relevant sections below for instructions.


## Uploading your project

This requires the flash loader tool to be in your PATH or built in an adjacent `build` or `build.mingw` directory from a local build (Run the build for your platform in the top level).

With the tool available, you can now run:
```
make [project-name].flash
```

To install and run your project. For example, you can run:

```
mkdir build.stm32
cd build.stm32
cmake .. -DCMAKE_TOOLCHAIN_FILE=../32blit.toolchain
make logo.flash
```

From the root of this repository to build, flash and run the `logo` example.

You can also run `32Blit.exe` (Windows) or `32Blit` (Linux) manually to copy a bin file to either QSPI Flash or the SD Card on your device.

For example to program to QSPI Flash from Windows:

```
32Blit.exe PROG COM8 raycaster.bin
```

Or copy to the inserted SD card:

```
32Blit.exe SAVE COM8 raycaster.bin
```

## Flashing 32Blit Firmware Via DFU

### Prepare the device

To enter DFU mode either hold the X & Y buttons and press the reset button or select `dfu mode` from the on device menu. The screen will go dark, this is normal.

### Build & Flash The Firmware

32Blit requires a firmware loader which manages games on Flash and (optionally) the SD card. You must build `firmware/flash-loader` and flash it to your device.

```
mkdir build.stm32
cd build.stm32
cmake .. -DCMAKE_TOOLCHAIN_FILE=../32blit.toolchain
make flash-loader
```

### Linux and macOS

Install `dfu-util` from your package manager then enter:

```
sudo dfu-util -a 0 -s 0x08000000 -D flash-loader.bin
```

Followed by the name of the .bin file that you just built.

### Windows

You will need [DfuSe Demonstration from st.com](https://www.st.com/en/development-tools/stsw-stm32080.html)) to flash `flash-loader.dfu` to your device.

## Troubleshooting

If you see `cannot create target because another target with the same name already exists` you've probably run `cmake ..` in the wrong directory (the project directory rather than the build directory), you should remove all but your project files and `cmake ..` again from the build directory.

If you have more than one device in DFU mode connected to your computer then find the 32blit using `lsusb` and add `-d vid:pid` to the dfu-util command. Replace `vid:pid` with the 4 character ID strings to target the correct device.

If `make example.flash` fails to find the correct port, re-run `cmake` with `-DFLASH_PORT=[PORT PATH]`.