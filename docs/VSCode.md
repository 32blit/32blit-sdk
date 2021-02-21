# Setting up Visual Studio Code <!-- omit in toc -->

These instructions cover setting up Visual Studio Code to build the CMake project.

A working knowledge of using and configuring Visual Studio Code is assumed.

- [Requirements](#requirements)
- [Initial setup for local builds](#initial-setup-for-local-builds)
- [IntelliSense](#intellisense)
- [CMake Arguments](#cmake-arguments)
- [Debugger configuration](#debugger-configuration)
- [Building for 32Blit](#building-for-32blit)
- [Troubleshooting](#troubleshooting)
  - [Debugging on macOS](#debugging-on-macos)
  - ["Unable to determine what CMake generator to use." on Windows](#unable-to-determine-what-cmake-generator-to-use-on-windows)
  - [`Shift` + `F5` to run not working when using a Visual Studio kit](#shift--f5-to-run-not-working-when-using-a-visual-studio-kit)

## Requirements

You'll need to install:

 - [Visual Studio Code](https://code.visualstudio.com/)
 - The [C/C++ extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)
 - The [CMake Tools extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)

Then open the cloned repository with "Open folder...".

## Initial setup for local builds
You should get a notification asking if you want to configure the project. Click "Yes" and select "[Unspecified]" from the "Select a Kit" dropdown for a local build with the default compiler.

You should now be able to build by pressing the "⚙ Build:" button, or `F7`. You can also run an example by pressing `Shift` + `F5` and debug by pressing the "Debug" button or `Ctrl` + `F5`.

## IntelliSense

After configuring the project a "CMake Tools would like to configure IntelliSense for this folder." notification should appear, click "Allow" to configure IntelliSense. If the notification does not appear, open the command palette (`Ctrl`/`Cmd` + `Shift` + `P`) run "C/C++: Change Configuration Provider..." and select "CMake Tools".

## CMake Arguments

To set CMake arguments (like `-D32BLIT_DIR` for out-of-tree builds), you need to add them to `.vscode/settings.json`:

```json
{
  // other options...
  "cmake.configureSettings": {
    "32BLIT_DIR": "/path/to/32blit-sdk"
  },
}
```

## Debugger configuration

You will need to create a `launch.json` file for debugging on some platforms, you should set `program` to `${command:cmake.launchTargetPath}` and `cwd` to `${workspaceFolder}/build`. See the [CMake Tools documentation](https://vector-of-bool.github.io/docs/vscode-cmake-tools/debugging.html#debugging-with-cmake-tools-and-launch-json) for more details.

## Building for 32Blit

Open the command palette (`Ctrl`/`Cmd` + `Shift` + `P`) and run "CMake: Edit User-Local CMake Kits".

Add this to the list:
```json
  {
    "name": "32Blit",
    "toolchainFile": "/path/to/32blit-sdk/32blit.toolchain"
  },
```
(Replacing `/path/to/32blit-sdk`, with the actual path.)

You should now be able to select "32Blit" as a kit. ("CMake: Change Kit" from the command palette or the button displaying the current kit at the bottom of the window). If you select a target ending with .flash from the list next to the "⚙ Build:" button, that example will be flashed to your device when you build.

## Troubleshooting

### Debugging on macOS
If you are running Catalina or higher, you may find difficulty in debugging local builds. To fix this, you need to install the [CodeLLDB](https://marketplace.visualstudio.com/items?itemName=vadimcn.vscode-lldb) extension. Add a 'Build and attach' configuration like so:

- Add a configuration with Debug > Add Configuration
- Paste in the following

``` json
{
   "name": "Launch and Debug",
   "type": "lldb",
   "request": "launch",
   "program": "${command:cmake.launchTargetPath}"
}
```
Now, when you want to attach the debugger, run with that configuration and now your breakpoints will be respected 🎉

### "Unable to determine what CMake generator to use." on Windows
You may need to run `code` from the VS Developer Command Prompt for CMake Tools to be able to find all of the required build tools.

### `Shift` + `F5` to run not working when using a Visual Studio kit
The CMake Visual studio generators may result in the SDL2 DLLs getting copied to the wrong place (DLLs in `build/`, exe in `build/Debug/`). You can work around this by adding `"cmake.generator": "NMake Makefiles"` to your settings.json to force the NMake generator instead.
