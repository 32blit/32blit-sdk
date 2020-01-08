### Building & Running on Linux or WSL + XMing

To build your project for testing, go into the relevant example directory. We'll use `palette-cycle` to demonstrate:

```
cd examples/palette-cycle
```

prepare the Makefile with CMake:

```
mkdir build
cd build
cmake ..
```

and compile the example:

```
make
```

To run the application on your computer, use the following command (from within the same directory):

```
./palette-cycle
```

\### XMing

To run the examples from WSL on Windows you will need to have XMing (or another XWindow Server) running on Windows. Click on the following link which will help you install and setup WSL and XMing together.

- [Information how to run XMing with WSL](https://virtualizationreview.com/articles/2017/02/08/graphical-programs-on-windows-subsystem-on-linux.aspx)
- [Download XMing setup](https://sourceforge.net/projects/xming/files/Xming/6.9.0.31/Xming-6-9-0-31-setup.exe/download)

You can then run code by either:
- prefixing the command with `DISPLAY=:0.0`, or 
- execute the command `export DISPLAY=:0.0` - which after you will not need to prefix the commands in the current session, just run them.

eg:

```
DISPLAY=:0.0 ./palette-cycle
```
