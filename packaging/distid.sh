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
    echo -n "archpkg"
    exit 0
fi

# Check Alpine Linux
#
which apk >/dev/null 2>&1

if [[ $? -eq 0 ]]
then
    echo -n "apk"
    exit 0
fi

# Check FreeBSD
#
which pkg >/dev/null 2>&1

if [[ $? -eq 0 ]]
then
    echo -n "bsdpkg"
    exit 0
fi

# Check Red Hat
#
which rpm >/dev/null 2>&1

if [[ $? -eq 0 ]]
then
    echo -n "rpm"
    exit 0
fi

# Check Debian
#
which dpkg >/dev/null 2>&1

if [[ $? -eq 0 ]]
then
    echo -n "deb"
    exit 0
fi

# Nothing else to probe, it's over!
#
echo "Unsupported distribution."
exit 1
