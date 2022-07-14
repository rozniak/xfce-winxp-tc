#!/bin/bash

#
# mui2iso.sh - Get ISO 639-1 Code from MUI Language Code
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
    [ARA]='ar_SA' # Arabic                - Saudi Arabia
    [BG]='bg_BG'  # Bulgarian             - Bulgaria
    [BR]='pt_BR'  # Portuguese            - Brazil
    [CHH]='zh_TW' # Chinese (Traditional) - Taiwan
    [CHS]='zh_CN' # Chinese (Simplified)  - China
    [CS]='cs_CZ'  # Czech                 - Czech Republic
    [DA]='da_DK'  # Danish                - Denmark
    [EL]='el_GR'  # Greek                 - Greece
    [ES]='es_ES'  # Spanish               - Spain
    [ET]='et_EE'  # Estonian              - Estonia
    [FI]='fi_FI'  # Finnish               - Finland
    [FR]='fr_FR'  # French                - France
    [GER]='de_DE' # German                - Germany
    [HEB]='he_IL' # Hebrew                - Israel
    [HR]='hr_HR'  # Croatian              - Croatia
    [HU]='hu_HU'  # Hungarian             - Hungary
    [IT]='it_IT'  # Italian               - Italy
    [JPN]='ja_JP' # Japanese              - Japan
    [KOR]='ko_KR' # Korean                - Korea
    [LT]='lt_LT'  # Lithuanian            - Lithuania
    [LV]='lv_LV'  # Latvian               - Latvia
    [NL]='nl_NL'  # Dutch                 - Netherlands
    [NO]='nb_NO'  # Norwegian             - Norway
    [PL]='pl_PL'  # Polish                - Poland
    [PT]='pt_PT'  # Portuguese            - Portugal
    [RO]='ro_RO'  # Romanian              - Romania
    [RU]='ru_RU'  # Russian               - Russia
    [SK]='sk_SK'  # Slovak                - Slovakia
    [SL]='sl_SI'  # Solvenian             - Slovenia
    [SV]='sv_SE'  # Swedish               - Sweden
    [TH]='th_TH'  # Thai                  - Thailand
    [TR]='tr_TR'  # Turkish               - Turkey
)



#
# ARGUMENTS
#
if [[ $# -eq 0 ]] || [[ "${1}" == "--help" ]]
then
    echo 'mui2iso.sh - Converts MUI langauge code to ISO 639-1'
    echo ''
    echo 'This script is a helper for retrieving the ISO 639-1 language code (used by'
    echo 'gettext) associated with a MUI code (used by Windows).'
    echo ''
    echo 'Note that this is for MUI codes such as FR or JPN, not the numerical codes'
    echo 'like 040e. This is because this the directory names in the sources use the'
    echo 'former rather than the latter.'
    echo ''
    echo 'Usage:'
    echo '    mui2iso.sh <mui code>'
    echo ''
    echo 'Example:'
    echo '    mui2iso.sh SK'
    echo ''
    exit 0
fi

if [[ $# -gt 1 ]]
then
    echo 'Too many arguments!'
    echo 'Usage: mui2iso.sh <mui code> OR mui2iso.sh for detailed usage'
    exit 1
fi



#
# MAIN SCRIPT
#

# The argument passed to this script is a Windows MUI code (not the numeric kind), we
# just translate it to an ISO 639-1 code in the standard output.
#
mui_code="${1}"

if [[ "${MAPPINGS[$mui_code]+_}" ]]
then
    echo -n "${MAPPINGS[$mui_code]}"
    exit 0
fi

echo -n 'Unknown MUI code.'
exit 1
