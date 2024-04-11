#!/bin/bash

#
# pkgimpl.sh - Packaging Implementation (FreeBSD)
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

    if [[ $ass_res -gt 0 ]]
    then
        cd "${CURDIR}"
        echo "Package assembly failure!"
        exit 1
    fi

    # Build package now
    #
    (cd "${pkg_dir}/usr/local" && find . -type f,l) | sed 's/\.\///' > plist
    pkg create -M manifest -r "${pkg_dir}" -p plist

    cd "${CURDIR}"

    if [[ $? -gt 0 ]]
    then
        echo "Package build failure!"
        exit 1
    fi

    # Move package to output
    #
    clean_pkg_name=`cat ${full_component_dir}/manifest | grep 'name:' | cut -d":" -f2 | xargs echo -n`

    find "${full_component_dir}" -iname "*.pkg" -exec mv '{}' "${OPT_OUTPUT_DIR}/${clean_pkg_name}.pkg" \;

    echo "Packaged ${clean_pkg_name}"
}
