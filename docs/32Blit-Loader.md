# 32Blit Loader Tool

The 32Blit loader is a tool for saving 32Blit games to your SD card over USB, or flashing them directly to the 32Blit's external flash for quick and easy testing.

## Building The 32Blit Loader Tool

### macOS & Linux

From the root of the repository you must:

```
mkdir build
cmake ..
make 32Blit
```

This will build the loader tool at `tools/src/32Blit`.

You should ideally add this directory to your `PATH` or alternatively copy this utility somewhere your system can find it.

>Additional info:  
> - A guide on adding a directory to your PATH  
>https://www.architectryan.com/2012/10/02/add-to-the-path-on-mac-os-x-mountain-lion/
>
> - To just move the file to a directory that is already in PATH, run (from the build directory you created above):  
>`sudo cp tools/src/32Blit /usr/local/bin` (for example)

### Windows - Visual Studio

You can build the tool with the Visual Studio solution (`vs/32blit.sln`). It will be built to `vs/x64/Debug/32Blit.exe` (or similar, depending on build configuration).

You should ideally add this directory to your `PATH` or alternatively copy this utility somewhere your system can find it.

### Windows - WSL

From the root of the repository you must:

```
mkdir build.mingw
cmake .. -DCMAKE_TOOLCHAIN_FILE=../mingw.toolchain
make 32Blit
```

This will build the loader tool at `tools/src/32Blit.exe`

Since WSL can run this program directly you should ideally add this directory to your `PATH` or alternatively copy this utility somewhere your system can find it.

## Usage

Usage: `32blit <process> <comport> <binfile>`

* `process` - Either `_RST`, `SAVE` or `PROG`
* `comport` - `AUTO` for automatic selection, otherwise `COMX` for Windows or the relevant device in `/dev/serial/by-id/` for Linux
* `binfile` - path to the binary file you want to flash or program

Options:

* `--reconnect` - Reconnect after `PROG` to show debug output

You can run `32Blit.exe` (Windows) or `32Blit` (Linux) manually to copy a bin file to either QSPI Flash or the SD Card on your device.

For example to program to QSPI Flash from Windows:

```
32Blit.exe PROG COM8 examples/raycaster/raycaster.bin
```

Or copy to the inserted SD card:

```
32Blit.exe SAVE COM8 examples/raycaster/raycaster.bin
```