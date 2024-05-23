#!/usr/bin/env bash

#
# build.sh - Build Single Component
#
# This source-code is part of Windows XP stuff for XFCE:
# <<https://www.oddmatics.uk>>
#
# Author(s): Rory Fewell <roryf@oddmatics.uk>
#

#
# CONSTANTS
#
CURDIR=`realpath "./"`
SCRIPTDIR=`dirname "$0"`

REPO_ROOT=`realpath "${SCRIPTDIR}/.."`

SH_DISTID="${SCRIPTDIR}/distid.sh"



#
# ARGUMENTS
#
OPT_BUILD_ROOT="${CURDIR}/build"
OPT_BUILD_TYPE="Release"
OPT_SKU="xpclient-pro"
OPT_USE_LOCAL_LIBS=0

while getopts "b:dhls:" opt;
do
    case "${opt}" in
        b)
            OPT_BUILD_ROOT="${OPTARG}"
            ;;

        d)
            OPT_BUILD_TYPE="Debug"
            ;;

        h)
            echo "Usage: build.sh [-bhls] <dir>"
            echo ""
            echo " -b : specify the directory to build relative to"
            echo " -d : produce checked build"
            echo " -h : display this help screen"
            echo " -l : use wintc libraries compiled here, not system"
            echo " -s : specify SKU to build (default xpclient-pro)"
            echo ""

            exit 0
            ;;

        l)
            OPT_USE_LOCAL_LIBS=1
            ;;

        s)
            OPT_SKU="${OPTARG}"
            ;;
    esac
done

shift $((OPTIND-1))

if [[ $# -ne 1 ]]
then
    echo "Should specify single component to build."
    exit 1
fi



#
# MAIN SCRIPT
#
rel_component_dir="${1}"
full_component_dir="${REPO_ROOT}/${rel_component_dir}"

if [[ ! -f "${SH_DISTID}" ]]
then
    echo "distid.sh not found - this should never happen!!"
    exit 1
fi

# Check component exists
#
if [[ ! -d "${full_component_dir}" ]]
then
    echo "No such component at ${full_component_dir}"
    exit 1
fi

# Identify distro build
#
dist_prefix="/usr"

. "${SH_DISTID}"

if [[ $? -gt 0 ]]
then
    "Failed to identify distribution."
    exit 1
fi

case "${DIST_ID}" in
    bsdpkg)
        dist_prefix="/usr/local"
        ;;
esac

# Ensure the build dir exists
#
full_build_dir="${OPT_BUILD_ROOT}/${rel_component_dir}"

mkdir -p "${full_build_dir}"

if [[ $? -gt 0 ]]
then
    echo "Unable to ensure the build directory exists, aborting."
    exit 1
fi

# Compile component
#
build_result=0

cd "${full_build_dir}"

rm -rf "${full_build_dir}"/*

cmake -DBUILD_SHARED_LIBS=ON                         \
      -DCMAKE_BUILD_TYPE="${OPT_BUILD_TYPE}"         \
      -DCMAKE_INSTALL_PREFIX="${dist_prefix}"        \
      -DWINTC_SKU="${OPT_SKU}"                       \
      -DWINTC_PKGMGR="${DIST_ID}"                    \
      -DWINTC_PKGMGR_EXT="${DIST_ID_EXT}"            \
      -DWINTC_USE_LOCAL_LIBS="${OPT_USE_LOCAL_LIBS}" \
      -DWINTC_LOCAL_LIBS_ROOT="${OPT_BUILD_ROOT}"    \
      "${full_component_dir}"
((build_result+=$?))

make -j$(nproc)
((build_result+=$?))

cd "${CURDIR}"

if [[ $build_result -gt 0 ]]
then
    echo "Failed build for ${rel_component_dir}"
    exit 1
fi

echo "Built ${rel_component_dir}"
