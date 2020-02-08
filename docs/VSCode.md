# Setting up Visual Studio Code

## Basic setup
You'll need to install:
 - [Visual Studio Code](https://code.visualstudio.com/)
 - The [C/C++ extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)
 - The [CMake Tools extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)

Then open the cloned repository with "Open folder...". You should get a notification asking if you want to configure the project. Click "Yes" and select "[Unspecified]" from the "Select a Kit" dropdown for a local build with the default compiler.

You should now be able to build by pressing the "⚙ Build:" button, or `F7`. You can also run an example by pressing `Shift` + `F5` and debug by pressing the "Debug" button or `Ctrl` + `F5`.

## IntelliSense

After configuring the project a "CMake Tools would like to configure IntelliSense for this folder." notification should appear, click "Allow" to configure IntelliSense. If the notification does not appear, open the command palette (`Ctrl`/`Cmd` + `Shift` + `P`) run "C/C++: Change Configuration Provider..." and select "CMake Tools".

## Debugger configuration

It should be possible to debug without setting up a `launch.json` file but if you need to use one, you should set `program` to `${command:cmake.launchTargetPath}`. See the [CMake Tools documentation](https://vector-of-bool.github.io/docs/vscode-cmake-tools/debugging.html#debugging-with-cmake-tools-and-launch-json) for more details.

## Building for 32Blit

Open the command palette (`Ctrl`/`Cmd` + `Shift` + `P`) and run "CMake: Edit User-Local CMake Kits".

Add this to the list:
```json
  {
    "name": "32Blit",
    "toolchainFile": "/path/to/32blit-beta/32blit.toolchain"
  },
```
(Replacing `/path/to/32blit-beta`, with the actual path.)

You should now be able to select "32Blit" as a kit. ("CMake: Change Kit" from the command palette or the button displaying the current kit at the bottom of the window). If you select a target ending with .flash from the list next to the "⚙ Build:" button, that example will be flashed to your device when you build.

## CMake Arguments

To set CMake arguments (like `-D32BLIT_PATH` for out-of-tree builds), you need to add them to `.vscode/settings.json`:

```json
{
  // other options...
  "cmake.configureSettings": {
    "32BLIT_PATH": "/path/to/32blit-beta"
  },
}
```