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
# FUNCTIONS
#
g_ambiguous=0
g_compare_against_env=0
g_compare_successful=0
g_detected_pkgmgr=""
g_detected_pkgmgr_ext=""

set_pkgmgr()
{
    if [[ \
        ${g_compare_against_env} -eq 1 && \
        "${1}" == "${DIST_ID}"         && \
        "${2}" == "${DIST_ID_EXT}"        \
    ]]
    then
        g_compare_successful=1
    fi

    if [[ ${g_detected_pkgmgr} != "" ]]
    then
        g_ambiguous=1
    fi

    g_detected_pkgmgr=${1}
    g_detected_pkgmgr_ext=${2}
}



#
# MAIN SCRIPT
#

# If DIST_ID is already defined, validate it
#
if [[ ! -z "${DIST_ID}" ]]
then
    l_target_valid=0

    case "${DIST_ID}" in
        # For all these distros, only <distro>-std is valid
        #
        apk | archpkg | bsdpkg | deb | rpm)
            if [[ -z "${DIST_ID_EXT}" ]]
            then
                DIST_ID_EXT="std"
            fi

            if [[ "${DIST_ID_EXT}" == "std" ]]
            then
                l_target_valid=1
            fi

            ;;

            # xbps must be either musl or glibc
            #
            xbps)
                case "${DIST_ID_EXT}" in
                    glibc | musl)
                        l_target_valid=1
                        ;;
                esac

                ;;
    esac

    if [[ $l_target_valid -eq 1 ]]
    then
        # Proceed on with the check as normal, to ensure the desired package
        # manager is installed
        #
        g_compare_against_env=1
    else
        echo "distid: The distro target is not valid." >&2
        return 1
    fi
fi

# Probe for package managers to try and determine what distro we're
# on
#

# Check Arch Linux
#
which pacman >/dev/null 2>&1

if [[ $? -eq 0 ]]
then
    set_pkgmgr "archpkg" "std"
fi

# Check Alpine Linux
#
which apk >/dev/null 2>&1

if [[ $? -eq 0 ]]
then
    set_pkgmgr "apk" "std"
fi

# Check Debian
#
which dpkg >/dev/null 2>&1

if [[ $? -eq 0 ]]
then
    set_pkgmgr "deb" "std"
fi

# Check FreeBSD
#
which pkg >/dev/null 2>&1

if [[ $? -eq 0 ]]
then
    set_pkgmgr "bsdpkg" "std"
fi

# Check Red Hat
#
which rpm >/dev/null 2>&1

if [[ $? -eq 0 ]]
then
    set_pkgmgr "rpm" "std"
fi

# Check Void Linux
#
which xbps-create >/dev/null 2>&1

if [[ $? -eq 0 ]]
then
    # This might be a rubbish way to determine glibc vs. musl, if it does suck
    # then someone needs to whinge and then I'll have to come up with something
    # better
    #
    find /usr/lib -iname "*ld-musl*" | read

    if [[ $? -eq 0 ]]
    then
        set_pkgmgr "xbps" "musl"
    else
        set_pkgmgr "xbps" "glibc"
    fi

    return 0
fi

# If we were just double-checking the already-set DIST_ID to make sure we have
# the package manager installed, then deal with that first
#
if [[ ${g_compare_against_env} -eq 1 ]]
then
    if [[ ${g_compare_successful} -eq 1 ]]
    then
        export DIST_ID="${DIST_ID}"
        export DIST_ID_EXT="${DIST_ID_EXT}"
        return 0
    else
        echo "distid: The format ${DIST_ID}-${DIST_ID_EXT} was specified." >&2
        echo "distid: Couldn't find the package manager for this format."  >&2
        echo "distid:"                                                     >&2
        echo "distid: Please double check what distro you're using, and"   >&2
        echo "distid: that you definitely have the package manager for"    >&2
        echo "distid: the format installed."                               >&2
        return 1
    fi
fi

# Okay so it was down to auto-detection, make sure we found a supported package
# manager - if we found more than 1 then the user must explicitly state what
# one to use via -t
#
if [[ ${g_ambiguous} -gt 0 ]]
then
    echo "distid: Ambiguity because multiple package managers present." >&2
    echo "distid: Use the -t switch to specify the target format."      >&2
    echo "distid: eg. -t deb"                                           >&2
    echo "distid: eg. -t xbps-musl"                                     >&2
    return 1
fi

if [[ ${g_detected_pkgmgr} == "" ]]
then
    echo "distid: No known package manager could be found."     >&2
    echo "distid: Please check your distribution is supported." >&2
    return 1
fi

# We passed the checks, happy days!
#
export DIST_ID="${g_detected_pkgmgr}"
export DIST_ID_EXT="${g_detected_pkgmgr_ext}"
return 0
