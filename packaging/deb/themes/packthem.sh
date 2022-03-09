#!/bin/bash

#
# packthem.sh - Visual Style Packaging Script (Debian)
#
# This source-code is part of Windows XP stuff for XFCE:
# <<https://www.oddmatics.uk>>
#
# Author(s): Rory Fewell <roryf@oddmatics.uk>
#

#
# CONSTANTS
#
CURDIR=`realpath -s "./"`
PKG_DIR=`realpath -s "./tmp.cur-pkg"`
REQUIRED_PACKAGES=(
    'fakeroot'
    'ruby-sass'
)
SCRIPTDIR=`dirname "$0"`

REPO_ROOT=`realpath -s "${SCRIPTDIR}/../../.."`
THEMES_ROOT="${REPO_ROOT}/themes"



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
    echo 'Usage: packthem.sh [<theme>]...'
    exit 1
fi



#
# MAIN SCRIPT
#

# The arguments passed to this script should specify the visual style themes to build,
# for example:
#
#     ./packthem.sh luna/blue zune symphony/royale
#
for themestr in "$@"
do
    theme_dir="${THEMES_ROOT}/${themestr}"
    theme_vars_path="${theme_dir}/themevars.sh"

    if [[ ! -d "${theme_dir}" ]]
    then
        echo "Can't find visual style: ${themestr}"
        continue
    fi

    # Import theme vars
    #
    if [[ ! -x "${theme_vars_path}" ]]
    then
        echo "Theme ${themestr} is missing themevars.sh or it is not executable."
        continue
    fi

    . "${theme_vars_path}"

    # Set up log path
    #
    log_path="${CURDIR}/theme-${THEME_RAWNAME}.log"

    if [[ -f "${log_path}" ]]
    then
        rm -f "${log_path}"

        if [[ $? -gt 0 ]]
        then
            echo "Failed to delete ${log_path}, skipping."
            continue
        fi
    fi

    # Packaging
    #
    pkg_name="wintc-theme-${THEME_RAWNAME}"

    # Set up our package's working directory
    #
    pkg_debian_dir="${PKG_DIR}/DEBIAN"
    pkg_theme_dir="${PKG_DIR}/usr/share/themes/${THEME_DISPLAYNAME}"
    pkg_gtk3_dir="${pkg_theme_dir}/gtk-3.0"

    if [[ -d "${PKG_DIR}" ]]
    then
        rm -rf "${PKG_DIR}"
    fi

    mkdir -p "${PKG_DIR}"
    mkdir -p "${pkg_debian_dir}"
    mkdir -p "${pkg_theme_dir}"
    mkdir -p "${pkg_gtk3_dir}"

    # Copy files
    #
    pkg_setup_result=0

    cp -r "${theme_dir}/gtk-2.0" "${pkg_theme_dir}" >> "${log_path}" 2>&1
    ((pkg_setup_result+=$?))

    cp -Pr "${theme_dir}/Resources" "${pkg_theme_dir}" >> "${log_path}" 2>&1
    ((pkg_setup_result+=$?))

    cp -Pr "${theme_dir}/xfwm4" "${pkg_theme_dir}" >> "${log_path}" 2>&1
    ((pkg_setup_result+=$?))

    # Compile SASS for GTK 3 theme
    #
    scss "${theme_dir}/gtk-3.0/main.scss" "${pkg_gtk3_dir}/gtk.css" --sourcemap=none >> "${log_path}" 2>&1
    ((pkg_setup_result+=$?))

    # Check package setup is good
    #
    if [[ $pkg_setup_result -gt 0 ]]
    then
        echo "Failed to copy files for ${themestr}, see ${log_path} for output."
        rm -rf "${PKG_DIR}"
        continue
    fi

    # Write debian-control
    #
    tee "${pkg_debian_dir}/control" > /dev/null << EOF
Package: ${pkg_name}
Version: 0.0.1
Maintainer: ${THEME_MAINTAINER}
Architecture: all
Section: non-free
Description: ${THEME_DESCRIPTION}
Depends: xfwm4
EOF

    # Compile package
    #
    fakeroot dpkg-deb -v --build "${PKG_DIR}" >> "${log_path}" 2>&1

    if [[ $? -gt 0 ]]
    then
        "Failed to build package for ${themestr}, see ${log_path} for output."
        continue
    fi

    # Tidy
    #
    mv "${PKG_DIR}.deb" "wintc-theme-${THEME_RAWNAME}.deb"

    rm -rf "${PKG_DIR}"

    echo "Built ${themestr}."
done
