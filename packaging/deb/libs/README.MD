# Shared Library Packaging (.deb)
This directory contains the shared library packaging script for Debian and derivatives (e.g. Ubuntu).

## Usage
Simply run the script, and pass in the name of the library/libraries you would like to build.

For example:
```bash
./packlibs.sh comgtk
```

This will build and output package for `libwintc-comgtk`. You can then install the package with:
```bash
sudo dpkg -i <libname.deb>
```

## Any Problems?
You may encounter issues if you have not already installed dependencies for the library/libraries you are building. A log file is generated as part of the script's build process, review it, and see if you are missing dependencies.

Also note that libraries in this directory may depend on each other - an example is `libwintc-exec` depends on `libwintc-comgtk`, so the latter must be packaged and installed first. If this problem is the cause of build errors, it should be obvious if the error points to an include of headers from one of the libraries present in here, such as this message:
```
~/xfce-winxp-tc/shared/exec/src/exec.c:5:10: fatal error: wintc-comgtk.h: No such file or directory
    5 | #include <wintc-comgtk.h>
      |          ^~~~~~~~~~~~~~~~
```

If you have any other problems, feel free to [submit an issue](https://github.com/rozniak/xfce-winxp-tc/issues) and include the generated log file. 😁
