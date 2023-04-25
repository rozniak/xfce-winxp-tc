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
CMAKE_LOG_PATH="${CURDIR}/cmake-out.log"
REQUIRED_PACKAGES=(
    'cmake'
    'fakeroot'
    'make'
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

    if [[ ! -d "${theme_dir}" ]]
    then
        echo "Can't find visual style: ${themestr}"
        continue
    fi

    # Compile theme
    #
    local_build_dir="${CURDIR}/build"

    if [[ -d "${local_build_dir}" ]]
    then
        rm -rf "${local_build_dir}"
    fi

    mkdir "${local_build_dir}"
    cd "${local_build_dir}"
    cmake -DWINTC_PKGMGR=deb -DCMAKE_INSTALL_PREFIX=/usr "${theme_dir}" > "${CMAKE_LOG_PATH}"
    make DESTDIR=./out all install
    cd "${CURDIR}"

    if [[ $? -gt 0 ]]
    then
        echo "Failed build for ${themestr}, moving on..."
        continue
    fi

    # Compile package
    #
    fakeroot dpkg-deb -v --build "${local_build_dir}/out"

    if [[ $? -gt 0 ]]
    then
        echo "Failed to build package for ${themestr}, moving on..."
        continue
    fi

    # Tidy
    #
    package_name=`grep "CMAKING" ${CMAKE_LOG_PATH} |  cut -d':' -f2`

    mv "${local_build_dir}/out.deb" "${package_name}.deb"
    rm -rf "${local_build_dir}"
    rm -f "${CMAKE_LOG_PATH}"

    echo "Built ${themestr}."
done
