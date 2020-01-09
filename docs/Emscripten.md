# Building & Running For Web (Emscripten)

Building in Emscripten - asm.js/WebAssembly - works on Linux and WSL.

You will need to install the emscripten compiler. See https://emscripten.org/docs/getting_started/downloads.html for complete instructions, but generally you should just browse to your desired directory and:

```
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

You should now have commands like `emcc` in your path, and an `EMSDK` environment variable set ready to build your project.

In your project directory:

``` shell
mkdir build.em
cmake .. -DCMAKE_TOOLCHAIN_FILE=$EMSDK/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake -G "Unix Makefiles"
make
python3 -m http.server
```
Finally, open the URL given by Python's HTTP server in your browser and open your project's .html file.
