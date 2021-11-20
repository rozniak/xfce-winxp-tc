#!/bin/bash

#
# packthem.sh - Desktop Theme Packaging Script (Debian)
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
PKG_DIR=`realpath -s "./tmp.theme-pkg"`
REQUIRED_PACKAGES=(
    'fakeroot'
    'ruby-sass'
)
SCRIPTDIR=`dirname "$0"`

REPO_ROOT=`realpath -s "${SCRIPTDIR}/../../.."`


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
if [[ $# -gt 0 ]]
then
    echo 'This script only builds Luna (Blue) currently! No args needed.'
    exit 1
fi


#
# MAIN SCRIPT
#

# This script is simply a reworking of the old build-deb.sh for now. We just build a
# package for Luna (Blue).
#
# This will change once themes are improved to separate base XP structure vs. the
# visual style specific stuff (bitmaps, colours, etc.)
#

# Prepare working dir
#
debian_dir="${PKG_DIR}/DEBIAN"
theme_dir="${PKG_DIR}/usr/share/themes/Luna"
theme_gtk3_dir="${theme_dir}/gtk-3.0"

if [[ -d "${PKG_DIR}" ]]
then
    rm -rf "${PKG_DIR}"
fi

mkdir "${PKG_DIR}"
mkdir -p "${debian_dir}"
mkdir -p "${theme_dir}"
mkdir -p "${theme_gtk3_dir}"

# Copy static content straight into package
#
pkg_setup_result=0
theme_root="${REPO_ROOT}/themes/luna/blue"

cp -r "${theme_root}/gtk-2.0" "${theme_dir}/gtk-2.0"
((pkg_setup_result+=$?))

cp -r "${theme_root}/Resources" "${theme_dir}/Resources"
((pkg_setup_result+=$?))

cp -r "${theme_root}/xfwm4" "${theme_dir}/xfwm4"
((pkg_setup_result+=$?))

# Compile SASS for GTK 3 theme
#
scss "${theme_root}/gtk-3.0/main.scss" "${theme_gtk3_dir}/gtk.css" --sourcemap=none >> "luna-blue.log" 2>&1
((pkg_setup_result+=$?))

# Add CONTROL file
#
cp "${theme_root}/debian-control" "${debian_dir}/control"
((pkg_setup_result+=$?))

# Check package setup is good
#
if [[ $pkg_setup_result -gt 0 ]]
then
    echo "Failed to copy files for Luna (Blue), see ${log_path} for output."
    rm -rf "${PKG_DIR}"
    exit 1
fi

# Compile package
#
fakeroot dpkg-deb -v --build "${PKG_DIR}" >> "luna-blue.log" 2>&1

if [[ $? -gt 0 ]]
then
    echo "Failed to build package for Luna (Blue), see luna-blue.log for output."
    rm -rf "${PKG_DIR}"
    exit 1
fi

# Tidy
#
mv "${PKG_DIR}.deb" "xfce-theme-luna-blue.deb"

rm -rf "${PKG_DIR}"

echo "Build Luna (Blue)"
