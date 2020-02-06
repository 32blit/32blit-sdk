# Setting up Visual Studio Code

## Basic setup
You'll need to install:
 - [Visual Studio Code](https://code.visualstudio.com/)
 - The [C/C++ extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)
 - The [CMake Tools extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)

Then open the cloned repository with "Open folder...".

## Intellisense

Open the command palette (`Ctrl` + `Shift` + `P`) run "C/C++: Change Configuration Provider..." and select "CMake Tools".

## Building for 32Blit

Open the command palette (`Ctrl` + `Shift` + `P`) and run "CMake: Edit User-Local CMake Kits".

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
  "cmake.configureArgs": [
    "-D32BLIT_PATH=/path/to/32blit-beta"
  ],
}
```