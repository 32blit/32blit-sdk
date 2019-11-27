# Tools

## Asset Builder

The `asset-builder` tool is intended to be a tool for packing 32blit assets into byte-arrays, includes, or data files for use in code.

Presently it supports only sprite packing, into the format requires for the current iteration of the 32blit API. This outputs to a C++ byte-array and includes a header with information about the sprite size and palette.

### Prerequisites:

```
sudo python3 -m pip install construct bitarray bitstring
```

### Usage:

Currently `asset-builder` outputs packaged data to stdout in C++ include style only. Output should be redirected into a file for usage (or copy-pasted from the terminal, if you wish):

```
./asset-builder sprite input-file.png > my-sprite.hpp
```