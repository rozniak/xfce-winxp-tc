cmake_minimum_required(VERSION 3.3)

set(REPO_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../..)

# Define some constants used by builds
#
set(WINTC_ASSETS_INSTALL_DIR share/wintc)

if (DEFINED WINTC_NO_PEDANTIC_COMPILE)
    set(WINTC_COMPILE_OPTIONS -Wall -Wextra -Werror -Wno-overlength-strings)
else()
    set(WINTC_COMPILE_OPTIONS -Wall -Wextra -Wpedantic -Werror -Wno-overlength-strings)
endif()

# Handle SKU stuff
#
set(
    WINTC_VALID_SKUS
    xpclient-home     # CLIENT
    xpclient-pro
    xpclient-linux
    xpclient-mce
    xpclient-tabletpc
    xpclient-starter
    xpclient-embedded
    xpclient-flp
    xpclient-wepos
    xpclient-wes
    xpclient-posready
    dnsrv-std         # SERVER
    dnsrv-ent
    dnsrv-dtc
    dnsrv-sbs
    dnsrv-web
    dnsrv-ccs
    dnsrv_r2-std      # SERVER R2
    dnsrv_r2-ent
    dnsrv_r2-dtc
    homesrv           # HOME SERVER
)

if (NOT DEFINED WINTC_SKU)
    message(
        FATAL_ERROR
        "No SKU has been specified."
    )
endif()

if (NOT ${WINTC_SKU} IN_LIST WINTC_VALID_SKUS)
    message(
        FATAL_ERROR
        "The SKU ${WINTC_SKU} is not valid."
    )
endif()


# Define func for importing constants from a file
#
function(wintc_source_vars SOURCE_PATH)
    file(STRINGS ${SOURCE_PATH} sourceLines)

    foreach(line ${sourceLines})
        if (${line} STREQUAL "")
            continue()
        endif()

        string(REGEX MATCH "^[^=]+" varIdentifier ${line})
        string(REGEX MATCH "[^=]+$" varValue      ${line})

        set(${varIdentifier} ${varValue} PARENT_SCOPE)
    endforeach()
endfunction()
