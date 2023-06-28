# Packaging
This directory contains the source files for packaging components.

## Structure
The contents of this directory is as follows:

`cmake-inc/` - common CMake scripts to be included by components in their builds

`deb/` - Debian (`.deb`) packaging implementation

`build.sh` - build script for compiling a single component

`buildall.sh` - build script for compiling all components in the project

`chkdeps.sh` - script for checking dependencies for building components are installed

`distid.sh` - helper script for identifying the distribution in use

`targets` - default build list for all components in the project

## TL;DR on Building
If you are new and simply want to compile everything, you should:
- Ensure you have the typical tools installed: `cmake`, `fakeroot`, `gcc`, `make`, `python3`
- Run `./chkdeps.sh -l` - this will inform you if you're missing any build-time dependencies
  - This script should output the package names, so you can bung it straight into your package manager, eg. `apt install $(./chkdeps.sh -l | cut -d':' -f2 | tr '\n' ' ')`
- Run `./buildall.sh` - this should build the entire project by default under the `xpclient-pro` SKU, outputting in `./xptc`

This is intended to *Just Work(TM)*, if you have any problems, please raise an issue!

## Support?
Supported distros/package formats:
- Debian (`.deb`)

Please check under the `packaging` tag to see if there is an open issue for your favourite distro/package format. You can help speed up the process by providing a simple example package script or TL;DR so that the porting process is easier (I'm not familiar with every distro out there!)
