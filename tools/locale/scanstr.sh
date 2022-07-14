#!/bin/bash

#
# scanstr.sh - Scan for Resource String
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
FILE_PRIORITY=(
    'explorer.exe'
    'shdoclc.dll'  # Good for menus
    'vbscript.dll' # Good for button prompts
    'shell32.dll'
    'comdlg32.dll'
    'wsecedit.dll' # Good for 'Ignore'
    'narrhook.dll' # Good for 'Window'
)
REQUIRED_PACKAGES=(
    'p7zip'
    'ripgrep'
    'python3'
)
SCRIPTDIR=`dirname "$0"`

LOG_PATH="${CURDIR}/scanerr.log"
MUIS_DIR="${SCRIPTDIR}/mui-stuff"
SH_MUI2ISO="${SCRIPTDIR}/mui2iso.sh"
TMP_7Z_DIR="${CURDIR}/tmp.extract"

SOURCE_XP_DIR="${MUIS_DIR}/xp"
TMP_DIALOG_DIR="${TMP_7Z_DIR}/.rsrc/DIALOG"
TMP_MENU_DIR="${TMP_7Z_DIR}/.rsrc/MENU"
TMP_STRING_FILE="${TMP_7Z_DIR}/.rsrc/string.txt"



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

if [[ ! -d "${SOURCE_XP_DIR}" ]]
then
    echo 'Cannot find XP source files - have you ran muiprep.sh yet?' >&2
    exit 1
fi

if [[ ! -f "${SH_MUI2ISO}" ]]
then
    echo "mui2iso script is missing? It should be at ${SH_MUI2ISO}" >&2
    exit 1
fi



