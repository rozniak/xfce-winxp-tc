# libxpsystray-plugin
This directory contains the source code for the Notification Area XFCE panel plugin.

![image](https://user-images.githubusercontent.com/13258281/141838891-0b94b957-c745-48cf-8c2c-1b06abfc1122.png)

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
- Put the compiled `libxpsystray-plugin.so` in `/usr/lib/x86_64-linux-gnu/xfce4/panel/plugins` (adjust for your architecture)
- Put `xpsystray.desktop` in `/usr/share/xfce4/panel/plugins`

Of course, with subsequent builds you will need to replace `libxpsystray-plugin.so` in the directory specified above, if you're developing.
