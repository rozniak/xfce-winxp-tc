#!/bin/bash

#
# packplug.sh - XFCE Panel Plugin Packaging Script (Debian)
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
PKG_DIR=`realpath -s "./tmp.plug-pkg"`
REQUIRED_PACKAGES=(
    'cmake'
    'fakeroot'
    'make'
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
    echo 'Usage: packplug.sh [<name>]...'
    exit 1
fi



#
# MAIN SCRIPT
#

# The arguments passed to this script should specify the panel plugins to build, for
# example:
#
#     ./packplug.sh shell/start shell/systray
#
for plugstr in "$@"
do
    plug_dir="${REPO_ROOT}/${plugstr}"
    plug_safename=${plugstr/\//-}
    log_path="${CURDIR}/${plug_safename}-build.log"

    if [[ ! -d "${plug_dir}" ]]
    then
        echo "Can't find plugin: ${plugstr}."
        continue
    fi

    if [[ -f "${log_path}" ]]
    then
        rm -f "${log_path}"

        if [[ $? -gt 0 ]]
        then
            echo "Failed to delete ${log_path}, skipping."
            continue
        fi
    fi

    # Set up build directory
    #
    build_dir="${plug_dir}/build"

    cd "${plug_dir}"

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
        echo "Compilation failed for ${plugstr}, see ${log_path} for output."
        continue
    fi

    # Set up our package's working directory
    #
    pkg_debian_dir="${PKG_DIR}/DEBIAN"
    pkg_desktop_dir="${PKG_DIR}/usr/share/xfce4/panel/plugins"
    pkg_locale_dir="${PKG_DIR}/usr/share/locale"
    pkg_plug_dir="${PKG_DIR}/usr/lib/x86_64-linux-gnu/xfce4/panel/plugins"
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
    mkdir -p "${pkg_desktop_dir}"
    mkdir -p "${pkg_locale_dir}"
    mkdir -p "${pkg_plug_dir}"

    # Copy files
    #
    pkg_setup_result=0

    cp "${plug_dir}/debian-control" "${pkg_debian_dir}/control" >> "${log_path}" 2>&1
    ((pkg_setup_result+=$?))

    cp "${plug_dir}/"*.desktop "${pkg_desktop_dir}" >> "${log_path}" 2>&1
    ((pkg_setup_result+=$?))

    cp "${build_dir}/"*.so "${pkg_plug_dir}" >> "${log_path}" 2>&1
    ((pkg_setup_result+=$?))

    if [[ -d "${plug_dir}/res" ]]
    then
        mkdir -p ${pkg_res_dir}

        cp "${plug_dir}/res/"* "${pkg_res_dir}" >> "${log_path}" 2>&1
        ((pkg_setup_result+=$?))
    fi

    # Include translations if present
    #
    if [[ -d "${plug_dir}/po" ]]
    then
        for pofile in "${plug_dir}/po"/*
        do
            if [[ "${pofile}" =~ /([a-z]{2}(_[A-Z]{2})?)\.po$ ]]
            then
                po_language="${BASH_REMATCH[1]}"
                locale_dir="${pkg_locale_dir}/${po_language}/LC_MESSAGES"

                mkdir -p "${locale_dir}"

                msgfmt -o "${locale_dir}/wintc-${plug_safename}.mo" "${pofile}" >> "${log_path}" 2>&1
                ((pkg_setup_result+=$?))
            fi
        done
    fi

    # Check package setup good
    #
    if [[ $pkg_setup_result -gt 0 ]]
    then
        echo "Failed to copy files for ${plugstr}, see ${log_path} for output."
        rm -rf "${PKG_DIR}"
        continue
    fi

    # Compile package
    #
    fakeroot dpkg-deb -v --build "${PKG_DIR}" >> "${log_path}" 2>&1

    if [[ $? -gt 0 ]]
    then
        echo "Failed to build package for ${plugdir}, see ${log_path} for output."
        rm -rf "${PKG_DIR}"
        continue
    fi

    # Tidy
    #
    mv "${PKG_DIR}.deb" "${plug_safename}-plugin.deb"

    rm -rf "${PKG_DIR}"

    echo "Built ${plugstr}."
done

echo "Packaging complete."
