cmake_minimum_required(VERSION 3.12)

set(REPO_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../..)

# Define some constants used by builds
#
set(WINTC_ASSETS_INSTALL_DIR share/wintc)

if (DEFINED WINTC_NO_PEDANTIC_COMPILE)
    set(WINTC_COMPILE_OPTIONS -Wall -Wextra -Werror -Wno-overlength-strings)
else()
    set(WINTC_COMPILE_OPTIONS -Wall -Wextra -Wpedantic -Werror -Wno-overlength-strings)
endif()

# Build options from user
#
option(
    WINTC_USE_LOCAL_LIBS
    "Use compiled libraries instead of system installed for libwintc* libs."
)

# Define pre-processor macro for checked builds
#
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if (${CMAKE_BUILD_TYPE} STREQUAL Debug)
    add_compile_definitions(WINTC_CHECKED)
endif()

# Enable ASAN for checked builds
#   - Do not enable for musl targets or FreeBSD
#
if (
    NOT ${WINTC_PKGMGR}     STREQUAL apk  AND
    NOT ${WINTC_PKGMGR_EXT} STREQUAL musl AND
    NOT ${WINTC_PKGMGR}     STREQUAL bsdpkg
)
    if (${CMAKE_BUILD_TYPE} STREQUAL Debug)
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    endif()
endif()

# Handle SKU stuff
#
set(
    WINTC_VALID_SKUS
    xpclient-per      # CLIENT
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
    dnsrv-app
    dnsrv-bla
    dnsrv-sbs
    dnsrv-ccs
    dnsrv_r2-std      # SERVER R2
    dnsrv_r2-ent
    dnsrv_r2-dtc
    dnsrv_r2-ss
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

# Define func for importing constants from text like
#
# VAR1=some value
# VAR2=36
# ...
#
function(wintc_source_vars SOURCE_LINES)
    foreach(line ${SOURCE_LINES})
        if (${line} STREQUAL "")
            continue()
        endif()

        string(REGEX MATCH "^[^=]+" varIdentifier ${line})
        string(REGEX MATCH "[^=]+$" varValue      ${line})

        set(${varIdentifier} ${varValue} PARENT_SCOPE)
    endforeach()
endfunction()

# Import version information
#
if (DEFINED ENV{WINTC_VER_TAG})
    set(WINTC_VER_MAJOR $ENV{WINTC_VER_MAJOR})
    set(WINTC_VER_MINOR $ENV{WINTC_VER_MINOR})
    set(WINTC_VER_BUILD $ENV{WINTC_VER_BUILD})
    set(WINTC_VER_DATETIME $ENV{WINTC_VER_DATETIME})
    set(WINTC_VER_BRANCH $ENV{WINTC_VER_BRANCH})
    set(WINTC_VER_DATESTAMP $ENV{WINTC_VER_DATESTAMP})
    set(WINTC_VER_HASH $ENV{WINTC_VER_HASH})
    set(WINTC_VER_USER $ENV{WINTC_VER_USER})
    set(WINTC_VER_TAG $ENV{WINTC_VER_TAG})
else()
    execute_process(
        COMMAND bash ${REPO_ROOT}/tools/bldutils/gentag/gentag.sh
        WORKING_DIRECTORY ${REPO_ROOT}
        OUTPUT_VARIABLE version_lines
    )

    string(REPLACE "\n" ";" version_lines ${version_lines})

    wintc_source_vars("${version_lines}")
endif()

set(WINTC_VER_PROJECT "${WINTC_VER_MAJOR}.${WINTC_VER_MINOR}.${WINTC_VER_BUILD}.${WINTC_VER_DATETIME}")
