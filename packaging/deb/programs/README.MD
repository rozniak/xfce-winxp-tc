# Program Packaging (.deb)
This directory contains the program packaging script for Debian and derivatives (e.g. Ubuntu).

## Usage
Simply run the script and pass in the name of the program you would like to build, relative to the repository's root.

For example:
```bash
./packprog.sh shell/run shell/winver
```

This will build and output packages for the `run` and `winver` programs. You can then install the package(s) with:
```bash
sudo dpkg -i <xxx-yyy.deb>
```

## Any Problems?
You may encounter issues if you have not already installed dependencies for the progams you are building. A log file is generated as part of the script's build process, review it, and see if you are missing dependencies.

If you have any other problems, feel free to [submit an issue](https://github.com/rozniak/xfce-winxp-tc/issues) and include the generated log file. 😁
