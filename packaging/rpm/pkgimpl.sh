#!/bin/bash

#
# pkgimpl.sh - Packaging Implementation (Red Hat)
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
    # Build the package now
    #
    cd "${full_component_dir}"

    rpmdev-setuptree
    TRUE_BUILD_ROOT="${full_component_dir}" rpmbuild -ba rpm.spec

    pkg_res=$?

    cd "${CURDIR}"

    if [[ $pkg_res -gt 0 ]]
    then
        echo "Package build failure!"
        exit 1
    fi

    # Move package to output
    #
    clean_pkg_name=`cat ${full_component_dir}/rpm.spec | grep 'Name:' | cut -d':' -f2 | xargs echo -n`

    find "${HOME}/rpmbuild/RPMS" -name "${clean_pkg_name}*.rpm" -exec mv '{}' "${OPT_OUTPUT_DIR}/${clean_pkg_name}.rpm" \;

    echo "Packaged ${clean_pkg_name}"
}
