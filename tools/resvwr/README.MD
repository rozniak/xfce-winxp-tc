# Resource Viewer
This directory contains the source code for the *Resource Viewer* tool.

![image](https://github.com/user-attachments/assets/a2c67868-29d2-4f4d-9b47-fa7b6723307f)

## Installation
This tool must be compiled before it can be used. Build, package, and install it the same way you would with any other program in this repo, eg:
```
(in the /packaging dir)
./build.sh tools/resvwr
./package.sh tools/resvwr
sudo dpkg -i local-out/wintc-resvwr.deb
```

Adjust the `dpkg` command to whatever the package manager is on your distro (eg. for Fedora do `sudo dnf install ./local-out/wintc-resvwr.rpm`)

# Usage
The tool allows you to view most of the GTK builder XML resources in this project. Some of course require custom widgets which may not be available.

It's a viewer only, to aid with development. It is not a fully featured editor like Glade.
