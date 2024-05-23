# Packaging
This directory contains the source files for packaging components.

## Structure
The contents of this directory is as follows:

`cmake-inc/` - common CMake scripts to be included by components in their builds

`apk/` - Alpine Linux (`.apk`) packaging implementation

`archpkg/` - Arch Linux (`.pkg.tar.zst`) packaging implementation

`bsdpkg/` - FreeBSD (`.pkg`) packaging implementation

`deb/` - Debian (`.deb`) packaging implementation

`rpm/` - Red Hat (`.rpm`) packaging implementation

`xbps/` - Void Linux (`.xbps`) packaging implementation

`build.sh` - build script for compiling a single component

`buildall.sh` - build script for compiling and packaging all components in the project

`chkdeps.sh` - script for checking dependencies for building components are installed

`distid.sh` - helper script for identifying the distribution in use

`package.sh` - script for packaging a single component (that has been built with `build.sh`)

`targets` - default build list for all components in the project

## TL;DR on Building
If you are new and simply want to compile everything, you should:
- Ensure you have the typical tools installed: `bash`, `cmake`, `coreutils`, `fakeroot`, `gcc`, `make`, `pkg-config`, `python3`
- Run `./chkdeps.sh -l` - this will inform you if you're missing any build-time dependencies
  - This script should output the package names, so you can bung it straight into your package manager, eg. `apt install $(./chkdeps.sh -l | cut -d':' -f2 | tr '\n' ' ')`
- Run `./buildall.sh` - this should build the entire project by default under the `xpclient-pro` SKU, outputting in `./xptc`

This is intended to *Just Work(TM)*, if you have any problems, please raise an issue!

## Support?
Supported distros/package formats:
- Alpine Linux (`.apk`)
- Arch Linux (`.pkg.tar.zst`)
- Debian (`.deb`)
- FreeBSD (`.pkg`)
- Red Hat (`.rpm`)
- Void Linux (`.xbps`)

Please check under the `packaging` tag to see if there is an open issue for your favourite distro/package format. You can help speed up the process by providing a simple example package script or TL;DR so that the porting process is easier (I'm not familiar with every distro out there!)
