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
SH_ZZZINC="${SCRIPTDIR}/zzz_inc.sh"



#
# ARGUMENTS
#
OPT_BUILD_DIR="${CURDIR}/build"
OPT_DIST_TARGET=""
OPT_OUTPUT_DIR="${CURDIR}/local-out"

while getopts "hi:o:t:" opt;
do
    case "${opt}" in
        h)
            echo "Usage: package.sh [-hiot] <dir>"
            echo ""
            echo " -h : display this help screen"
            echo " -i : specify root build directory"
            echo " -o : specify output directory"
            echo " -t : specify the distro target (don't autodetect)"
            echo ""

            exit 0
            ;;

        i)
            OPT_BUILD_DIR="${OPTARG}"
            ;;

        o)
            OPT_OUTPUT_DIR="${OPTARG}"
            ;;

        t)
            OPT_DIST_TARGET="${OPTARG}"
            ;;
    esac
done

shift $((OPTIND-1))

if [[ $# -ne 1 ]]
then
    echo "package: Should specify single component to package." >&2
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

if [[ ! -f "${SH_ZZZINC}" ]]
then
    echo "zzz_inc.sh not found - this should never happen!!"
    exit 1
fi

# Pull includes
#
. "${SH_ZZZINC}"

if [[ ! -z "${OPT_DIST_TARGET}" ]]
then
    zzz_dist_target_to_vars "${OPT_DIST_TARGET}"

    if [[ $? -gt 0 ]]
    then
        exit 1
    fi
fi

# Ensure the output containing dir exists
#
if [[ ! -d "${OPT_OUTPUT_DIR}" ]]
then
    mkdir -p "${OPT_OUTPUT_DIR}"

    if [[ $? -gt 0 ]]
    then
        echo "package: Unable to ensure output directory exists, aborting." >&2
        exit 1
    fi
fi

# Identify our distro
#
. "${SH_DISTID}"

if [[ $? -gt 0 ]]
then
    exit 1
fi

# Ensure packaging implementation available
#
sh_pkg_impl="${SCRIPTDIR}/${DIST_ID}/pkgimpl.sh"

if [[ ! -f "${sh_pkg_impl}" ]]
then
    echo "package: Packaging implementation for ${DIST_ID} not found!" >&2
    exit 1
fi

# Ensure the built component exists
#
rel_component_dir="${1}"
full_component_dir="${OPT_BUILD_DIR}/${rel_component_dir}"

if [[ ! -d "${full_component_dir}" ]]
then
    echo "package: Component doesn't seem to be built at ${full_component_dir}" >&2
    exit 1
fi

# Hand off to implementation
#
. "${sh_pkg_impl}"

do_packaging
