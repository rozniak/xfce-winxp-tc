cmake_minimum_required(VERSION 3.12)

project(
    wintc-build-tag
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion build tag."
    LANGUAGES C
)

set(PROJECT_ANYARCH    true)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)

# Set up variables
#
set(OUT_PATH_RELEASE ${CMAKE_BINARY_DIR}/release)

if (${WINTC_SKU} MATCHES "^xpclient-(.+)")
    set(REL_SKU_NAME "Microsoft Windows XP")
    set(REL_SKU_TAGLINE "Version 2002")

    if (${CMAKE_MATCH_1} STREQUAL "per")
        set(REL_SKU_EDITION "Home Edition")
    elseif (${CMAKE_MATCH_1} STREQUAL "pro")
        if (
            ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "amd64" OR
            ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64"
        )
            set(REL_SKU_EDITION "Professional x64 Edition")
            set(REL_SKU_TAGLINE "Version 2003")
        elseif (
            ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "ia64"    OR
            ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64" OR
            ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armv8"
        )
            set(REL_SKU_EDITION "64-Bit Edition")
        else()
            set(REL_SKU_EDITION "Professional")
        endif()
    elseif (${CMAKE_MATCH_1} STREQUAL "mce")
        set(REL_SKU_EDITION "Media Center Edition 2005")
    elseif (${CMAKE_MATCH_1} STREQUAL "tabletpc")
        set(REL_SKU_EDITION "Tablet PC Edition 2005")
    elseif (${CMAKE_MATCH_1} STREQUAL "embedded")
        set(REL_SKU_EDITION "Embedded")
    endif()
elseif (${WINTC_SKU} MATCHES "^dnsrv(_r2)?-(.+)")
    set(REL_SKU_NAME "Microsoft Windows Server 2003")

    if ("${CMAKE_MATCH_1}" STREQUAL "_r2")
        set(REL_SKU_NAME "Microsoft Windows Server 2003 R2")
    endif()

    if (${CMAKE_MATCH_2} STREQUAL "std")
        set(REL_SKU_EDITION "Standard Edition")
    elseif (${CMAKE_MATCH_2} STREQUAL "ent")
        set(REL_SKU_EDITION "Enterprise Edition")
    elseif (${CMAKE_MATCH_2} STREQUAL "dtc")
        set(REL_SKU_EDITION "Datacenter Edition")
    elseif (${CMAKE_MATCH_2} STREQUAL "app")
        set(REL_SKU_EDITION "Appliance Edition")
    elseif (${CMAKE_MATCH_2} STREQUAL "bla")
        set(REL_SKU_EDITION "Web Edition")
    elseif (${CMAKE_MATCH_2} STREQUAL "sbs")
        set(REL_SKU_EDITION "Small Business Server")
    elseif (${CMAKE_MATCH_2} STREQUAL "ccs")
        set(REL_SKU_EDITION "Compute Cluster Edition")
    elseif (${CMAKE_MATCH_2} STREQUAL "ss")
        set(REL_SKU_EDITION "Storage Server")
    endif()
elseif (${WINTC_SKU} STREQUAL "homesrv")
    set(REL_SKU_NAME "Microsoft Windows Home Server")
endif()

set(REL_PRETTY_NAME "${REL_SKU_NAME} ${REL_SKU_EDITION}")

# Installation
#
wintc_configure_and_install_packaging()

configure_file(release.in ${OUT_PATH_RELEASE} @ONLY)

install(FILES ${OUT_PATH_RELEASE} DESTINATION ${WINTC_ASSETS_INSTALL_DIR})
