#!/bin/bash

#
# pkgimpl.sh - Packaging Implementation (Arch Linux)
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
    # Build package now
    #
    cd "${full_component_dir}"
    makepkg -f --repackage

    pkg_res=$?

    cd "${CURDIR}"

    if [[ $pkg_res -gt 0 ]]
    then
        echo "Package build failure!"
        exit 1
    fi

    # Move package to output
    #
    clean_pkg_name=`cat ${full_component_dir}/PKGBUILD | grep 'pkgname' | cut -d'=' -f2 | xargs echo -n`

    find "${full_component_dir}" -iname "${clean_pkg_name}*.pkg.tar.zst" -exec mv '{}' "${OPT_OUTPUT_DIR}/${clean_pkg_name}.pkg.tar.zst" \;

    echo "Packaged ${clean_pkg_name}"
}
