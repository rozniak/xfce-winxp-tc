#!/bin/bash

#
# muiprep.sh - MUI Working Directory Preparation Script
#
# This source-code is part of the Windows XP stuff for XFCE:
# <<https://www.oddmatics.uk>>
#
# Author(s): Rory Fewell <roryf@oddmatics.uk>
#

#
# CONSTANTS
#
CURDIR=`realpath -s "./"`
REQUIRED_PACKAGES=(
    'cabextract'
)
SCRIPTDIR=`dirname "$0"`

LOG_PATH="${CURDIR}/preperr.log"
MUI_DIR="${SCRIPTDIR}/mui-stuff"
SH_LCID2MUI="${SCRIPTDIR}/lcid2mui.sh"
TMP_SP_DIR="${CURDIR}/spmv.tmp"

declare -A SPDLL_NAMES=(
    [SPRA]='xpsp1res.dll.mui'
    [SPRB]='xpsp2res.dll.mui'
    [SPRC]='xpsp3res.dll.mui'
    [OBRB]='xpob2res.dll.mui'
)



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
if [[ $# -eq 0 ]] || [[ "${1}" == "--help" ]]
then
    echo 'muiprep.sh - MUI working dir prep script'
    echo ''
    echo 'This script sets up a working dir for grabbing translated strings from the'
    echo 'MUI sources.'
    echo ''
    echo 'The expectation is that you have extracted the discs somewhere, this script'
    echo 'looks for I386 directories located in the <path> argument. The contents are'
    echo 'determined to be a language MUI or the XP install files, and expanded into'
    echo 'a working directory.'
    echo ''
    echo 'The working directory is then used by scanstr.sh for resolving the'
    echo 'translations.'
    echo ''
    echo 'Basically it is worth getting dirs set up as follows:'
    echo '/files'
    echo '/files/MuiDisc1 (disc 1 contents)'
    echo '/files/MuiDisc2 (disc 2 contents)'
    echo '/files/MuiDisc3 (disc 3 contents)'
    echo '/files/MuiDisc4 (disc 4 contents)'
    echo '/files/MuiDisc5 (disc 5 contents)'
    echo '/files/XPDisc   (XP disc contents)'
    echo ''
    echo 'Does not have to look exactly like this, because it will look for I386 dirs'
    echo 'but hopefully you get the idea.'
    echo ''
    echo 'Hopefully.'
    echo ''
    echo 'Usage:'
    echo '    muiprep.sh <path>'
    echo ''
    echo 'Example:'
    echo '    muiprep.sh ~/muiDiscs'
    echo ''
    exit 0
fi

if [[ $# -gt 1 ]]
then
    echo 'Too many arguments!'
    echo 'Usage: muiprep.sh <path> OR muiprep.sh for detailed usage.'
    exit 1
fi



#
# MAIN SCRIPT
#

# The argument passed to this script is a path where extracted MUI and XP setup discs
# are stored:
#
#     ./muiprep.sh ~/discs
#
# We'll search for I386 dirs in ~/discs for MUI and XP files to expand to our working
# dir.
#
mui_search_base="${1}"

mkdir -p "${MUI_DIR}" 2>>"${LOG_PATH}"

if [[ $? -gt 0 ]]
then
    echo "Failed to create ${MUI_DIR}, see ${LOG_PATH} for output."
    exit 1
fi

find "${mui_search_base}" -type d -wholename "*/I386" | while read i386_dir; do
    if compgen -G "${i386_dir}/*.DLL.MU_" > /dev/null
    then
        # This a MUI dir
        #
        if [[ "${i386_dir}" =~ ([A-Z]+)\.MUI ]]
        then
            mui_name="${BASH_REMATCH[1]}"
        fi

        mui_expand_dir="${MUI_DIR}/${mui_name}"

        echo "MUI '${mui_name}' files detected in ${i386_dir}"

        mkdir -p "${mui_expand_dir}"

        find "${i386_dir}" -type f       \
                           -name "*.MU_" \
                           -exec sh -c "cabextract -d${mui_expand_dir} {} >/dev/null 2>>${LOG_PATH}" \;

        if [[ $? -gt 0 ]]
        then
            echo "Failed to expand files, see ${LOG_PATH} for output."
            exit 1
        fi

        echo "MUI '${mui_name}' files prepared."
    else
        # This is the Windows XP disc
        #
        exts=('*.CP_' '*.DL_' '*.EX_' '*.SY_')
        result=0
        xp_expand_dir="${MUI_DIR}/xp"

        echo "Windows XP files detected in ${i386_dir}"

        # Check that this is an SP3 disc...
        #
        if ! compgen -G "${i386_dir}/SPRC*.DL_" > /dev/null
        then
            echo "This does not appear to be Service Pack 3! Aborting."
            exit 1
        fi

        # All good, continue
        #
        mkdir -p "${xp_expand_dir}"

        # Handle the MUI service pack message DLLs
        #
        mkdir -p "${TMP_SP_DIR}"

        mv "${i386_dir}/"SPR*.DL_ "${TMP_SP_DIR}" >/dev/null 2>>"${LOG_PATH}"
        mv "${i386_dir}/"OBRB* "${TMP_SP_DIR}" >/dev/null 2>>"${LOG_PATH}"

        for spres_file in "${TMP_SP_DIR}/"*; do
            sp_basename=`basename "${spres_file}"`
            sp_kind=`echo "${sp_basename}" | head -c 4`
            sp_lcid=`echo "${sp_basename}" | head -c 8 | tail -c 4`

            target_filename="${SPDLL_NAMES[$sp_kind]}"
            target_mui=`${SH_LCID2MUI} "${sp_lcid}"`

            target_dir="${MUI_DIR}/${target_mui}"

            mkdir -p "${target_dir}"

            cabextract -p "${spres_file}" > "${target_dir}/${target_filename}" 2>>"${LOG_PATH}"

            if [[ $? -gt 0 ]]
            then
                echo "Failed to expand files, see ${LOG_PATH} for output."
                exit 1
            fi
        done

        # Handle the actual disc contents we're interested in
        #
        for ext in "${exts[@]}"
        do
            find "${i386_dir}" -type f        \
                               -name "${ext}" \
                               -exec sh -c "cabextract -d${xp_expand_dir} {} >/dev/null 2>>${LOG_PATH}" \;

            if [[ $? -gt 0 ]]
            then
                echo "Failed to expand files, see ${LOG_PATH} for output."
                exit 1
            fi
        done

        # Tidy up
        #
        mv "${TMP_SP_DIR}/"* "${i386_dir}" >/dev/null 2>>"${LOG_PATH}"
        rmdir "${TMP_SP_DIR}"

        echo 'Windows XP files prepared.'
    fi
done
