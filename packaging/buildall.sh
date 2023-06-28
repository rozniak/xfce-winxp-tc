#!/bin/bash

#
# buildall.sh - Build Entire System
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

REPO_ROOT=`realpath -s "${SCRIPTDIR}/.."`
TARGETS_PATH="${SCRIPTDIR}/targets"

SH_BUILD="${SCRIPTDIR}/build.sh"
SH_CHKDEPS="${SCRIPTDIR}/chkdeps.sh"
SH_DISTID="${SCRIPTDIR}/distid.sh"



#
# ARGUMENTS
#
OPT_BUILDLIST="${TARGETS_PATH}"
OPT_OUTPUT_DIR=""
OPT_SKU="xpclient-pro"

while getopts "c:ho:s:" opt;
do
    case "${opt}" in
        c)
            OPT_BUILDLIST="${OPTARG}"
            ;;

        h)
            echo "Usage: buildall.sh [-chos]"
            echo " -c : provide a list of components (default 'targets')"
            echo " -h : display this help screen"
            echo " -o : specify output directory for packages"
            echo " -s : specify SKU to build (default xpclient-pro)"
            echo ""

            exit 0
            ;;

        o)
            OPT_OUTPUT_DIR="${OPTARG}"
            ;;

        s)
            OPT_SKU="${OPTARG}"
            ;;
    esac
done


#
# FUNCTIONS
#
g_checked_pkg_sh=0

declare -a g_built_libs

build_component()
{
    local dist="${1}"
    local rel_dir="${2}"
    local target_dir="${REPO_ROOT}/${rel_dir}"
    local deps_path="${target_dir}/deps"

    # Ensure package script available
    #
    local sh_package="${SCRIPTDIR}/${dist}/package.sh"

    if [[ g_checked_pkg_sh -eq 0 ]]
    then
        if [[ ! -f "${sh_package}" ]]
        then
            echo "package.sh missing for ${dist}, cannot continue."
            exit 1
        fi

        g_checked_pkg_sh=1
    fi

    # All good, continue build
    #
    echo "buildall: Building ${rel_dir}"

    # Ensure we build library dependencies first
    #
    local deps_fd=0

    if [[ -f "${deps_path}" ]]
    then
        while IFS= read -u "${deps_fd}" -r dep_listing
        do
            # Looks like one of ours? (wintc-*)
            #
            if [[ "${dep_listing}" =~ wintc-(.+) ]]
            then
                local lib_shortname="${BASH_REMATCH[1]}"

                # Check we haven't already built this
                #
                check_already_built "${lib_shortname}"

                if [[ $? -eq 1 ]]
                then
                    continue
                fi

                # Check it's actually a library
                #
                if [[ -d "${REPO_ROOT}/shared/${lib_shortname}" ]]
                then
                    build_component "${dist}" "shared/${lib_shortname}"
                    g_built_libs+=("${lib_shortname}")
                fi
            fi
        done {deps_fd}<"${deps_path}"
    fi

    # Now build the component
    #
    "${SH_BUILD}" -l -s "${OPT_SKU}" "${rel_dir}"

    if [[ $? -gt 0 ]]
    then
        echo "buildall: Compile failure, bailing."
        exit 1
    fi

    # Package the component
    #
    "${sh_package}" -o "${OPT_OUTPUT_DIR}" "${rel_dir}"

    if [[ $? -gt 0 ]]
    then
        echo "buildall: Package failure, bailing."
        exit 1
    fi
}

check_already_built()
{
    local check_lib="${1}"

    for lib_name in "${g_built_libs[@]}"
    do
        if [[ "${lib_name}" == "${check_lib}" ]]
        then
            return 1
        fi
    done

    return 0
}


#
# MAIN SCRIPT
#
if [[ ! -f "${SH_BUILD}" ]]
then
    echo "build.sh not found - this should never happen!!"
    exit 1
fi

if [[ ! -f "${SH_CHKDEPS}" ]]
then
    echo "chkdeps.sh not found - this should never happen!!"
    exit 1
fi

if [[ ! -f "${SH_DISTID}" ]]
then
    echo "distid.sh not found - this should never happen!!"
    exit 1
fi

if [[ ! -f "${OPT_BUILDLIST}" ]]
then
    echo "Build list not found or readable: ${OPT_BUILDLIST}"
    exit 1
fi

# Identify our distro
#
dist_id=`${SH_DISTID}`

if [[ $? -gt 0 ]]
then
    echo "Failed to identify distribution."
    exit 1
fi

# Generate build tag
#
cur_arch=`uname -m | xargs echo -n`
cur_branch=`git branch --show-current | xargs echo -n`
cur_hash=`git rev-parse --short HEAD | xargs echo -n`
cur_user=`whoami | xargs echo -n`

tag="${cur_hash}.${cur_arch}.${cur_branch}.${OPT_SKU}(${cur_user},${dist_id})"

echo "Doing full system build for ${tag}"

if [[ "${OPT_OUTPUT_DIR}" == "" ]]
then
    OPT_OUTPUT_DIR="${CURDIR}/xptc/${cur_hash}.${cur_branch}/${cur_arch}/${dist_id}"

    mkdir -p "${OPT_OUTPUT_DIR}"
fi

if [[ ! -d "${OPT_OUTPUT_DIR}" ]]
then
    echo "Cannot ensure output directory "${OPT_OUTPUT_DIR}" exists."
    exit 1
fi

# Check system deps
#
"${SH_CHKDEPS}" -c "${OPT_BUILDLIST}" -l

if [[ $? -gt 0 ]]
then
    echo "Dependencies check unsatisfied or failed."
    exit 1
fi

# Building the whole thing
#
while IFS= read -u "${targets_fd}" -r rel_target_dir
do
    build_component "${dist_id}" "${rel_target_dir}"
done {targets_fd}<"${OPT_BUILDLIST}"

echo "Build complete for ${tag}"
