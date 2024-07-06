#/usr/bin/env sh

# This script will export the vars for distro / init system detection, kinda
# based off of /packaging/distid.sh
#
# TODO: No init detection yet...
#

# Check Arch Linux
#
which pacman >/dev/null 2>&1

if [ $? -eq 0 ]
then
    export DIST_ID="archpkg"

    # Try to be more specific...
    #
    . /etc/os-release

    if [ -z "$ID" ]
    then
        export DIST_ID_EXT="unk"
    else
        export DIST_ID_EXT="$ID"
    fi

    return 0
fi

# Check Alpine Linux
#
which apk >/dev/null 2>&1

if [ $? -eq 0 ]
then
    export DIST_ID="apk"
    export DIST_ID_EXT="alpine" # TODO: Temp, check pmOS...

    return 0
fi

# Check FreeBSD
#
which pkg >/dev/null 2>&1

if [ $? -eq 0 ]
then
    export DIST_ID="bsdpkg"
    export DIST_ID_EXT="freebsd" # TODO: Temp again

    return 0
fi

# Check Red Hat
#
which rpm >/dev/null 2>&1

if [ $? -eq 0 ]
then
    export DIST_ID="rpm"
    export DIST_ID_EXT="unk" # TODO: Check Fedora

    return 0
fi

# Check Void Linux
#
which xbps-install >/dev/null 2>&1

if [ $? -eq 0 ]
then
    export DIST_ID="xbps"

    # Nicked from /packaging/distid.sh
    # 
    find /usr/lib -iname "*ld-musl*" | read

    if [ $? -eq 0 ]
    then
        DIST_ID_EXT="musl"
    else
        DIST_ID_EXT="glibc"
    fi

    return 0
fi

# Check Debian
#
which dpkg >/dev/null 2>&1

if [ $? -eq 0 ]
then
    export DIST_ID="deb"

    # More specific
    #
    . /etc/os-release

    if [ -z "$ID" ]
    then
        export DIST_ID_EXT="unk"
    else
        export DIST_ID_EXT="$ID"
    fi

    return 0
fi

# We don't support anything else!
#
export DIST_ID="unsupported"
export DIST_ID_EXT="unsupported"

return 1
