#!/bin/bash

#
# transpot.sh - Translate POT File
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
REQUIRED_PACKAGES=(
    'gettext'
)
SCRIPTDIR=`dirname "$0"`

LOG_PATH="${CURDIR}/transerr.log"
MUIS_DIR="${SCRIPTDIR}/mui-stuff"
SH_MUI2ISO="${SCRIPTDIR}/mui2iso.sh"
SH_SCANSTR="${SCRIPTDIR}/scanstr.sh"



#
# PRE-FLIGHT CHECKS
#
for package in "${REQUIRED_PACKAGES[@]}"
do
    dpkg -s $package > /dev/null 2>&1

    if [[ $? -gt 0 ]]
    then
        echo "You are missing package: ${package}" >&2
        exit 1
    fi
done

if [[ ! -d "${MUIS_DIR}" ]]
then
    echo 'Cannot find MUI sources - have you ran muiprep.sh yet?' >&2
    exit 1
fi

if [[ ! -f "${SH_MUI2ISO}" ]]
then
    echo "mui2iso script is missing? It should be at ${SH_MUI2ISO}" >&2
    exit 1
fi

if [[ ! -f "${SH_SCANSTR}" ]]
then
    echo "scanstr script is missing? It should be at ${SH_SCANSTR}" >&2
    exit 1
fi



#
# ARGUMENTS
#
if [[ $# -eq 0 ]] || [[ "${1}" == "--help" ]]
then
    echo 'transpot.sh - Transalte POT File'
    echo ''
    echo 'This script attempts to provide translations for a POT file using the Windows'
    echo 'XP MUI sources. Translations will automatically be outputted as .po files in'
    echo 'the same directory as the POT file.'
    echo ''
    echo 'Usage:'
    echo '    transpot.sh <pot file>'
    echo ''
    echo 'Example:'
    echo '    transpot.sh locale.pot'
    echo ''
    exit 0
fi

if [[ $# -gt 1 ]]
then
    echo 'Too many arguments!' >&2
    echo 'Usage: transpot.sh <pot file> OR transpot.sh for detailed usage.' >&2
    exit 1
fi



#
# MAIN SCRIPT
#

# The argument passed to this script is the path to the .pot file to provide
# translations for - we will translate into each MUI present, as individual .po files
# in the .pot file's directory
#
potfile_path="${1}"
potfile_dir=`dirname "${potfile_path}"`

if [[ ! -f "${potfile_path}" ]]
then
    "File not found: ${potfile_path}" >&2
    exit 1
fi

# Get a mapping of each installed MUI to ISO code
#
declare -A mui_codes

while IFS= read -r mui_dir
do
    mui_code=`basename "${mui_dir}"`
    iso_code=`${SH_MUI2ISO} "${mui_code}"`

    if [[ $? -gt 0 ]]
    then
        echo "Failed to retrieve ISO code for ${mui_code}" >&2
        exit 1
    fi

    mui_codes[${mui_code}]="${iso_code}"
done < <(find "${MUIS_DIR}"/* -maxdepth 0 -type d -not -name '.' -a -not -name 'xp')

# Generate the language .po files
#
for mui_code in "${!mui_codes[@]}"
do
    iso_code="${mui_codes[${mui_code}]}"
    po_path="${potfile_dir}/${iso_code}.po"

    msginit -i "${potfile_path}" \
            -l "${iso_code}" \
            --no-translator \
            -o "${po_path}" \
        >/dev/null 2>"${LOG_PATH}"

    if [[ $? -gt 0 ]]
    then
        echo "Failed to generate ${po_path}, see ${LOG_PATH} for output."
        exit 1
    fi
done

# Iterate through the .pot file - look for msgids to translate and merge them into each
# translation one by one
#
parsed_msgid=''
parsing=0

while IFS= read -u "${pot_fd}" -r pot_line
do
    if [[ "${parsing}" -eq 0 ]]
    then
        # We're looking for msgid lines to begin parsing and build the string
        #
        if [[ ! "${pot_line}" =~ ^msgid ]]
        then
            continue
        fi

        parsed_msgid=`echo -n "${pot_line}" | cut -d' ' -f2- | sed 's/"\(.*\)"/\1/'`
        parsing=1
    else
        # If we hit a msgstr line, we've finished parsing the msgid and should
        # translate it now
        #
        if [[ "${pot_line}" =~ ^msgstr ]]
        then
            # Only translate if it's an actual msgid - the blank one is just for the
            # metadata
            #
            if [[ "${parsed_msgid}" != '' ]]
            then
                echo "Attempting to translate \"${parsed_msgid}\"..."
                translations=`${SH_SCANSTR} "${parsed_msgid}" 2>>"${LOG_PATH}"`

                case $? in
                    0)
                        echo "Found:"
                        while IFS= read -r trans_combo
                        do
                            echo "${trans_combo}"
                            iso_code=`echo "${trans_combo}" | cut -d'#' -f1`
                            translation=`echo "${trans_combo}" | cut -d'#' -f2-`

                            po_path="${potfile_dir}/${iso_code}.po"
                            tmp_path="${po_path}.merge"

                            cat << EOF > "${tmp_path}"
msgid ""
msgstr "Content-Type: text/plain; charset=UTF-8\n"

msgid "${parsed_msgid}"
msgstr "${translation}"
EOF

                            msgcat --output-file="${po_path}" \
                                   --use-first \
                                   "${po_path}" \
                                   "${tmp_path}" >/dev/null 2>"${LOG_PATH}"

                            if [[ $? -gt 0 ]]
                            then
                                echo "Failed to provide translations, see ${LOG_PATH} for output."
                                exit 1
                            fi

                            rm -f "${tmp_path}"
                        done <<< "${translations}"
                        echo "Next..."
                        ;;

                    2)
                        echo "No translations found for '${parsed_msgid}'"
                        ;;

                    *)
                        echo "Failed to translate phrases, see ${LOG_PATH} for output."
                        exit 1
                        ;;
                esac
            fi

            parsed_msgid=''
            parsing=0

            continue
        fi

        # Continue building the string
        #
        parsed_msgid+=`echo -n "${pot_line}" | sed 's/"\(.*\)"/\1/'`
    fi
done {pot_fd}<"${potfile_path}"
