#!/usr/bin/env bash

#
# mkrel.sh - Make Release
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

SH_BUILD="${SCRIPTDIR}/build.sh"



#
# MAIN SCRIPT
#

# Check we have mkisofs
#
which mkisofs >/dev/null 2>&1

if [[ $? -gt 0 ]]
then
    echo "mkrel: Missing mkisofs, please install it." >&2
    exit 1
fi

# Look for the latest build
#
latest_build=$(cd "${CURDIR}/xptc" && ls -Ndt -- ./*/ | head -n 1 | cut -d'/' -f2)

if [[ $? -gt 0 || -z "${latest_build}" ]]
then
    echo "mkrel: No builds available." >&2
    exit 1
fi

echo "mkrel: Making image for ${latest_build}" >&2

# Just assume one type of package format/arch is present for now
#
# FIXME: Sort this when there is docker/container support for building for
#        many platforms
#
one_package=$(cd "${CURDIR}/xptc/${latest_build}" && find . -type f | head -n 1)

if [[ $? -gt 0 || -z "${one_package}" ]]
then
    echo "mkrel: No packages available in build." >&2
    exit 1
fi

# Pull the format details
#
pkgfmt=$(echo "${one_package}" | cut -d'/' -f2)
pkgfmtext=$(echo "${one_package}" | cut -d'/' -f3)
pkgarch=$(echo "${one_package}" | cut -d'/' -f4)

# Create the output directory
#
staging_dir="${CURDIR}/relstage"

if [[ -d "${staging_dir}" ]]
then
    rm -rf "${staging_dir}/*"

    if [[ $? -gt 0 ]]
    then
        echo "mkrel: Failed to clear ${staging_dir}" >&2
        exit 1
    fi
else
    mkdir "${staging_dir}"

    if [[ $? -gt 0 ]]
    then
        echo "mkrel: Failed to make staging dir at ${staging_dir}" >&2
        exit 1
    fi
fi

# Copy all packages to target dir
#
src_pkgs_dir=$(dirname "${CURDIR}/xptc/${latest_build}/${one_package}")
staging_pkgs_dir="${staging_dir}/${pkgfmt}/${pkgfmtext}/${pkgarch}"

mkdir -p "${staging_pkgs_dir}"
find ${src_pkgs_dir} -type f -exec cp '{}' "${staging_pkgs_dir}" \;

if [[ $? -gt 0 ]]
then
    echo "mkrel: Failed to copy packages to staging dir." >&2
    exit 1
fi

# Build setup
#
${SH_BUILD} base/setup/initial

(cd ${CURDIR}/build/base/setup/initial; DESTDIR="${staging_dir}" make install)

if [[ $? -gt 0 ]]
then
    echo "mkrel: Failed to build setup." >&2
    exit 1
fi

# Make image
#
mkisofs -o wintc.iso "${staging_dir}"

if [[ $? -gt 0 ]]
then
    echo "mkrel: Failed to create image." >&2
    exit 1
fi

# Cleanup
#
rm -rf "${staging_dir}"

echo "mkrel: Release image created." >&2
