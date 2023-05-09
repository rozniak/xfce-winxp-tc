# Debian Packaging

This directory contains the packaging script for Debian and derivatives (e.g. Ubuntu).

## Usage
Simply run the script, and pass in the path of the component you would like to build relative to the repo root.

For example:
```bash
./bldnpkg.sh cursors/no-shadow/standard icons/luna themes/professional
```

This will build and output packages for the above components. You can then install the packages with:
```bash
sudo dpkg -i <package>
```

## Any Problems?
Some packages require dependencies, so you may encounter build failures if you do not have them installed. Some dependencies refer to other components inside this repo.

For instance if you attempt to build `libwintc-exec` via `./bldnpkg.sh shared/exec` you will get a build failure if you have not yet installed `libwintc-comgtk` first.

If you have any other problems, feel free to [submit an issue](https://github.com/rozniak/xfce-winxp-tc/issues) with error output details. :grin:
