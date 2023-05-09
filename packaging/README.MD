# Packaging
This directory contains the source files for packaging components.

## Structure
The contents of this directory is as follows:

`cmake-inc/` - common CMake scripts to be included by components in their builds

`deb/` - Debian (`.deb`) packaging

## Support?
At the moment you'll notice there is only a `deb` subdirectory. I personally use Debian, so I have got Debian packaging functional - if you're also a Debian user, or you use a derivative such as Ubuntu, then you should be good to go.

For users on Linux distributions that use other package managers, I will get around to supporting those one day, it just takes a lot of work and a lot of testing. This `README` will be updated to reflect the status of package manager support when the work is done.
