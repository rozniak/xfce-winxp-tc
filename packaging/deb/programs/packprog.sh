#!/bin/bash

#
# packprog.sh - Program Packaging Script (Debian)
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
PKG_DIR=`realpath -s "./tmp.prog-pkg"`
REQUIRED_PACKAGES=(
    'cmake'
    'fakeroot'
    'make'
    'gettext'
)
SCRIPTDIR=`dirname "$0"`

REPO_ROOT=`realpath -s "${SCRIPTDIR}/../../.."`



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
    echo 'Usage: packprog.sh [<name>]...'
    exit 1
fi



#
# MAIN SCRIPT
#

# The arguments passed to this script should specify the program to build, for
# example:
#
#     ./packprog.sh shell/run windows/paint
#
for progstr in "$@"
do
    prog_dir="${REPO_ROOT}/${progstr}"
    prog_name=""
    prog_safename=${progstr/\//-}
    prog_section=""
    log_path="${CURDIR}/${prog_safename}-build.log"

    if [[ ! -d "${prog_dir}" ]]
    then
        echo "Can't find program: ${progstr}."
        continue
    fi

    if [[ -f "${log_path}" ]]
    then
        rm -rf "${log_path}"

        if [[ $? -gt 0 ]]
        then
            echo "Failed to delete ${log_path}, skipping."
            continue
        fi
    fi

    if [[ "$progstr" =~ ([A-Za-z0-9._-]+)/([A-Za-z0-9._-]+) ]]
    then
        prog_section="${BASH_REMATCH[1]}"
        prog_name="${BASH_REMATCH[2]}"
    else
        echo "Unrecognised program: ${progstr}"
        continue
    fi

    # Set up build directory
    #
    build_dir="${prog_dir}/build"

    cd "${prog_dir}"

    if [[ -d "${build_dir}" ]]
    then
        rm -rf "${build_dir}"

        if [[ $? -gt 0 ]]
        then
            echo "Failed to delete ${build_dir}, skipping."
            continue
        fi
    fi

    mkdir "${build_dir}"
    cd "${build_dir}"

    # Compile the source
    #
    cmake .. >> "${log_path}" 2>&1
    make -j${CORE_COUNT} >> "${log_path}" 2>&1

    if [[ $? -gt 0 ]]
    then
        echo "Compilation failed for ${progstr}, see ${log_path} for output."
        continue
    fi

    # Set up our packages working directory
    #
    pkg_debian_dir="${PKG_DIR}/DEBIAN"
    pkg_desktop_dir="${PKG_DIR}/usr/share/applications"
    pkg_locale_dir="${PKG_DIR}/usr/share/locale"
    pkg_prog_dir="${PKG_DIR}/usr/bin"
    pkg_res_dir="${PKG_DIR}/usr/share/winxp/shell-res"

    if [[ -d "${PKG_DIR}" ]]
    then
        rm -rf "${PKG_DIR}"

        if [[ $? -gt 0 ]]
        then
            echo "Failed to clear temp dir: ${PKG_DIR}, see ${log_path} for output."
            continue
        fi
    fi

    cd "${CURDIR}"

    mkdir -p "${PKG_DIR}"
    mkdir -p "${pkg_debian_dir}"
    mkdir -p "${pkg_prog_dir}"
    mkdir -p "${pkg_locale_dir}"

    # Copy files
    #
    pkg_setup_result=0

    cp "${prog_dir}/debian-control" "${pkg_debian_dir}/control" >> "${log_path}" 2>&1
    ((pkg_setup_result+=$?))

    cp "${build_dir}/${prog_name}" "${pkg_prog_dir}" >> "${log_path}" 2>&1
    ((pkg_setup_result+=$?))

    # Include desktop entry if present
    #
    if [[ -f "${prog_dir}/${prog_name}.desktop" ]]
    then
        mkdir -p "${pkg_desktop_dir}"

        cp "${prog_dir}/${prog_name}.desktop" "${pkg_desktop_dir}" >> "${log_path}" 2>&1
        ((pkg_setup_result+=$?))
    fi

    # Include translations if present
    #
    if [[ -d "${prog_dir}/po" ]]
    then
        for pofile in "${prog_dir}/po"/*
        do
            if [[ "${pofile}" =~ /([a-z]{2}(_[A-Z]{2})?)\.po$ ]]
            then
                po_language="${BASH_REMATCH[1]}"
                locale_dir="${pkg_locale_dir}/${po_language}/LC_MESSAGES"

                mkdir -p "${locale_dir}"

                msgfmt -o "${locale_dir}/wintc-${prog_name}.mo" "${pofile}" >> "${log_path}" 2>&1
                ((pkg_setup_result+=$?))
            fi
        done
    fi

    # Include resources if present
    #
    if [[ -d "${prog_dir}/res" ]]
    then
        mkdir -p "${pkg_res_dir}"

        cp "${prog_dir}/res/"* "${pkg_res_dir}" >> "${log_path}" 2>&1
        ((pkg_setup_result+=$?))
    fi

    # Check package setup was good
    #
    if [[ $pkg_setup_result -gt 0 ]]
    then
        echo "Failed to copy files for ${progstr}, see ${log_path} for output."
        rm -rf "${PKG_DIR}"
        continue
    fi

    # Compile package
    #
    fakeroot dpkg-deb -v --build "${PKG_DIR}" >> "${log_path}" 2>&1

    if [[ $? -gt 0 ]]
    then
        echo "Failed to build package for ${progstr}, see ${log_path} for output."
        rm -rf "${PKG_DIR}"
        continue
    fi

    # Tidy
    #
    mv "${PKG_DIR}.deb" "${prog_safename}.deb"

    rm -rf "${PKG_DIR}"

    echo "Built ${progstr}"
done
