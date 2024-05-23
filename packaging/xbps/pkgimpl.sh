#!/bin/bash

#
# pkgimpl.sh - Packaging Implementation (Void Linux)
#
# This source-code is part of Windows XP stuff for XFCE:
# <<https://www.oddmatics.uk>>
#
# Author(s): Rory Fewell <roryf@oddmatics.uk>
#



#
# FUNCTIONS
#
do_packaging()
{
    pkg_dir="${full_component_dir}/out"

    # Assemble package
    #
    cd "${full_component_dir}"
    make install DESTDIR="${pkg_dir}"

    ass_res=$?

    cd "${CURDIR}"

    if [[ $ass_res -gt 0 ]]
    then
        echo "Package assembly failure!"
        exit 1
    fi

    # Build package now
    #
    . "${full_component_dir}/xbps-vars.sh"

    xbps-create --architecture "${XBPS_ARCH}"                             \
                --dependencies "${XBPS_DEPENDENCIES}"                     \
                --desc         "${XBPS_DESC}"                             \
                --homepage     "https://github.com/rozniak/xfce-winxp-tc" \
                --license      "${XBPS_LICENSE}"                          \
                --maintainer   "${XBPS_MAINTAINER}"                       \
                --pkgver       "${XBPS_PKGVER}"                           \
                "${pkg_dir}"

    # Move package to output
    #
    find "${CURDIR}" -maxdepth 1                      \
                     -type     f                      \
                     -iname    "${XBPS_PKGVER}*.xbps" \
                     -exec mv '{}' "${OPT_OUTPUT_DIR}" \;

    echo "Packaged ${XBPS_PKGVER}"
}
