#!/bin/bash

#
# packicon.sh - Icon Packaging Script (Debian)
#
# This source-code is part of Windows XP stuff for XFCE:
# <<https://www.oddmatics.uk>>
#
# Author(s): Rory Fewell <roryf@oddmaics.uk>
#

#
# CONSTANTS
#
L_SCRIPTDIR=`dirname "$0"`

CURDIR=`realpath -s "./"`
MAPPINGS_FILENAME='mappings'
PKG_DIR=`realpath -s "./tmp.ico-pkg"`
REQUIRED_PACKAGES=(
    'fakeroot'
)
SCRIPTDIR=`realpath -s "${L_SCRIPTDIR}"`

REPO_ROOT=`realpath -s "${SCRIPTDIR}/../../.."`
ICONS_ROOT="${REPO_ROOT}/icons"


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
# MAIN SCRIPT
#

# The arguments passed to this script should specify the icon themes to build, for
# example:
#
#     ./packicon.sh luna professional
#
for themestr in "$@"
do
    theme_dir="${ICONS_ROOT}/${themestr}"
    theme_map_path="${theme_dir}/mappings"
    theme_res="${theme_dir}/res"

    if [[ ! -d "${theme_dir}" ]]
    then
        echo "Can't find icon theme: ${themestr}."
        continue
    fi

    # Set up our package's working directory
    #
    pkg_theme_dir="${PKG_DIR}/usr/share/icons/${themestr}"
    pkg_debian_dir="${PKG_DIR}/DEBIAN"
    pkg_icores_dir="${pkg_theme_dir}/res"

    if [[ -d "${PKG_DIR}" ]]
    then
        rm -rf "${PKG_DIR}"
    fi

    mkdir "${PKG_DIR}"
    mkdir -p "${pkg_icores_dir}"
    mkdir "${pkg_debian_dir}"

    # Copy resources to package
    #
    cp -r "${theme_res}"/* "${pkg_icores_dir}"

    # Create size dirs
    #
    mkdir "${pkg_theme_dir}/16x16"
    mkdir "${pkg_theme_dir}/24x24"
    mkdir "${pkg_theme_dir}/32x32"
    mkdir "${pkg_theme_dir}/48x48"

    # Create symbolic links
    #
    readarray -t mappings < $theme_map_path

    for mapping in ${mappings[*]}
    do
        if [[ $mapping =~ ([a-z]+)/([a-z0-9.+_-]+)--\>([A-Za-z_]+) ]]
        then
            icon_context="${BASH_REMATCH[1]}"
            icon_name="${BASH_REMATCH[2]}"
            target_icon="${BASH_REMATCH[3]}"

            for size in 16x16 24x24 32x32 48x48;
            do
                link_dir="${pkg_theme_dir}/${size}/${icon_context}"
                link_fullpath="${link_dir}/${icon_name}.png"
                link_target="../../res/${size}/${target_icon}.png"

                mkdir -p "${link_dir}"

                ln -s "${link_target}" "${link_fullpath}"
            done
        else
            echo "Invalid mapping '${mapping}' in ${theme_map_path}"
            continue
        fi
    done

    # Copy across index.theme
    #
    cp "${theme_dir}/index.theme" "${pkg_theme_dir}"
    cp "${theme_dir}/debian-control" "${pkg_debian_dir}/control"

    # Create postinst script to do the icon cache
    #
    debian_postinst=`cat <<EOF
#!/bin/bash

gtk-update-icon-cache /usr/share/icons/${themestr}/
EOF
`

    echo "${debian_postinst}" > "${pkg_debian_dir}/postinst"
    chmod +x "${pkg_debian_dir}/postinst"

    # Compile package
    #
    fakeroot dpkg-deb -v --build "${PKG_DIR}" > /dev/null 2>&1

    if [[ $? -gt 0 ]]
    then
        echo "Failed to build package for ${themestr}."
        continue
    fi

    mv "${PKG_DIR}.deb" "icon-theme-${themestr}.deb"

    rm -rf "${PKG_DIR}"

    echo "Built ${themestr}."
done

echo "Packaging complete."
