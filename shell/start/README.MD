# libstartmenu-plugin
This directory contains the source code for the Start menu XFCE panel plugin.

![image](https://user-images.githubusercontent.com/13258281/141834450-cc629c05-7d97-45c9-aa0d-ff20d1c69880.png)

## Installation
See the `README.MD` under `/packaging/deb/panel` for packaging and installation.

## Building
Standard CMake build for the plugin:
- `mkdir build`
- `cd build`
- `cmake ..`
  - (if you're missing libraries they will be mentioned here)
- `make`

Then you must put the files in the right places in the filesystem:
- Put the compiled `libstartmenu-plugin.so` in `/usr/lib/x86_64-linux-gnu/xfce4/panel/plugins` (adjust for your architecture)
- Put `startmenu.desktop` in `/usr/share/xfce4/panel/plugins`
- Copy files under `/res/*` to `/usr/share/winxp/shell-res` (you may need to create this directory)

Of course, with subsequent builds you will need to replace `libstartmenu.so` in the directory specified above, if you're developing.

## Keyboard Shortcut
At the moment there is no ability to open the Start menu with the `super`/`Win` key - there is an issue open for this however: [#86](https://github.com/rozniak/xfce-winxp-tc/issues/86)
