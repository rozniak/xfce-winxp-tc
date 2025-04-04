cmake_minimum_required(VERSION 3.12)

project(
    libwintc-syscfg
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion system configuration library."
    LANGUAGES C
)

set(PROJECT_ANYARCH    false)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/libraries/CMakeLists.txt)
include(../../packaging/cmake-inc/linking/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)

wintc_resolve_library(glib-2.0 GLIB)

add_library(
    libwintc-syscfg
    public/enums.h
    src/keys.c
    public/keys.h
)

set_target_properties(
    libwintc-syscfg
    PROPERTIES
    SOVERSION 1
    VERSION ${PROJECT_VERSION}
)

target_compile_options(
    libwintc-syscfg
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    libwintc-syscfg
    SYSTEM
    BEFORE
    PRIVATE ${GLIB_INCLUDE_DIRS}
)

target_link_directories(
    libwintc-syscfg
    PRIVATE ${GLIB_LIBRARY_DIRS}
)

target_link_libraries(
    libwintc-syscfg
    PRIVATE ${GLIB_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()
wintc_add_pkgconfig_install()
wintc_install_public_headers()

install(
    TARGETS libwintc-syscfg
    LIBRARY DESTINATION ${LIB_DIR}
)
