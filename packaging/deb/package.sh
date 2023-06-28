#!/bin/bash

#
# package.sh - Packaging Script (Debian)
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
SCRIPTDIR=`dirname "$0"`

REPO_ROOT=`realpath -s "${SCRIPTDIR}/../.."`



#
# ARGUMENTS
#
OPT_BUILD_DIR="${CURDIR}/build"
OPT_OUTPUT_DIR="${CURDIR}/dpkg-out"

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

# Ensure the built component exists
#
rel_component_dir="${1}"
full_component_dir="${OPT_BUILD_DIR}/${rel_component_dir}"
pkg_dir="${full_component_dir}/out"

if [[ ! -d "${pkg_dir}" ]]
then
    "Expected component to be built at ${full_component_dir}, it's missing."
    exit 1
fi

# Build package now
#
fakeroot dpkg-deb -v --build "${pkg_dir}"

if [[ $? -gt 0 ]]
then
    "Package build failure!"
    exit 1
fi

# Move package to output
#
clean_pkg_name=`cat ${full_component_dir}/control | grep 'Package:' | cut -d":" -f2 | xargs echo -n`

mv "${pkg_dir}.deb" "${OPT_OUTPUT_DIR}/${clean_pkg_name}.deb"

echo "Packaged ${clean_pkg_name}"
