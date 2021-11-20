# Cursor Theme Packaging (.deb)
This directory contains the cursor theme packaging script for Debian and derivatives (e.g. Ubuntu).

## Usage
Simply run the script, and pass in the path of the theme(s) you would like to build, relative to the `/cursors` directory.

For example:
```bash
./packcurs.sh no-shadow/standard shadow/standard
```

This will build and output packages for the standard cursor theme with, and without, pointer shadows. You can then install the package(s) with:
```bash
sudo dpkg -i <cursor-theme-xxx-yyy.deb>
```

## Any Problems?
If you have problems running the script, feel free to [submit an issue](https://github.com/rozniak/xfce-winxp-tc/issues) and include the generated log file. 😁
