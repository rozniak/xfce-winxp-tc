cmake_minimum_required(VERSION 3.12)

project(
    wintc-setup-gui
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion graphical phase setup."
    LANGUAGES C
)

set(PROJECT_ANYARCH false)
set(PROJECT_FREESTATUS false)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../../packaging/cmake-inc/linking/CMakeLists.txt)
include(../../../packaging/cmake-inc/packaging/CMakeLists.txt)
include(../../../packaging/cmake-inc/resources/CMakeLists.txt)

wintc_resolve_library(glib-2.0       GLIB)
wintc_resolve_library(gtk+-3.0       GTK3)
wintc_resolve_library(wintc-comgtk   WINTC_COMGTK)
wintc_resolve_library(wintc-shelldpa WINTC_SHELLDPA)
wintc_resolve_library(wintc-shlang   WINTC_SHLANG)

wintc_compile_resources()

add_executable(
    wintc-setup-gui
    src/main.c
    src/resources.c
    src/setupwnd.c
    src/setupwnd.h
)

set_target_properties(
    wintc-setup-gui
    PROPERTIES
    OUTPUT_NAME wsetupx
)

target_compile_options(
    wintc-setup-gui
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    wintc-setup-gui
    SYSTEM
    BEFORE
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHELLDPA_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHLANG_INCLUDE_DIRS}
)

target_link_directories(
    wintc-setup-gui
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHELLDPA_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHLANG_LIBRARY_DIRS}
)

target_link_libraries(
    wintc-setup-gui
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WINTC_SHELLDPA_LIBRARIES}
    PRIVATE ${WINTC_SHLANG_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()

install(
    FILES res/wsetupx.service
    DESTINATION ${WINTC_ASSETS_INSTALL_DIR}/setup
)
install(
    FILES src/xinitrc
    DESTINATION ${WINTC_ASSETS_INSTALL_DIR}/setup
)
install(
    TARGETS wintc-setup-gui
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
