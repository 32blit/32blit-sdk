### Building & Running on OSX

You will need build tools and CMAKE. Assuming you have [homebrew](https://docs.brew.sh/Installation) installed:

``` shell
xcode-select --install
brew install cmake
```

You'll also need to build and install SDL2:

``` shell
wget https://www.libsdl.org/release/SDL2-2.0.10.zip
unzip SDL2-2.0.10.zip
cd SDL2-2.0.10
mkdir build
cd build
../configure
make
sudo make install
```

Finally, from the root directory of this repository:

``` shell
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../darwin.toolchain
make
```

When the build completes you should find all the examples within the build directory.
