cmake_minimum_required(VERSION 3.0)

project(
    libwintc-shllang
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion shell language string utilities."
)

set(PROJECT_ANYARCH    false)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(
    PROJECT_DEPENDENCIES
    libglib2.0-dev
)

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/libraries/CMakeLists.txt)
include(../../packaging/cmake-inc/locale/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GLIB REQUIRED glib-2.0)

wintc_create_config_h()

add_library(
    libwintc-shllang
    src/config.h
    src/controls.c
    src/controls.h
    src/places.c
    src/places.h
    src/punctuation.h
)

set_target_properties(
    libwintc-shllang
    PROPERTIES
    PUBLIC_HEADER public/wintc-shllang.h
    SOVERSION 1
    VERSION ${PROJECT_VERSION}
)

target_compile_options(
    libwintc-shllang
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    libwintc-shllang
    SYSTEM
    PRIVATE ${GLIB_INCLUDE_DIRS}
)

target_link_directories(
    libwintc-shllang
    PRIVATE ${GLIB_LIBRARY_DIRS}
)

target_link_libraries(
    libwintc-shllang
    PRIVATE ${GLIB_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()
wintc_add_pkgconfig_install()
wintc_compile_and_install_pofiles()

install(
    TARGETS libwintc-shllang
    LIBRARY DESTINATION       ${LIB_DIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
