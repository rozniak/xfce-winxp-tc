#!/bin/bash

#
# bldnpkg.sh - Build and Package Script (Debian)
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
REQUIRED_PACKAGES=(
    'cmake'
    'fakeroot'
    'make'
)
SCRIPTDIR=`dirname "$0"`

CMAKE_LOG_PATH="${CURDIR}/cmake-out.log"
REPO_ROOT=`realpath -s "${SCRIPTDIR}/../.."`



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
    echo 'Usage: bldnpkg.sh [<dir>]...'
    exit 1
fi



#
# MAIN SCRIPT
#

# The arguments passed to this script should specify directories with
# components we can build and package, for example:
#
#     ./bldnpkg.sh shell/start themes/luna/blue
#
for component_dir in "$@"
do
    full_component_dir="${REPO_ROOT}/${component_dir}"

    if [[ ! -d "${full_component_dir}" ]]
    then
        echo "Can't find component: ${component_dir}, aborting."
        exit 1
    fi

    # Compile component
    #
    build_result=0
    local_build_dir="${CURDIR}/build"

    if [[ -d "${local_build_dir}" ]]
    then
        rm -rf "${local_build_dir}"
    fi

    mkdir "${local_build_dir}"
    cd "${local_build_dir}"

    # Default to Pro SKU for now
    cmake -DWINTC_SKU=xpclient-pro -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=/usr -DWINTC_PKGMGR=deb "${full_component_dir}" > "${CMAKE_LOG_PATH}"
    ((build_result+=$?))

    cat "${CMAKE_LOG_PATH}" # So the output is still in stdout as well

    make DESTDIR=./out all install
    ((build_result+=$?))

    cd "${CURDIR}"

    if [[ $build_result -gt 0 ]]
    then
        echo "Failed build for ${component_dir}, aborting."
        exit 1
    fi

    # Compile package
    #
    fakeroot dpkg-deb -v --build "${local_build_dir}/out"

    if [[ $? -gt 0 ]]
    then
        echo "Failed to build package for ${component_dir}, aborting."
        exit 1
    fi

    # Tidy
    #
    package_name=`grep "CMAKING" ${CMAKE_LOG_PATH} | cut -d':' -f2`

    mv "${local_build_dir}/out.deb" "${package_name}.deb"
    rm -rf "${local_build_dir}"
    rm -f "${CMAKE_LOG_PATH}"

    echo "Built ${component_dir}"
done
