# 32Blit Firmware <!-- omit in toc -->

This repository includes firmware for the 32Blit that lets you manage games on SD card, and copy or flash games via USB serial.

- [Prerequisites](#prerequisites)
- [Building & Flashing The 32Blit Firmware](#building--flashing-the-32blit-firmware)
  - [Building The Firmware](#building-the-firmware)
  - [Flashing The Firmware To Your 32Blit](#flashing-the-firmware-to-your-32blit)
    - [Prepare the device](#prepare-the-device)
    - [Linux and macOS](#linux-and-macos)
    - [Windows](#windows)
- [Troubleshooting](#troubleshooting)
  - [Finding The Right DFU Device](#finding-the-right-dfu-device)

# Prerequisites

In order to use the 32blit firmware, you will need to:

1. Build and install the 32Blit firmware (if you don't have it already)
2. The 32blit tools `pip install 32blit`
3. Flash the firmware to your 32Blit

You must make sure you have an ARM GCC cross-compile environment set up on your computer, refer to the relevant documentation below:

* [Windows](Windows-WSL.md)
* [Linux](Linux.md)
* [macOS](macOS.md)
* [ChromeOS](ChromeOS.md)

These instructions assume that you will create build directories in the *root* of this repository.

# Building & Flashing The 32Blit Firmware

## Building The Firmware

From the root of the repository prepare cmake to target 32Blit if you haven't already:

```
mkdir build.stm32
cd build.stm32
cmake .. -DCMAKE_TOOLCHAIN_FILE=../32blit.toolchain
```

Now build the firmware binary and dfu files (for Linux/macOS and Windows flashing respectively):

```
make firmware
```

## Flashing The Firmware To Your 32Blit

### Prepare the device

To enter DFU mode either hold the X & Y buttons and press the reset button or select `dfu mode` from the on device menu. The screen will go dark, this is normal.

![hold the X & Y buttons and press the reset button](resources/enter-dfu-mode.png)

### Linux and macOS

Install `dfu-util` from your package manager then enter:

```
sudo dfu-util -a 0 -s 0x08000000 -D firmware/firmware.bin
```

### Windows

You will need [DfuSe Demonstration from st.com](https://www.st.com/en/development-tools/stsw-stm32080.html)) to flash `firmware.dfu` to your device.

Once installed you should be able, from the `build.stm32` directory you created earlier, to flash 32Blit's firmware by running:

```
../tools/wsl-flash firmware/firmware.dfu
```

Note: `wsl-flash` uses a hard-coded path to DfuSe 3.0.6 in `c:\Program Files (x86)`

If this fails you can run the `DfusSeDemo` application and pick your 32Blit (it should be "STM Device in DFU Mode") from the "Available USB Devices" drop down.

In the "Upload Action" section hit "Choose" and select the `firmware.dfu` file you built earlier. (It should be in `build.stm32/firmware/firmware.dfu`) and finally hit "Upgrade" to flash the file.

# Troubleshooting

## Finding The Right DFU Device

If you have more than one device in DFU mode connected to your computer then find the 32blit using `lsusb` and add `-d vid:pid` to the dfu-util command. Replace `vid:pid` with the 4 character ID strings to target the correct device.
