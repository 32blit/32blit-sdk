# Tools <!-- omit in toc -->

The individual asset tools have been deprecated in favour of https://github.com/pimoroni/32blit-tools which installs `32blit` or `32blit.exe` command on Linux/Mac or Windows respectively.

You can install these tools with `pip`:

```
pip3 install 32blit
```

Head on over to https://github.com/pimoroni/32blit-tools for further documentation covering the installation of the new tools.

- [Asset Pipeline](#asset-pipeline)
  - [Additional Options](#additional-options)
- [Visual Studio](#visual-studio)

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
screen.sprites = Surface::load(sprites_data);
```

For your level data you might want to create an in-memory copy of it that you can edit to change your level in game:

```c++
// Store your level width/height in tiles
constexpr uint16_t level_width = 64;
constexpr uint16_t level_height = 64;

// Allocate memory for the writeable copy of the level
auto local_level_data = new uint8_t[level_width * level_height]

// Copy read-only level data into writeable copy
memcpy(local_level_data, level_data, level_width * level_height);

// Load our level data into the TileMap
level = new TileMap(local_level_data, nullptr, Size(level_width, level_height), screen.sprites);
```

## Additional Options
```yaml
assets.cpp:                         # Output filename, can also be a .hpp
    assets/sprites.png:
        name: sprites_data          # Name of variable generated for this asset, if omitted is generated from the
                                    # file name

        type: image/image           # Type of this asset, usually auto-detected from the extension

        palette: assets/palette.png # Optional palette for the image. Supports .act, .pal, .gpl and any image file

        transparent: 255,0,255      # Optional colour to map to transparency

        packed: yes                 # Pack the image into the minimum number of bits, for example an image with
                                    # two colours would use one bit per pixel. Set to false to always use eight
                                    # bits per pixel

        strict: true                # Allow only the colours defined in the palette, if false (the default)
                                    # colours are automatically added to the palette

    # If you do not need to specify any options other than the name, you can use
    assets/sprites.png: sprites_data

    # Parses a Tiled map
    assets/level.tmx:
        name: level_data
        type: map/tiled
    
    # Embeds a file without any processing
    assets/raw.bin:
        name: raw_bin
        type: raw/binary
    
    # Parses a CSV file into an array of bytes
    assets/raw.csv:
        name: csv_data
        type: raw/csv
     
    # Generates a font from an image
    # It can be used as `const Font font(asset_font)`
    assets/font.png:
        name: asset_font

        type: font/image            # This is required here as it would be auto-detected as an image asset

        height: 8                   # The height of the font, should match the image file if specified

        horizontal_spacing: 1       # Additional space between characters for variable-width mode (defaults to 1)

        vertical_spacing: 1         # Space between lines (defaults to 1)

        space_width: 4              # Width of the space character (defaults to 1)

    # Generates a font from a font file, can use any format supported by freetype
    assets/Comic_Sans_MS.ttf:
        name: asset_comic_font

        type: font/font

        height: 16                  # The height of the font to request, the resulting font's height may be
                                    # slightly different

        vertical_spacing: 1         # Space between lines (defaults to 1)

# It is also possible to define multiple outputs
more-assets.hpp:
    assets/extra_sprites.png: extra_sprites

```

# Visual Studio
To use `assets.yml` with a Visual Studio project, you need to run the packer as a pre-build step. Make sure that the output path is in the project's include path.
```
python -m ttblit pack --force --config $(ProjectDir)assets.yml --output $(ProjectDir)
```
To add the build step right click the project and click properties. From there go to build events and paste the command into the command line text box.

If the step is failing to complete then make sure that both python and the 32blit pip package is installed. 

After the build completes you will want to add the two files it generates to the project. (assets.cpp and assets.hpp).
Right click the project go to add then existing item and choose the two files from the project directory.

Failing to do this may cause errors when trying to debug similar to the ones below.

`LNK2001	unresolved external symbol "unsigned char const * const sprites_data" (?sprites_data@@3QBEB)`

`LNK1120	1 unresolved externals`
