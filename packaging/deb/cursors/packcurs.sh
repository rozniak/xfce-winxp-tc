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

        log_path="${CURDIR}/${theme_scheme}-${theme_shadows}.log"

        if [[ ! -d "${theme_cfg_dir}" ]]
        then
            echo "Cannot find xcursorgen configs for ${themestr}, skipping."
            continue
        fi

        if [[ ! -f "${theme_map_path}" ]]
        then
            echo "Cannot find mappings file for ${themestr}, skipping."
            continue
        fi

        if [[ -f "${log_path}" ]]
        then
            rm -f "${log_path}"

            if [[ $? -gt 0 ]]
            then
                echo "Failed to delete ${log_path}, skipping."
                continue
            fi
        fi

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

        mkdir -p "${PKG_DIR}"
        mkdir -p "${pkg_cursor_dir}"
        mkdir -p "${pkg_curres_dir}"
        mkdir -p "${pkg_debian_dir}"

        # Generate X cursors, shift them to our working dir and remove the '.out' ext
        #
        cd "${theme_dir}"

        find "${theme_cfg_dir}" -type f -exec xcursorgen '{}' '{}.out' \; > "${log_path}" 2>&1
        mv "${theme_cfg_dir}"/*.out "${pkg_curres_dir}" > "${log_path}" 2>&1
        find "${pkg_curres_dir}" -type f -execdir rename 's/(.*)\.cfg\.out/\1/' '{}' \; > "${log_path}" 2>&1

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

                ln -s "${pkg_res_target}" "${pkg_xcur_source}" > "${log_path}" 2>&1

                if [[ $? -gt 0 ]]
                then
                    "Failed to create symlink for ${xcurname} in ${themestr}."
                    continue
                fi
            else
                echo "Invalid mapping '${mapping}' in ${theme_map_path}."
                continue
            fi
        done

        # Copy files
        #
        pkg_setup_result=0

        cp "${theme_dir}/index.theme" "${pkg_theme_dir}" >> "${log_path}" 2>&1
        ((pkg_setup_result+=$?))

        cp "${theme_dir}/debian-control" "${pkg_debian_dir}/control" >> "${log_path}" 2>&1
        ((pkg_setup_result+=$?))

        # Check package setup good
        #
        if [[ $pkg_setup_result -gt 0 ]]
        then
            echo "Failed to copy files for ${themestr}, see ${log_path} for output."
            rm -rf "${PKG_DIR}"
            continue
        fi

        # Compile package
        #
        cd "${CURDIR}"

        fakeroot dpkg-deb -v --build "${PKG_DIR}" >> "${log_path}" 2>&1

        if [[ $? -gt 0 ]]
        then
            "Failed to build package for ${themestr}, see ${log_path} for output."
            continue
        fi

        # Tidy
        #
        mv "${PKG_DIR}.deb" "cursor-theme-${theme_shadows}-${theme_scheme}.deb"

        rm -rf "${PKG_DIR}"

        echo "Built ${themestr}."
    else
        echo "Failed to parse cursor theme name: ${themestr}."
        continue
    fi
done

echo "Packaging complete."
