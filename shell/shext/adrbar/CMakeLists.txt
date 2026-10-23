cmake_minimum_required(VERSION 3.14)

project(
    libwintc-toolbar-address
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion Address Bar shell toolbar."
    LANGUAGES C
)

set(PROJECT_ANYARCH    false)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../../packaging/cmake-inc/libraries/CMakeLists.txt)
include(../../../packaging/cmake-inc/linking/CMakeLists.txt)
include(../../../packaging/cmake-inc/packaging/CMakeLists.txt)
include(../../../packaging/cmake-inc/resources/CMakeLists.txt)

wintc_resolve_library(glib-2.0       GLIB           PRIVATE)
wintc_resolve_library(gtk+-3.0       GTK3           PRIVATE)
wintc_resolve_library(wintc-comgtk   WINTC_COMGTK   PRIVATE)
wintc_resolve_library(wintc-shellext WINTC_SHELLEXT PRIVATE)
wintc_resolve_library(wintc-shlang   WINTC_SHLANG   PRIVATE)

wintc_compile_resources()

add_library(
    libwintc-toolbar-address
    src/adrbar.c
    src/adrbar.h
    src/resources.c
    src/toolbar.c
)

target_compile_options(
    libwintc-toolbar-address
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    libwintc-toolbar-address
    SYSTEM
    BEFORE
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHELLEXT_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHLANG_INCLUDE_DIRS}
)

target_link_directories(
    libwintc-toolbar-address
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHELLEXT_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHLANG_LIBRARY_DIRS}
)

target_link_libraries(
    libwintc-toolbar-address
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WINTC_SHELLEXT_LIBRARIES}
    PRIVATE ${WINTC_SHLANG_LIBRARIES}
)

#
# Installation
#
wintc_configure_and_install_packaging()

install(
    FILES adrbar.desktop
    DESTINATION share/wintc/toolbar
)
install(
    TARGETS libwintc-toolbar-address
    DESTINATION ${LIB_DIR}/wintc/toolbar
)

