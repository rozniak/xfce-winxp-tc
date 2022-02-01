#!/bin/bash

#
# packfnts.sh - Font Packaging Script (Debian)
#
# This source-code is part of Windows XP stuff for XFCE:
# <<https://www.oddmatics.uk>>
#

#
# CONSTANTS
#
CURDIR=`realpath -s "./"`
PKG_DIR=`realpath -s "./tmp.cur-pkg"`
PKG_NAME="wintc-fonts-xp"
REQUIRED_PACKAGES=(
    'fakeroot'
)
SCRIPTDIR=`dirname "$0"`

REPO_ROOT=`realpath -s "${SCRIPTDIR}/../../.."`
FONTS_ROOT="${REPO_ROOT}/fonts"



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
    echo 'Usage: packfnts.sh'
    exit 1
fi



#
# MAIN SCRIPT
#
log_path="${CURDIR}/${PKG_NAME}.log"
pkg_setup_result=0

pkg_ttf_dir="${PKG_DIR}/usr/share/fonts/truetype/wintc"
pkg_debian_dir="${PKG_DIR}/DEBIAN"

if [[ -d "${PKG_DIR}" ]]
then
    rm -rf "${PKG_DIR}"
fi

if [[ -f "${log_path}" ]]
then
    rm -rf "${log_path}"

    if [[ $? -gt 0 ]]
    then
        echo "Failed to delete ${log_path}, exiting."
        exit 1
    fi
fi

mkdir -p "${PKG_DIR}"
mkdir -p "${pkg_ttf_dir}"
mkdir -p "${pkg_debian_dir}"

cp "${FONTS_ROOT}/ttf"/* "${pkg_ttf_dir}" >> "${log_path}" 2>&1
((pkg_setup_result+=$?))

cp "${FONTS_ROOT}/debian-control" "${pkg_debian_dir}/control" >> "${log_path}" 2>&1
((pkg_setup_result+=$?))

# Check package setup good
#
if [[ $pkg_setup_result -gt 0 ]]
then
    echo "Failed to copy font files, see ${log_path} for output."
    rm -rf "${PKG_DIR}"
    exit 1
fi

# Compile package
#
cd "${CURDIR}"

fakeroot dpkg-deb -v --build "${PKG_DIR}" >> "${log_path}" 2>&1

if [[ $? -gt 0 ]]
then
    "Failed to build package for ${PKG_NAME}, see ${log_path} for output."
    exit 1
fi

# Tidy
#
mv "${PKG_DIR}.deb" "${PKG_NAME}.deb"

rm -rf "${PKG_DIR}"

echo "Built ${PKG_NAME}."
