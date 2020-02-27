# Tools

The individual asset tools have been deprecated in favour of https://github.com/pimoroni/32blit-tools which installs `32blit` or `32blit.exe` command on Linux/Mac or Windows respectively.

Head on over to https://github.com/pimoroni/32blit-tools for documentation covering the new tools.

# Asset Pipeline

The new `32blit` tool has been integrated into CMake so that you can turn your raw images and assets into packed data at compile time.

To use this new setup your project must have an `assets.yml` and you should reference it in your `CMakeLists.txt` right after `blit_executable` like so:

```cmake
blit_assets_yaml (project-name assets.yml)
```

Substituting `project-name` for the first argument given to `blit_executable`.

Assuming you assets are in a directory called `assets/`, a simple `assets.yml` might look like this:

```yml
assets.cpp:
    assets/sprites.png:
        name: sprites_data
    assets/level.tmx:
        name: level_data
```

When you configure and compile your project, this will add an `assets.cpp` to your project's sources and output an `assets.hpp` file which you can include into your code to reference your assets:

```c++
#include "assets.hpp"
```

This file will include `sprites_data`, `sprites_data_length`, `level_data`, and `level_data_length`.

You can load sprites like so:

```c++
screen.sprites = SpriteSheet::load(asset_sprites);
```

For your level data you might want to create an in-memory copy of it that you can edit to change your level in game:

```c++
// Store your level width/height in tiles
constexpr uint16_t level_width = 64;
constexpr uint16_t level_height = 64;

// Allocate memory for the writeable copy of the level
uint8_t *local_level_data = (uint8_t *)malloc(level_width * level_height);

// Copy read-only level data into writeable copy
for(auto x = 0; x < level_width * level_height; x++){
  local_level_data[x] = level_data[x];
}

// Load our level data into the TileMap
level = new TileMap((uint8_t *)local_level_data, nullptr, Size(level_width, level_height), screen.sprites);
```

# Old Tools

## Sprite Builder

The `sprite-builder` tool is intended to be a tool for packing 32blit images into byte-arrays, includes, or data files for use in code.

The script outputs to a C++ byte-array and includes a header with information about the image size and palette.

Have a look into the script for further details regarding the formats.

### Prerequisites:

``` shell
sudo python3 -m pip install construct bitarray bitstring pillow
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
