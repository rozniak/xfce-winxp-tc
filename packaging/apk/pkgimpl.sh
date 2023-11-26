#!/bin/bash

#
# pkgimpl.sh - Packaging Implementation (Alpine Linux)
#
# This source-code is part of Windows XP stuff for XFCE:
# <<https://www.oddmatics.uk>>
#
# Author(s): Rory Fewell <roryf@oddmatics.uk>
#
# Special Thanks:
#     Martijn Braam <martijn@brixit.nl> (early Alpine/APK support)
#



#
# FUNCTIONS
#
do_packaging()
{
    # Build package now
    #
    cd "${full_component_dir}"
    abuild rootpkg

    pkg_res=$?

    cd "${CURDIR}"

    if [[ $pkg_res -gt 0 ]]
    then
        echo "Package build failure!"
        exit 1
    fi

    # Move package to output
    #
    clean_pkg_name=`cat ${full_component_dir}/APKBUILD | grep 'pkgname' | cut -d'=' -f2 | xargs echo -n`

    find ~/packages -iname "${clean_pkg_name}*.apk" -exec mv '{}' "${OPT_OUTPUT_DIR}/${clean_pkg_name}.apk" \;

    echo "Packaged ${clean_pkg_name}"
}