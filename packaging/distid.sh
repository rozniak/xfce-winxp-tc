#!/usr/bin/env bash

#
# distid.sh - Identify Distro
#
# This source-code is part of Windows XP stuff for XFCE:
# <<https://www.oddmatics.uk>>
#
# Author(s): Rory Fewell <roryf@oddmatics.uk>
#

#
# MAIN SCRIPT
#

# Probe for package managers to try and determine what distro we're
# on
#
# NOTE: Since #253, Debian/dpkg is checked last, because potentially users of
#       other distros might have dpkg installed which throws off detection
#
#       I think it's unlikely the other package managers will be installed on
#       different distros... mainly just dpkg
#

# Check Arch Linux
#
which pacman >/dev/null 2>&1

if [[ $? -eq 0 ]]
then
    export DIST_ID="archpkg"
    export DIST_ID_EXT="std"
    return 0
fi

# Check Alpine Linux
#
which apk >/dev/null 2>&1

if [[ $? -eq 0 ]]
then
    export DIST_ID="apk"
    export DIST_ID_EXT="std"
    return 0
fi

# Check FreeBSD
#
which pkg >/dev/null 2>&1

if [[ $? -eq 0 ]]
then
    export DIST_ID="bsdpkg"
    export DIST_ID_EXT="std"
    return 0
fi

# Check Red Hat
#
which rpm >/dev/null 2>&1

if [[ $? -eq 0 ]]
then
    export DIST_ID="rpm"
    export DIST_ID_EXT="std"
    return 0
fi

# Check Void Linux
#
which xbps-create >/dev/null 2>&1

if [[ $? -eq 0 ]]
then
    export DIST_ID="xbps"

    # This might be a rubbish way to determine glibc vs. musl, if it does suck
    # then someone needs to whinge and then I'll have to come up with something
    # better
    #
    find /usr/lib -iname "*ld-musl*" | read

    if [[ $? -eq 0 ]]
    then
        export DIST_ID_EXT="musl"
    else
        export DIST_ID_EXT="glibc"
    fi

    return 0
fi

# Check Debian
#
which dpkg >/dev/null 2>&1

if [[ $? -eq 0 ]]
then
    export DIST_ID="deb"
    export DIST_ID_EXT="std"
    return 0
fi

# Nothing else to probe, it's over!
#
echo "Unsupported distribution."
return 1
