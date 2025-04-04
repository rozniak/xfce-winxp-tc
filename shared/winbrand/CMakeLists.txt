cmake_minimum_required(VERSION 3.12)

project(
    libwintc-winbrand
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion Windows branding library."
    LANGUAGES C
)

set(PROJECT_ANYARCH    false)
set(PROJECT_FREESTATUS false)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/libraries/CMakeLists.txt)
include(../../packaging/cmake-inc/linking/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)

set(WINTC_BRAND_DIR ${WINTC_ASSETS_INSTALL_DIR}/brand)

wintc_resolve_library(gdk-pixbuf-2.0 GDK_PIXBUF)
wintc_resolve_library(glib-2.0       GLIB)
wintc_resolve_library(gtk+-3.0       GTK3)
wintc_resolve_library(wintc-comgtk   WINTC_COMGTK)

add_library(
    libwintc-winbrand
    public/brand.h
    src/brand.c
)

set_target_properties(
    libwintc-winbrand
    PROPERTIES
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
    BEFORE
    PRIVATE ${GDK_PIXBUF_INCLUDE_DIRS}
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
)

target_link_directories(
    libwintc-winbrand
    PRIVATE ${GDK_PIXBUF_LIBRARY_DIRS}
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
)

target_link_libraries(
    libwintc-winbrand
    PRIVATE ${GDK_PIXBUF_LIBRARIES}
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
)

# Pick banner based on SKU
#
set(WINBRAND_BANNER_NAME ${WINTC_SKU})

if (${WINTC_SKU} STREQUAL "xpclient-pro")
    if (
        ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "amd64" OR
        ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64"
    )
        set(WINBRAND_BANNER_NAME ${WINTC_SKU}_amd64)
    elseif (
        ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "ia64"    OR
        ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64" OR
        ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armv8"
    )
        set(WINBRAND_BANNER_NAME ${WINTC_SKU}_ia64)
    endif()
elseif (${WINTC_SKU} MATCHES "^dnsrv(_r2)?-(.+)")
    if (${CMAKE_MATCH_2} MATCHES "(dtc|ent|std)")
        set(WINBRAND_BANNER_NAME ${WINTC_SKU}_amd64)
    endif()
endif()



# Installation
#
wintc_configure_and_install_packaging()
wintc_add_pkgconfig_install()
wintc_install_public_headers()

install(
    FILES res/banner/${WINBRAND_BANNER_NAME}.png
    DESTINATION ${WINTC_BRAND_DIR}
    RENAME banner.png
)
install(
    FILES res/bannerx/${WINBRAND_BANNER_NAME}.png
    DESTINATION ${WINTC_BRAND_DIR}
    RENAME bannerx.png
)
install(
    FILES res/strip-static.png
    DESTINATION ${WINTC_BRAND_DIR}
)
install(
    FILES res/strip-anim.png
    DESTINATION ${WINTC_BRAND_DIR}
)
install(
    TARGETS libwintc-winbrand
    LIBRARY DESTINATION ${LIB_DIR}
)
