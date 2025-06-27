#/usr/bin/env sh

# This script will export the vars for distro / init system detection, kinda
# based off of /packaging/distid.sh
#
# TODO: No init detection yet...
#

if [ -f /etc/os-release ]
then
    . /etc/os-release
fi

export WSETUP_DIST_NAME="$PRETTY_NAME"

case "$ID" in
    # apk
    #
    alpine)
        export WSETUP_DIST_PKGFMT="apk"
        ;;

    # archpkg
    #
    arch | endeavouros | manjaro)
        export WSETUP_DIST_PKGFMT="archpkg"
        ;;

    # bsdpkg
    #
    freebsd)
        export WSETUP_DIST_PKGFMT="bsdpkg"
        ;;

    # deb
    #
    debian | linuxmint | ubuntu | zorin)
        export WSETUP_DIST_PKGFMT="deb"
        ;;

    # rpm
    #
    fedora)
        export WSETUP_DIST_PKGFMT="rpm"
        ;;

    # xbps
    #
    void)
        export WSETUP_DIST_PKGFMT="xbps"

        find /usr/lib -iname "*ld-musl*" | read

        if [ $? -eq 0 ]
        then
            export WSETUP_DIST_PKGFMT_EXT="musl"
        else
            export WSETUP_DIST_PKGFMT_EXT="glibc"
        fi
        ;;
esac
