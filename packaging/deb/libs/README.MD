# Shared Library Packaging (.deb)
This directory contains the shared library packaging script for Debian and derivatives (e.g. Ubuntu).

## Usage
Simply run the script, and pass in the name of the library/libraries you would like to build.

For example:
```bash
./packlibs.sh comgtk exec
```

This will build and output packages for `libwintc-comgtk` and `libwintc-exec`. You can then install the package(s) with:
```bash
sudo dpkg -i <libname.deb>
```

## Any Problems?
You may encounter issues if you have not already installed dependencies for the library/libraries you are building. A log file is generated as part of the script's build process, review it, and see if you are missing dependencies.

If you have any other problems, feel free to [submit an issue](https://github.com/rozniak/xfce-winxp-tc/issues) and include the generated log file. 😁
