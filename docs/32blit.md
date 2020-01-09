# Building & Running On 32blit

To build your project for 32blit using arm-none-eabi-gcc you should prepare the Makefile with CMake using the provided toolchain file.

From the root of your project:

```
mkdir build.stm32
cd build.stm32
cmake .. -DCMAKE_TOOLCHAIN_FILE=../../../32blit.toolchain
```

And then `make` as normal.

The result of your build will be a `.bin`, `.hex`, `.elf` and DfuSe-compatible `.dfu` file.

## Troubleshooting

If you see `cannot create target because another target with the same name already exists` you've probably run `cmake ..` in the wrong directory (the project directory rather than the build directory), you should remove all but your project files and `cmake ..` again from the build directory.