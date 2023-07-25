#!/bin/bash

#
# pkgimpl.sh - Packaging Implementation (Debian)
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
    make install DESTDIR="${pkg_dir}" CMAKE_INSTALL_ALWAYS=1

    ass_res=$?
    
    cd "${CURDIR}"

    if [[ $ass_res -gt 0 ]]
    then
        echo "Package assembly failure!"
        exit 1
    fi

    # Build package now
    #
    fakeroot dpkg-deb -v --build "${pkg_dir}"

    if [[ $? -gt 0 ]]
    then
        echo "Package build failure!"
        exit 1
    fi

    # Move package to output
    #
    clean_pkg_name=`cat ${full_component_dir}/control | grep 'Package:' | cut -d":" -f2 | xargs echo -n`

    mv "${pkg_dir}.deb" "${OPT_OUTPUT_DIR}/${clean_pkg_name}.deb"

    echo "Packaged ${clean_pkg_name}"
}
