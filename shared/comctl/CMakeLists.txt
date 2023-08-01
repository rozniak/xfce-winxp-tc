cmake_minimum_required(VERSION 3.0)

project(
    libwintc-comctl
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion common controls library."
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
include(../../packaging/cmake-inc/resources/CMakeLists.txt)

wintc_resolve_library(glib-2.0 GLIB)
wintc_resolve_library(gtk+-3.0 GTK3)

wintc_compile_resources()

add_library(
    libwintc-comctl
    src/resources.c
    src/style.c
    src/style.h
)

set_target_properties(
    libwintc-comctl
    PROPERTIES
    PUBLIC_HEADER public/wintc-comctl.h
    SOVERSION 1
    VERSION ${PROJECT_VERSION}
)

target_compile_options(
    libwintc-comctl
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    libwintc-comctl
    SYSTEM
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
)

target_link_directories(
    libwintc-comctl
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
)

target_link_libraries(
    libwintc-comctl
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()
wintc_add_pkgconfig_install()

install(
    TARGETS libwintc-comctl
    LIBRARY       DESTINATION ${LIB_DIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
