# Font Packaging
This directory contains the font packaging script for Debian and derivatives (e.g. Ubuntu).

## Usage
Simply run the script itself - it requires no arguments as the system fonts are the only thing to package.
```
./packfnts.sh
```

This will build and output the package for the fonts, which can then be installed with:
```
sudo dpkg -i wintc-fonts-xp.deb
```

## Any Problems?
If you have problems running the script, feel free to [submit an issue](https://github.com/rozniak/xfce-winxp-tc/issues) and include the generated log file. 😁
