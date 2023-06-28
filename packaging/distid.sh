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
if [[ ! -f "/etc/os-release" ]]
then
    echo "Unable to identify distribution."
    exit 1
fi

# Retrieve distro ID
#
distro_in_use=`cat /etc/os-release | grep "^ID" | cut -d"=" -f2`

case "${distro_in_use}" in
    debian)
        echo -n "deb"
        ;;

    *)
        echo "Unsupported distribution."
        exit 1
        ;;
esac
