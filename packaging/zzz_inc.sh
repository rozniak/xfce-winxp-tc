#!/usr/bin/env bash

#
# zzz_inc.sh - Build Script Includes
#
# This source-code is part of Windows XP stuff for XFCE:
# <<https://www.oddmatics.uk>>
#
# Author(s): Rory Fewell <roryf@oddmatics.uk>
#

#
# FUNCTIONS
#
zzz_dist_target_to_vars()
{
    if [[ "${1}" =~ ^([a-z]+)(-([a-z]+))?$ ]]
    then
        export DIST_ID="${BASH_REMATCH[1]}"
        export DIST_ID_EXT="${BASH_REMATCH[3]}"
        return 0
    fi

    echo "zzz_inc: Target passed in via -t was not understood." >&2
    echo "zzz_inc: Should look like"                            >&2
    echo "zzz_inc: -t deb"                                      >&2
    echo "zzz_inc: -t xbps-glibc"                               >&2

    return 1
}
