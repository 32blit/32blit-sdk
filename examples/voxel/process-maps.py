#!/usr/bin/env python3

# processes a colour map and a height map into the format needed
# for the voxel demo to stream map data on the fly from the 
# SD card.
#
# colour map: should be a 256 colour indexed image (bmp, png, etc)
# height map: should be a greyscale image where "white" is higher
#
# both images must have the same dimensions which must be powers
# of two (for example 1024x1024, 256x256, etc).
#
# the images are broken up into 32x32 chunks and the chunks are stored
# in row-column order with height data and colour indexes interleaved. 
# this allows a single chunk to be read from the file with a seek/read pair.
#
# the colour palette is also saved out as a continuous run of r, g, b
# byte triplets at the start of the file.
#
# the resulting file has the following format:
#
# colour palette information (768 bytes)
# chunk #1 (2048 bytes)
# chunk #2 (2048 bytes)
# ...
# chunk #n (2048 bytes)
 
from PIL import Image
import sys
import os.path

colour_map_filename = sys.argv[1]
height_map_filename = sys.argv[2]

colour_map_image = Image.open(colour_map_filename)
height_map_image = Image.open(height_map_filename)

width, height = colour_map_image.size

chunks = int(width / 32)

data = []

palette = colour_map_image.getpalette()
for i in range(0, 256):
  data.append(palette[i * 3 + 0])
  data.append(palette[i * 3 + 1])
  data.append(palette[i * 3 + 2])

for cy in range(0, chunks):
  for cx in range(0, chunks):
    for py in range(0, 32):
      for px in range(0, 32):
        data.append(height_map_image.getpixel((px + cx * 32, py + cy * 32)))
        data.append(colour_map_image.getpixel((px + cx * 32, py + cy * 32)))        

map_file = open("terrain.map", "wb")
map_file.write(bytearray(data))