#
# ARGUMENTS
#
if [[ $# -eq 0 ]] || [[ "${1}" == "--help" ]]
then
    echo 'scanstr.sh - Scan for Resource String'
    echo ''
    echo 'This script scans the Windows XP files to try and find a string, and get its'
    echo 'ID from resources.'
    echo ''
    echo 'Usage:'
    echo '    scanstr.sh <string>'
    echo ''
    echo 'Example:'
    echo '    scanstr.sh "Cancel"'
    echo ''
    exit 0
fi

if [[ $# -gt 1 ]]
then
    echo 'Too many arguments!' >&2
    echo 'Usage: scanstr.sh <string> OR scanstr.sh for detailed usage.' >&2
    exit 1
fi



#
# FUNCTIONS
#

# cache_file_strings
#
# Retrieves strings from the specified file and appends them to the string cache file
#
cache_file_strings()
{
    local cache_path="${3}"
    local file_kind="${1}"
    local file_path="${2}"
    local strings=`python3 pullstr/pullstr.py "${file_kind}" "${file_path}"`

    if [[ ! -f "${file_path}" ]]
    then
        return 1
    fi

    while IFS= read -r string; do
        echo -e "${string}" | iconv -f UTF-8 -t UTF-16LE >> "${cache_path}"
    done <<< "${strings}"

    return 0
}

# cache_texts_in_dir
#
# Iterates across files in the specified directory and caches strings
#
cache_texts_in_dir()
{
    local cache_path="${3}"
    local dir_path="${2}"
    local file_kind="${1}"

    if [[ ! -d "${dir_path}" ]]
    then
        return 1
    fi

    for file in "${dir_path}"/*; do
        cache_file_strings "${file_kind}" "${file}" "${cache_path}" "${index}"
    done

    return 0
}

# cache_strings
#
# Retrieves the string table for a file and caches it for reading
#
cache_strings()
{
    local file_path="${1}"
    local extras_index=70000 # Start 'extra' strings at 70000, higher than any valid ID
    local cache_path="${file_path}.strings"

    if [[ -f "${cache_path}" ]]
    then
        return 0
    fi

    if [[ -d "${TMP_7Z_DIR}" ]]
    then
        rm -rf "${TMP_7Z_DIR}" >/dev/null 2>>"${LOG_PATH}"
    fi

    # Use 7zip to extract the file - should contain .rsrc/strings.txt if there is a
    # string table within
    #
    7z x "${file_path}" -o"${TMP_7Z_DIR}" >/dev/null 2>>"${LOG_PATH}"

    if [[ $? -gt 0 ]]
    then
        echo "Failed to extract ${file_path} with 7zip." >&2
        return 1
    fi

    if [[ -f "${TMP_STRING_FILE}" ]]
    then
        # Cache the strings file now
        #
        mv "${TMP_STRING_FILE}" "${cache_path}" >/dev/null 2>>"${LOG_PATH}"
    else
        # Create a dummy file so we don't waste time on this file in future
        #
        touch "${cache_path}" >/dev/null 2>>"${LOG_PATH}"
    fi

    if [[ $? -gt 0 ]]
    then
        return 1
    fi

    # Cache strings from DIALOG and MENU resources ('extras' here meaning, not part of
    # the extracted string table)
    #
    cache_texts_in_dir "DIALOG" "${TMP_DIALOG_DIR}" "${cache_path}"
    cache_texts_in_dir "MENU" "${TMP_MENU_DIR}" "${cache_path}"

    rm -rf "${TMP_7Z_DIR}" >/dev/null 2>>"${LOG_PATH}"

    # Clean up results, convert to UTF8 first so sed can work, then back to UTF16LE
    #
    iconv -f UTF-16LE -t UTF-8 "${cache_path}" >"${cache_path}.utf8"

    # Rewrite eg. %1!ls! to %s
    sed -i 's/%[0-9]\(!l\?[cs]!\)\?/%s/gi' "${cache_path}.utf8"

    # Rewrite %s!d! to %d
    sed -i 's/%s!d!/%d/gi' "${cache_path}.utf8"

    iconv -f UTF-8 -t UTF-16LE "${cache_path}.utf8" >"${cache_path}"

    rm -f "${cache_path}.utf8"

    return 0
}



#
# MAIN SCRIPT
#

# The argument passed to this script is the string we want to look for in the XP files
# in order to retrieve the resource ID.
#
#     ./scanstr.sh "Click here to begin"
#
# First thing though we want to adjust the string so it doesn't give us trouble when we
# search for it
#
safe_string="${1}"

safe_string="${safe_string//\\/\\\\}"
safe_string="${safe_string//\?/\\?}"

# Kind of a hack... if this is a 'short' string then add an ampersand into the search
# regex
#
# You may wonder why - the reason is because shorter strings may be commands like
# 'Retry' or 'Cancel', and the stored string on Windows might have an accelerator in
# place
#
# Essentially if the input string is 'Retry', we want to ensure that we capture
# '&Retry' as well
#
# Accelerators are removed in the translations though as a reversal of this process
#
short_string_len=10
string_len="${#1}"

if [[ "${string_len}" -le "${short_string_len}" ]]
then
    safe_string=`echo -n "${safe_string}" | sed 's/./\&?&/g'`
fi

# Search through XP source files that contain the string we're looking for, then build
# an array that prioritises certain files (eg. explorer.exe)
#
declare -a clean_matched_files
declare -a final_files
readarray -t matched_files < <(rg --text --sort path --files-with-matches -E 'utf-16' "${safe_string}" "${SOURCE_XP_DIR}")

# Clean up matched files (remove .strings extensions)
#
for matched_file in "${matched_files[@]}"
do
    stripped_filename=`echo "${matched_file}" | sed 's/\.strings//'`

    clean_matched_files+=("${stripped_filename}")
done

# Prioritise certain files over others
#
for priority_file in "${FILE_PRIORITY[@]}"
do
    # Look for this file in the matched files
    #
    for search_file in "${clean_matched_files[@]}"
    do
        search_file_basename=`basename "${search_file}"`

        if [[ "${search_file_basename}" == "${priority_file}" ]]
        then
            final_files+=("${search_file}")
        fi
    done
done

# Add remaining non-prioritised files, avoiding duplicates
#
for matched_file in "${clean_matched_files[@]}"
do
    # Add any remaining files in here
    #
    for present_file in "${final_files[@]}"
    do
        if [[ "${matched_file}" == "${present_file}" ]]
        then
            continue 2
        fi
    done

    final_files+=("${matched_file}")
done

# Now iterate through the files and look for the string
#
for res_file in "${final_files[@]}"
do
    found_strings=0
    res_cache_file="${res_file}.strings"
    res_file_basename=`basename "${res_file}"`
    res_id=0
    res_mui_basename="${res_file_basename}.mui"

    # Ensure there is a cached copy of the string table
    #
    cache_strings "${res_file}"

    if [[ $? -gt 0 ]]
    then
        continue
    fi

    # Try to retrieve the resource ID for the string
    #
    res_id=`rg --text --no-line-number -E 'utf-16' "^\d+\s+${safe_string}.?$" "${res_cache_file}" | cut -f1 | head -n 1`

    if [[ "${res_id}" -eq 0 ]]
    then
        continue
    fi

    # Now examine the MUIs to find translations for the file
    #
    while IFS= read -r mui_dir
    do
        mui_string=""
        res_mui_file="${mui_dir}/${res_mui_basename}"
        res_mui_cache_file="${res_mui_file}.strings"

        if [[ ! -f "${res_mui_file}" ]]
        then
            continue
        fi

        # Ensure a cached copy of the string table for the MUI
        #
        cache_strings "${res_mui_file}"

        # Try to retrieve the translated string
        #
        mui_string=`rg --text --no-line-number -E 'utf-16' "^${res_id}\s+.+" "${res_mui_cache_file}" | cut -f2`

        if [[ "${mui_string}" != "" ]]
        then
            found_strings=1
        fi

        # Format and output
        #
        mui_code=`basename "${mui_dir}"`
        iso_code=`${SH_MUI2ISO} "${mui_code}"`

        if [[ $? -gt 0 ]]
        then
            echo "Failed to retrieve ISO code for ${mui_code}" >&2
            exit 1
        fi

        mui_string=`echo -n "${mui_string}" | sed 's/[\\r&]//g'`
        mui_string=`echo -n "${mui_string}" | sed 's/\(.\+\)\s$/\1/g'`
        mui_string=`echo -n "${mui_string}" | sed 's/"/\\\"/g'`

        echo "${iso_code}#${mui_string}" # Use a hash as the delimiter
    done < <(find "${MUIS_DIR}" -maxdepth 1 -type d -not -name '.' -a -not -name 'xp')

    # If we found strings successfully - exit
    #
    if [[ "${found_strings}" -eq 1 ]]
    then
        exit 0
    fi
done

# Failed to find the string!
#
echo -n "String not found." >&2
exit 2
