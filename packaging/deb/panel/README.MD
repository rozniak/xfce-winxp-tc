# XFCE Panel Plugin Packaging (.deb)
This directory contains the XFCE panel plugin packaging script for Debian and derivatives (e.g. Ubuntu).

## Usage
Simply run the script, and pass in the name of the panel plugin you would like to build, relative to the repository's root.

For example:
```bash
./packplug.sh shell/start shell/systray
```

This will build and output packages for the Start menu and notification area panel plugins. You can then install the package(s) with:
```bash
sudo dpkg -i <xxx-yyy-plugin.deb>
```

## Any Problems?
You may encounter issues if you have not already installed dependencies for the panel plugins you are building. A log file is generated as part of the script's build process, review it, and see if you are missing dependencies.

If you have any other problems, feel free to [submit an issue](https://github.com/rozniak/xfce-winxp-tc/issues) and include the generated log file. 😁
