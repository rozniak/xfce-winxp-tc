#!/bin/bash

#
# lcid2mui.sh - Get MUI Language Code from Locale ID
#
# This source-code is part of Windows XP stuff for XFCE:
# <<https://www.oddmatics.uk>>
#
# Author(s): Rory Fewell <roryf@oddmatics.uk>
#

#
# CONSTANTS
#
declare -A MAPPINGS=(
    [0401]='ARA'
    [0402]='BG'
    [0404]='CHH'
    [0405]='CS'
    [0406]='DA'
    [0407]='GER'
    [0408]='EL'
    [040B]='FI'
    [040C]='FR'
    [040D]='HEB'
    [040E]='HU'
    [0410]='IT'
    [0411]='JPN'
    [0412]='KOR'
    [0413]='NL'
    [0414]='NO'
    [0415]='PL'
    [0416]='BR'
    [0418]='RO'
    [0419]='RU'
    [041A]='HR'
    [041B]='SK'
    [041D]='SV'
    [041E]='TH'
    [041F]='TR'
    [0424]='SL'
    [0425]='ET'
    [0426]='LV'
    [0427]='LT'
    [0804]='CHS'
    [0816]='PT'
    [0C0A]='ES'
)



#
# ARGUMENTS
#
if [[ $# -eq 0 ]] || [[ "${1}" == '--help' ]]
then
    echo 'lcid2mui.sh - Converts Locale ID to MUI language code'
    echo ''
    echo 'This script is a helper for retrieving the MUI code associated with an'
    echo 'LCID (Windows things...).'
    echo ''
    echo 'This applies to the hexadecimal format LCIDs (eg. 0401) used in filenames.'
    echo ''
    echo 'Usage:'
    echo '    lcid2mui.sh <lcid>'
    echo ''
    echo 'Example:'
    echo '    lcid2mui.sh 0C0A'
    echo ''
    exit 0
fi

if [[ $# -gt 1 ]]
then
    echo 'Too many arguments!'
    echo 'Usage: lcid2mui.sh <lcid> OR lcid2mui.sh for detailed usage'
    exit 1
fi



#
# MAIN SCRIPT
#

# The argument passed to this script is a Windows Locale ID in hexadecimal (eg. 0401),
# we just translate it to a MUI code (eg. 'ARA') in the standard output
#
lcid="${1}"

if [[ "${MAPPINGS[$lcid]+_}" ]]
then
    echo -n "${MAPPINGS[$lcid]}"
    exit 0
fi

echo -n 'Unknown LCID.'
exit 1
