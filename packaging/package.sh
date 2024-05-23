#!/usr/bin/env bash

#
# package.sh - Packaging Script
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

REPO_ROOT=`realpath "${SCRIPTDIR}/../.."`

SH_DISTID="${SCRIPTDIR}/distid.sh"



#
# ARGUMENTS
#
OPT_BUILD_DIR="${CURDIR}/build"
OPT_OUTPUT_DIR="${CURDIR}/local-out"

while getopts "hi:o:" opt;
do
    case "${opt}" in
        h)
            echo "Usage: package.sh [-hio] <dir>"
            echo ""
            echo " -h : display this help screen"
            echo " -i : specify root build directory"
            echo " -o : specify output directory"
            echo ""

            exit 0
            ;;

        i)
            OPT_BUILD_DIR="${OPTARG}"
            ;;

        o)
            OPT_OUTPUT_DIR="${OPTARG}"
            ;;
    esac
done

shift $((OPTIND-1))

if [[ $# -ne 1 ]]
then
    echo "Should specify single component to package."
    exit 1
fi



#
# MAIN SCRIPT
#
if [[ ! -f "${SH_DISTID}" ]]
then
    echo "distid.sh not found - this should never happen!!"
    exit 1
fi

# Ensure the output containing dir exists
#
if [[ ! -d "${OPT_OUTPUT_DIR}" ]]
then
    mkdir -p "${OPT_OUTPUT_DIR}"

    if [[ $? -gt 0 ]]
    then
        echo "Unable to ensure output directory exists, aborting."
        exit 1
    fi
fi

# Identify our distro
#
. "${SH_DISTID}"

if [[ $? -gt 0 ]]
then
    echo "Failed to identify distribution."
    exit 1
fi

# Ensure packaging implementation available
#
sh_pkg_impl="${SCRIPTDIR}/${DIST_ID}/pkgimpl.sh"

if [[ ! -f "${sh_pkg_impl}" ]]
then
    echo "Packaging implementation for ${DIST_ID} not found!"
    exit 1
fi

# Ensure the built component exists
#
rel_component_dir="${1}"
full_component_dir="${OPT_BUILD_DIR}/${rel_component_dir}"

if [[ ! -d "${full_component_dir}" ]]
then
    echo "Component doesn't seem to be built at ${full_component_dir}"
    exit 1
fi

# Hand off to implementation
#
. "${sh_pkg_impl}"

do_packaging
