# Video Capture

`32blit-sdl` supports optional FFMPEG video capture, which you can enable by grabbing a FFMPEG snapshot, building it, and turning on `ENABLE_FFMPEG` when building your project.


1. `sudo apt install liblzma-dev`
2. `wget https://github.com/FFmpeg/FFmpeg/archive/n4.1.4.zip`
3. `unzip n4.1.4.zip`
4. `cd FFmpeg-n4.1.4`
5. `./configure --prefix=$(pwd)/build`
6. `make && make install`

Then configure your 32blit project with:

```
mkdir build
cd build
cmake .. -DVIDEO_CAPTURE=true
```

When running your game, you can now hit `r` to start and stop recording.
