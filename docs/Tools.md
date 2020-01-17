# Tools

## Sprite Builder

The `sprite-builder` tool is intended to be a tool for packing 32blit images into byte-arrays, includes, or data files for use in code.

The script outputs to a C++ byte-array and includes a header with information about the image size and palette.

Have a look into the script for further details regarding the formats.

### Prerequisites:

``` shell
sudo python3 -m pip install construct bitarray bitstring
```

### Usage:

Currently `sprite-builder` knows two types of conversion:

- Packed paletted images, and
- Raw images like RGBA, RGB888 or RGB565

Both conversions output data to stdout in C include style only. Output should be redirected into a file for usage (or copy-pasted from the terminal, if you wish).

#### Packed

A packed sprite can have a variable number of palette colours up to 255.

Data is packed at the number of bits-per-pixel required to index each palette entry.

IE: a two colour image will be packed with 1bpp

If you start with the 32blit, start with this format.

``` shell
./sprite-builder packed input-file.png
```

#### Raw

An unpacked sprite is just a block of raw surface data
in either RGBA (4 bytes), RGB888 (3 bytes) or RGB565 (2 bytes) format.

Unpacked sprites are *huge* but useful for streaming into the framebuffer from storage.

The max power of two sprite sizes supported by this file format are-

- 64x64 in RGBA and RGB888 mode
- 128x128 in RGB565 mode

``` shell
./sprite-builder raw --format RGB565 input-file.png
```
