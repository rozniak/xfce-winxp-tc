cmake_minimum_required(VERSION 3.0)

project(
    libwintc-winbrand
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion Windows branding library."
)

set(PROJECT_ANYARCH    false)
set(PROJECT_FREESTATUS false)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(
    PROJECT_DEPENDENCIES
    libgdk-pixbuf-2.0-0
    libglib2.0-0
)

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/libraries/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GDK_PIXBUF REQUIRED gdk-pixbuf-2.0)
pkg_check_modules(GLIB       REQUIRED glib-2.0)

configure_file(src/brand.c.in ${PROJECT_ROOT}/src/brand.c @ONLY)

add_library(
    libwintc-winbrand
    src/brand.h
    src/brand.c
)

set_target_properties(
    libwintc-winbrand
    PROPERTIES
    PUBLIC_HEADER public/wintc-winbrand.h
    SOVERSION 1
    VERSION ${PROJECT_VERSION}
)

target_compile_options(
    libwintc-winbrand
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    libwintc-winbrand
    SYSTEM
    PRIVATE ${GDK_PIXBUF_INCLUDE_DIRS}
    PRIVATE ${GLIB_INCLUDE_DIRS}
)

target_link_directories(
    libwintc-winbrand
    PRIVATE ${GDK_PIXBUF_LIBRARY_DIRS}
    PRIVATE ${GLIB_LIBRARY_DIRS}
)

target_link_libraries(
    libwintc-winbrand
    PRIVATE ${GDK_PIXBUF_LIBRARIES}
    PRIVATE ${GLIB_LIBRARIES}
)

# Pick banner based on SKU
#
if (${WINTC_SKU} MATCHES "^xpclient-(.+)")
    if (${CMAKE_MATCH_1} STREQUAL "pro")
        if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
            set(WINBRAND_BANNER_NAME x64pro)
        elseif (
            ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "ia64"  OR
            ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armv8"
        )
            set(WINBRAND_BANNER_NAME 64bit)
        else()
            set(WINBRAND_BANNER_NAME pro)
        endif()
    else()
        set(WINBRAND_BANNER_NAME ${CMAKE_MATCH_1})
    endif()
elseif (${WINTC_SKU} MATCHES "^dnsrv-(.+)")
    if (
        ${CMAKE_MATCH_1} STREQUAL "sbs" OR
        ${CMAKE_MATCH_1} STREQUAL "web" OR
        ${CMAKE_MATCH_1} STREQUAL "ccs" OR
        NOT ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64"
    )
        set(WINBRAND_BANNER_NAME srv2k3${CMAKE_MATCH_1})
    else()
        set(WINBRAND_BANNER_NAME x64srv2k3${CMAKE_MATCH_1})
    endif()
elseif (${WINTC_SKU} MATCHES "^dnsrv_r2-(.+)")
    if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
        set(WINBRAND_BANNER_NAME x64srv2k3r2${CMAKE_MATCH_1})
    else()
        set(WINBRAND_BANNER_NAME srv2k3r2${CMAKE_MATCH_1})
    endif()
elseif (${WINTC_SKU} STREQUAL "homesrv")
    set(WINBRAND_BANNER_NAME homesrv)
else()
    message(
        FATAL_ERROR
        "No banner defined for SKU: ${WINTC_SKU}"
    )
endif()

# Installation
#
wintc_configure_and_install_packaging()
wintc_add_pkgconfig_install()

install(
    FILES res/banner-${WINBRAND_BANNER_NAME}.png
    DESTINATION ${WINTC_ASSETS_INSTALL_DIR}/brand
    RENAME banner.png
)
install(
    FILES res/strip-xp.png
    DESTINATION ${WINTC_ASSETS_INSTALL_DIR}/brand
    RENAME strip.png
)
install(
    TARGETS libwintc-winbrand
    LIBRARY       DESTINATION ${LIB_DIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
