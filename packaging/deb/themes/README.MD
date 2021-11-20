# Desktop Theme Packaging (.deb)
This directory contains the desktop theme packaging script for Debian and derivatives (e.g. Ubuntu).

## Usage
Simply run the script. At the moment it will specifically build the Luna (Blue) theme.
```bash
./packthem.sh
```

This will build and output the Luna (Blue) theme. You can then install the package with:
```bash
sudo dpkg -i xfce-theme-luna-blue.deb
```

## Any Problems?
The theme is still a WIP, and a lot of work is due (along with other themes). It's usable though.

If you come across a bug, [check the issues](https://github.com/rozniak/xfce-winxp-tc/issues), and if your particular problem is not there then file a new issue. 😁
