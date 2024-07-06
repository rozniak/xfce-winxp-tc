cmake_minimum_required(VERSION 3.5)

project(
    wintc-setup-oobe
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion out-of-box experience."
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

wintc_resolve_library(glib-2.0       GLIB)
wintc_resolve_library(gtk+-3.0       GTK3)
wintc_resolve_library(wintc-comgtk   WINTC_COMGTK)
wintc_resolve_library(wintc-shelldpa WINTC_SHELLDPA)

add_executable(
    wintc-setup-oobe
    src/main.c
    src/window.c
    src/window.h
)

set_target_properties(
    wintc-setup-oobe
    PROPERTIES
    OUTPUT_NAME msoobe
)

target_compile_options(
    wintc-setup-oobe
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    wintc-setup-oobe
    SYSTEM
    BEFORE
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHELLDPA_INCLUDE_DIRS}
)

target_link_directories(
    wintc-setup-oobe
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHELLDPA_LIBRARY_DIRS}
)

target_link_libraries(
    wintc-setup-oobe
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WINTC_SHELLDPA_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()

install(
    TARGETS wintc-setup-oobe
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
