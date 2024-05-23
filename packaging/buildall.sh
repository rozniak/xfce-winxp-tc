#!/usr/bin/env bash

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
CURDIR=`realpath "./"`
SCRIPTDIR=`dirname "$0"`

REPO_ROOT=`realpath "${SCRIPTDIR}/.."`
TARGETS_PATH="${SCRIPTDIR}/targets"
BLDUTILS_ROOT="${REPO_ROOT}/tools/bldutils"

SH_BUILD="${SCRIPTDIR}/build.sh"
SH_CHKDEPS="${SCRIPTDIR}/chkdeps.sh"
SH_DISTID="${SCRIPTDIR}/distid.sh"
SH_GENTAG="${BLDUTILS_ROOT}/gentag/gentag.sh"
SH_PACKAGE="${SCRIPTDIR}/package.sh"



#
# ARGUMENTS
#
OPT_BUILDLIST="${TARGETS_PATH}"
OPT_CHECKED=0
OPT_OUTPUT_DIR=""
OPT_SKU="xpclient-pro"
OPT_SKIP_PACKAGING=0

while getopts "c:dho:s:z" opt;
do
    case "${opt}" in
        c)
            OPT_BUILDLIST="${OPTARG}"
            ;;

        d)
            OPT_CHECKED=1
            ;;

        h)
            echo "Usage: buildall.sh [-chosz]"
            echo " -c : provide a list of components (default 'targets')"
            echo " -d : produce checked build"
            echo " -h : display this help screen"
            echo " -o : specify output directory for packages"
            echo " -s : specify SKU to build (default xpclient-pro)"
            echo " -z : skip packaging steps, compile only"
            echo ""

            exit 0
            ;;

        o)
            OPT_OUTPUT_DIR="${OPTARG}"
            ;;

        s)
            OPT_SKU="${OPTARG}"
            ;;

        z)
            OPT_SKIP_PACKAGING=1
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
    local rel_dir="${1}"
    local target_dir="${REPO_ROOT}/${rel_dir}"
    local deps_path="${target_dir}/deps"

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
                    build_component "shared/${lib_shortname}"
                    g_built_libs+=("${lib_shortname}")
                fi
            fi
        done {deps_fd}<"${deps_path}"
    fi

    # Now build the component
    #
    if [[ $OPT_CHECKED -eq 1 ]]
    then
        "${SH_BUILD}" -d -l -s "${OPT_SKU}" "${rel_dir}"
    else
        "${SH_BUILD}" -l -s "${OPT_SKU}" "${rel_dir}"
    fi

    if [[ $? -gt 0 ]]
    then
        echo "buildall: Compile failure, bailing."
        exit 1
    fi

    # Package the component
    #
    if [[ $OPT_SKIP_PACKAGING -eq 1 ]]
    then
        return
    fi

    "${SH_PACKAGE}" -o "${OPT_OUTPUT_DIR}" "${rel_dir}"

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

check_present()
{
    local check_path="${1}"

    if [[ ! -f "${check_path}" ]]
    then
        echo "${check_path} not found - this should never happen!!"
        exit 1
    fi
}


#
# MAIN SCRIPT
#
check_present "${SH_BUILD}"
check_present "${SH_CHKDEPS}"
check_present "${SH_DISTID}"
check_present "${SH_GENTAG}"
check_present "${SH_PACKAGE}"

if [[ ! -f "${OPT_BUILDLIST}" ]]
then
    echo "Build list not found or readable: ${OPT_BUILDLIST}"
    exit 1
fi

# Identify our distro
#
. "${SH_DISTID}"

if [[ $? -gt 0 ]]
then
    echo "Failed to identify distribution."
    exit 1
fi

# Generate build tag
#
build_subdir="fre"
build_type="free"
cur_arch=`uname -m | xargs echo -n`
tag=`${SH_GENTAG}`

if [[ $OPT_CHECKED -eq 1 ]]
then
    build_subdir="chk"
    build_type="checked"
fi

echo "Doing full system build for ${tag} (${cur_arch}, ${DIST_ID}-${DIST_ID_EXT}) (${build_type})"

# Handle output dir for packaging
#
if [[ $OPT_SKIP_PACKAGING -eq 0 ]]
then
    if [[ "${OPT_OUTPUT_DIR}" == "" ]]
    then
        OPT_OUTPUT_DIR="${CURDIR}/xptc/${tag}/${DIST_ID}/${DIST_ID_EXT}/${cur_arch}/${build_subdir}"

        mkdir -p "${OPT_OUTPUT_DIR}"
    fi

    if [[ ! -d "${OPT_OUTPUT_DIR}" ]]
    then
        echo "Cannot ensure output directory "${OPT_OUTPUT_DIR}" exists."
        exit 1
    fi
else
    echo "Packaging will be skipped for this session."
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
    build_component "${rel_target_dir}"
done {targets_fd}<"${OPT_BUILDLIST}"

echo "Build complete for ${tag} (${build_type})"
