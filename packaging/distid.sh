#!/bin/bash

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

# Check Debian
# 
which dpkg >/dev/null 2>&1

if [[ $? -eq 0 ]]
then
    echo -n "deb"
    exit 0
fi

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

# Nothing else to probe, it's over!
#
echo "Unsupported distribution."
exit 1
