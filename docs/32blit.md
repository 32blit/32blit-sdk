# Building & Running On 32blit

To build your project for 32blit using arm-none-eabi-gcc you should prepare the Makefile with CMake using the provided toolchain file.

From the root of your project:

```
cmake . -B build.stm32 -DCMAKE_TOOLCHAIN_FILE=../../32blit.toolchain
make -C build.stm32
```

The result of your build will be a `.bin`, `.hex`, `.elf` and DfuSe-compatible `.dfu` file.

## Troubleshooting

If you see `cannot create target because another target with the same name already exists` you've probably run `cmake ..` in the wrong directory (the project directory rather than the build directory), you should remove all but your project files and `cmake ..` again from the build directory.