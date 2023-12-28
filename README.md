# Smart Motor Control Project

Smart Motor Control Project is a control system assignment.

# Install

Clone this repository and run:
```
make
```
to compile, the binary file will create in `bin/` directory.

Because this use serial port, you may want to use root user to run this (or add user to dialout groups? this not work for me). Then you possibly want to use `xhost local:root` to allow root user using X client.

# Dependencies

This project use Linux serial port for communicating motor, so this is only works on Linux or possibly other Linux like machine.

Using [ocornut/imgui: Dear ImGui](https://github.com/ocornut/imgui) for GUI. Make sure you have `glfw` installed. In Arch Linux, the package is `glfw-x11`. The imgui repo may not come with `git clone` command, in that case, you will manually clone the imgui repo in `lib/` directory.

# Hardware

The motor we use is [Smart Motor SM23165D](https://www.animatics.com/products/smartmotor/class-5-d-style-smartmotor/sm23165d.html) (I believe...).
