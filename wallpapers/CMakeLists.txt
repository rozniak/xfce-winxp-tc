cmake_minimum_required(VERSION 3.12)

project(
    wintc-wallpapers
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion wallpapers."
    LANGUAGES C
)

set(PROJECT_ANYARCH    true)
set(PROJECT_FREESTATUS false)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../packaging/cmake-inc/common/CMakeLists.txt)
include(../packaging/cmake-inc/packaging/CMakeLists.txt)

set(WINTC_WALLPAPER_DIR share/backgrounds/wintc)

# Installation
#
wintc_configure_and_install_packaging()

install(DIRECTORY ${PROJECT_ROOT}/common/ DESTINATION ${WINTC_WALLPAPER_DIR})
install(DIRECTORY ${PROJECT_ROOT}/tile/   DESTINATION ${WINTC_WALLPAPER_DIR})

if (${WINTC_SKU} STREQUAL "xpclient-per")
    install(
        FILES ${PROJECT_ROOT}/per/brand.jpg
        DESTINATION ${WINTC_WALLPAPER_DIR}
        RENAME winxp.jpg
    )
elseif (${WINTC_SKU} STREQUAL "xpclient-mce")
    install(
        FILES ${PROJECT_ROOT}/mce/brand.jpg
        DESTINATION ${WINTC_WALLPAPER_DIR}
        RENAME winxp.jpg
    )
elseif (${WINTC_SKU} STREQUAL "xpclient-pro")
    if (
        ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "amd64" OR
        ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64"
    )
        install(
            FILES ${PROJECT_ROOT}/pro/brandamd.jpg
            DESTINATION ${WINTC_WALLPAPER_DIR}
            RENAME winxp.jpg
        )
    elseif(
        ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "ia64"    OR
        ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64" OR
        ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armv8"
    )
        install(
            FILES ${PROJECT_ROOT}/pro/brand64.jpg
            DESTINATION ${WINTC_WALLPAPER_DIR}
            RENAME winxp.jpg
        )
    else()
        install(
            FILES ${PROJECT_ROOT}/pro/brand.jpg
            DESTINATION ${WINTC_WALLPAPER_DIR}
            RENAME winxp.jpg
        )
    endif()
elseif (${WINTC_SKU} MATCHES "homesrv")
    install(
        FILES ${PROJECT_ROOT}/qhs/brand.png
        DESTINATION ${WINTC_WALLPAPER_DIR}
        RENAME winxp.png
    )
elseif (${WINTC_SKU} MATCHES "^xpclient")
    install(
        FILES ${PROJECT_ROOT}/pro/brand.jpg
        DESTINATION ${WINTC_WALLPAPER_DIR}
        RENAME winxp.jpg
    )
elseif (${WINTC_SKU} MATCHES "^dnsrv_r2")
    install(
        FILES ${PROJECT_ROOT}/dnsrv_r2/brand.jpg
        DESTINATION ${WINTC_WALLPAPER_DIR}
        RENAME winxp.jpg
    )
elseif (${WINTC_SKU} MATCHES "^dnsrv")
    install(
        FILES ${PROJECT_ROOT}/dnsrv/brand.jpg
        DESTINATION ${WINTC_WALLPAPER_DIR}
        RENAME winxp.jpg
    )
endif()
