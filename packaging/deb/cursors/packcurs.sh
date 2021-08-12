#!/bin/bash

#
# packcurs.sh - Cursor Packaging Script (Debian)
#
# This source-code is part of Windows XP stuff for XFCE:
# <<https://www.oddmatics.uk>>
#
# Author(s): Rory Fewell <roryf@oddmaics.uk>
#

#
# CONSTANTS
#
CURDIR=`realpath -s "./"`
MAPPINGS_FILENAME='mappings'
PKG_DIR=`realpath -s "./tmp.cur-pkg"`
REQUIRED_PACKAGES=(
    'fakeroot'
    'rename'
    'x11-apps'
)
SCRIPTDIR=`dirname "$0"`

REPO_ROOT=`realpath -s "${SCRIPTDIR}/../../.."`
CURSORS_ROOT="${REPO_ROOT}/cursors"



#
# PRE-FLIGHT CHECKS
#
for package in "${REQUIRED_PACKAGES[@]}"
do
    dpkg -s $package > /dev/null 2>&1

    if [[ $? -gt 0 ]]
    then
        echo "You are missing package: ${package}"
        exit 1
    fi
done


#
# ARGUMENTS
#
if [[ $# -eq 0 ]]
then
    echo 'Usage: packcurs.sh [<no-shadow|with-shadow>/<name>]...'
    exit 1
fi


#
# MAIN SCRIPT
#

# The arguments passed to this script should specify the cursor themes to build, for
# example:
#
#     ./packcurs.sh no-shadow/standard shadow/standard
#
for themestr in "$@"
do
    theme_dir="${CURSORS_ROOT}/${themestr}"

    if [[ ! -d "${theme_dir}" ]]
    then
        echo "Can't find cursor theme: ${themestr}."
        continue
    fi

    # Split the theme name out with regex
    #
    if [[ $themestr =~ ([A-Za-z0-9._-]+)/([A-Za-z0-9._-]+) ]]
    then
        # Theme paths and names
        #
        theme_cfg_dir="${theme_dir}/cfg"
        theme_map_path="${theme_dir}/mappings"

        theme_shadows="${BASH_REMATCH[1]}"
        theme_scheme="${BASH_REMATCH[2]}"

        # Set up our package's working directory
        #
        pkg_theme_dir="${PKG_DIR}/usr/share/icons/${theme_scheme}-${theme_shadows}"
        pkg_cursor_dir="${pkg_theme_dir}/cursors"
        pkg_curres_dir="${pkg_cursor_dir}/res"
        pkg_debian_dir="${PKG_DIR}/DEBIAN"

        if [[ -d "${PKG_DIR}" ]]
        then
            rm -rf "${PKG_DIR}"
        fi

        mkdir "${PKG_DIR}"
        mkdir -p "${pkg_curres_dir}"
        mkdir "${pkg_debian_dir}"

        # Generate X cursors, shift them to our working dir and remove the '.out' ext
        #
        cd "${theme_dir}"

        find "${theme_cfg_dir}" -type f -exec xcursorgen '{}' '{}.out' \; > /dev/null 2>&1
        mv "${theme_cfg_dir}"/*.out "${pkg_curres_dir}"
        find "${pkg_curres_dir}" -type f -execdir rename 's/(.*)\.cfg\.out/\1/' '{}' \; > /dev/null 2>&1

        # Create symbolic links
        #
        readarray -t mappings < $theme_map_path

        cd "${pkg_cursors_dir}"

        for mapping in ${mappings[*]}
        do
            if [[ $mapping =~ ([A-Za-z0-9_-]+)--\>([A-Za-z0-9_-]+) ]]
            then
                xcurname="${BASH_REMATCH[1]}"
                resname="${BASH_REMATCH[2]}"

                pkg_res_target="res/${resname}"
                pkg_xcur_source="${pkg_cursor_dir}/${xcurname}"

                ln -s "${pkg_res_target}" "${pkg_xcur_source}"
            else
                echo "Invalid mapping '${mapping}' in ${theme_map_path}."
                continue
            fi
        done

        # Copy across index.theme
        #
        cp "${theme_dir}/index.theme" "${pkg_theme_dir}"

        # Create CONTROL file
        #
        # FIXME: One day this should probably read the description and maintainer(?)
        #        from a file in the cursor theme's directory
        #
        debian_control=`cat <<EOF
Package: cursor-theme-${theme_shadows}-${theme_scheme}
Version: 0.0.1
Maintainer: Rory Fewell <roryf@oddmatics.uk>
Architecture: all
Section: non-free
Description: A Windows XP cursor theme.
EOF
`
        echo "${debian_control}" > "${pkg_debian_dir}/control"

        # Compile package
        #
        cd "${CURDIR}"

        fakeroot dpkg-deb -v --build "${PKG_DIR}" > /dev/null 2>&1

        if [[ $? -gt 0 ]]
        then
            "Failed to build package for ${themestr}."
            continue
        fi

        mv "${PKG_DIR}.deb" "cursor-theme-${theme_shadows}-${theme_scheme}.deb"

        rm -rf "${PKG_DIR}"

        echo "Built ${themestr}."
    else
        echo "Failed to parse cursor theme name: ${themestr}."
        continue
    fi
done

echo "Packaging complete."
