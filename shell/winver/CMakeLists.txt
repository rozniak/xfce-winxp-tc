cmake_minimum_required(VERSION 3.0)

project(
    wintc-shell-winver
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion 'About Windows' utility."
)

set(PROJECT_ANYARCH    false)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(
    PROJECT_DEPENDENCIES
    libglib2.0-0
    libgtk-3-0
    libwintc-comgtk
)

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GLIB         REQUIRED glib-2.0)
pkg_check_modules(GTK3         REQUIRED gtk+-3.0)
pkg_check_modules(WINTC_COMGTK REQUIRED wintc-comgtk)

add_executable(
    wintc-shell-winver
    src/winver.c
)

set_target_properties(
    wintc-shell-winver
    PROPERTIES
    OUTPUT_NAME winver
)

target_compile_options(
    wintc-shell-winver
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    wintc-shell-winver
    SYSTEM
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
)

target_link_directories(
    wintc-shell-winver
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
)

target_link_libraries(
    wintc-shell-winver
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
)

# Installation
#
wintc_configure_and_install_packaging()

install(
    DIRECTORY res/
    DESTINATION ${WINTC_ASSETS_INSTALL_DIR}/shell-res
)
install(
    TARGETS wintc-shell-winver
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
