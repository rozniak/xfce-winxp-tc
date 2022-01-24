#!/bin/bash

#
# packsnds.sh - Sound Theme Packaging Script (Debian)
#
# This source-code is part of Windows XP stuff for XFCE:
# <<https://www.oddmatics.uk>>
#

#
# CONSTANTS
#
CURDIR=`realpath -s "./"`
MAPPINGS_FILENAME="mappings"
PKG_DIR=`realpath -s "./tmp.cur-pkg"`
PKG_NAME="wintc-sound-theme-xp"
REQUIRED_PACKAGES=(
    'fakeroot'
)
SCRIPTDIR=`dirname "$0"`
THEME_NAME="Windows XP Default"

REPO_ROOT=`realpath -s "${SCRIPTDIR}/../../.."`
SOUNDS_ROOT="${REPO_ROOT}/sounds"



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
if [[ $# -gt 0 ]]
then
    echo 'Usage: packsnds.sh'
    exit 1
fi



#
# MAIN SCRIPT
#

# This script will build the sound theme in /sounds
#
log_path="${CURDIR}/${PKG_NAME}.log"
pkg_setup_result=0

pkg_theme_dir="${PKG_DIR}/usr/share/sounds/${THEME_NAME}"
pkg_stereo_dir="${pkg_theme_dir}/stereo"
pkg_res_dir="${pkg_stereo_dir}/res"
pkg_debian_dir="${PKG_DIR}/DEBIAN"

if [[ -d "${PKG_DIR}" ]]
then
    rm -rf "${PKG_DIR}"
fi

if [[ -f "${log_path}" ]]
then
    rm -rf "${log_path}"

    if [[ $? -gt 0 ]]
    then
        echo "Failed to delete ${log_path}, exiting."
        exit 1
    fi
fi

mkdir -p "${PKG_DIR}"
mkdir -p "${pkg_theme_dir}"
mkdir -p "${pkg_stereo_dir}"
mkdir -p "${pkg_res_dir}"
mkdir -p "${pkg_debian_dir}"

cp "${SOUNDS_ROOT}/sfx"/* "${pkg_res_dir}" >> "${log_path}" 2>&1
((pkg_setup_result+=$?))

cp "${SOUNDS_ROOT}/index.theme" "${pkg_theme_dir}" >> "${log_path}" 2>&1
((pkg_setup_result+=$?))

cp "${SOUNDS_ROOT}/debian-control" "${pkg_debian_dir}/control" >> "${log_path}" 2>&1
((pkg_setup_result+=$?))

# Check initial copying was good
#
if [[ $pkg_setup_result -gt 0 ]]
then
    echo "Failed to copy files for ${PKG_NAME}, see ${log_path} for output."
    rm -rf "${PKG_DIR}"
    exit 1
fi

# Create symbolic links
#
readarray -t mappings < "${SOUNDS_ROOT}/mappings"

cd "${pkg_stereo_dir}"

for mapping in ${mappings[*]}
do
    if [[ "${mapping}" =~ ([A-Za-z0-9_-]+)--\>([A-Za-z0-9_-]+) ]]
    then
        keyname="${BASH_REMATCH[1]}"
        resname="${BASH_REMATCH[2]}"

        pkg_res_target="res/${resname}.wav"
        pkg_snd_source="${pkg_stereo_dir}/${keyname}.wav"

        ln -s "${pkg_res_target}" "${pkg_snd_source}" >> "${log_path}" 2>&1
        ((pkg_setup_result+=$?))

        if [[ $? -gt 0 ]]
        then
            "Failed to create symlink for ${keyname}." >> ${log_path} 2>&1
            continue
        fi
    else
        echo "Invalid mapping '${mapping}'." >> ${log_path} 2>&1
        continue
    fi
done

# Check symlinks were okay
#
if [[ $pkg_setup_result -gt 0 ]]
then
    echo "WARNING: Some symlinks failed, check ${log_path} for output."
fi

# Compile package
#
cd "${CURDIR}"

fakeroot dpkg-deb -v --build "${PKG_DIR}" >> "${log_path}" 2>&1

if [[ $? -gt 0 ]]
then
    "Failed to build package, see ${log_path} for output."
    exit 1
fi

# Tidy
#
mv "${PKG_DIR}.deb" "${PKG_NAME}.deb"

rm -rf "${PKG_DIR}"

echo "Built ${PKG_NAME}."
