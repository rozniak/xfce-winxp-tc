#!/bin/bash

#
# packlibs.sh - Shared Library Packaging Script (Debian)
#
# This source-code is part of Windows XP stuff for XFCE:
# <<https://www.oddmatics.uk>>
#
# Author(s): Rory Fewell <roryf@oddmatics.uk>
#

#
# CONSTANTS
#
CORE_COUNT=`nproc`
CURDIR=`realpath -s "./"`
PKG_DIR=`realpath -s "./tmp.lib-pkg"`
REQUIRED_PACKAGES=(
    'cmake'
    'fakeroot'
    'make'
)
SCRIPTDIR=`dirname "$0"`

REPO_ROOT=`realpath -s "${SCRIPTDIR}/../../.."`
SO_ROOT="${REPO_ROOT}/shared"



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
    echo 'Usage: packlibs.sh [<name>]...'
    exit 1
fi



#
# MAIN SCRIPT
#

# The arguments passed to this script should specify the libraries to build, for
# example:
#
#     ./packlibs.sh exec
#
for libstr in "$@"
do
    lib_dir="${SO_ROOT}/${libstr}"
    log_path="${CURDIR}/${libstr}-build.log"

    if [[ ! -d "${lib_dir}" ]]
    then
        echo "Can't find library: ${libstr}."
        exit 1
    fi

    if [[ -f "${log_path}" ]]
    then
        rm -f "${log_path}"

        if [[ $? -gt 0 ]]
        then
            echo "Failed delete ${log_path}, skipping build."
            continue
        fi
    fi

    # Set up working directory
    #
    build_dir="${lib_dir}/build"

    cd "${lib_dir}"

    if [[ -d "${build_dir}" ]]
    then
        rm -rf "${build_dir}"
    fi

    mkdir "${build_dir}"
    cd "${build_dir}"

    # Compile the source
    #
    cmake .. -DBUILD_SHARED_LIBS=ON >> "${log_path}" 2>&1
    make -j${CORE_COUNT} >> "${log_path}" 2>&1

    if [[ $? -gt 0 ]]
    then
        echo "Compilation failed for ${libstr}, see ${log_path} for output."
        continue
    fi

    # Source dir paths from the pkg-config file (arch-dependant dirs)
    #
    source <(head --lines=5 "${build_dir}/"*.pc)

    # Set up our package's working directory
    #
    pkg_debian_dir="${PKG_DIR}/DEBIAN"
    pkg_include_dir="${PKG_DIR}${includedir}"
    pkg_lib_dir="${PKG_DIR}${libdir}"
    pkg_pkgconfig_dir="${PKG_DIR}${pkgconfigdir}"

    mkdir -p "${pkg_debian_dir}"
    mkdir -p "${pkg_include_dir}"
    mkdir -p "${pkg_lib_dir}"
    mkdir -p "${pkg_pkgconfig_dir}"

    # Copy files
    #
    cd "${CURDIR}"

    cp "${build_dir}/debian-control" "${pkg_debian_dir}/control"
    cp "${build_dir}/"*.h "${pkg_include_dir}"
    cp "${build_dir}/"*.so* "${pkg_lib_dir}"
    cp "${build_dir}/"*.pc "${pkg_pkgconfig_dir}"

    # Compile package
    #
    fakeroot dpkg-deb -v --build "${PKG_DIR}" >> "${log_path}" 2>&1

    if [[ $? -gt 0 ]]
    then
        echo "Failed to build package for ${libstr}, see ${log_path} for output."
        continue
    fi

    mv "${PKG_DIR}.deb" "lib${libstr}.deb"

    rm -rf "${PKG_DIR}"

    echo "Built lib${libstr}."
done

echo "Packaging complete."
