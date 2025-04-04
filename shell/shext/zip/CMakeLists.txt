cmake_minimum_required(VERSION 3.12)

project(
    libwintc-shext-zip
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion ZIP file shell extension."
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

wintc_resolve_library(glib-2.0       GLIB)
wintc_resolve_library(gtk+-3.0       GTK3)
wintc_resolve_library(wintc-comgtk   WINTC_COMGTK)
wintc_resolve_library(wintc-shellext WINTC_SHELLEXT)
wintc_resolve_library(libzip         ZIP)

add_library(
    libwintc-shext-zip
    src/shext.c
    src/vwzip.c
    src/vwzip.h
)

target_compile_options(
    libwintc-shext-zip
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    libwintc-shext-zip
    SYSTEM
    BEFORE
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHELLEXT_INCLUDE_DIRS}
    PRIVATE ${ZIP_INCLUDE_DIRS}
)

target_link_directories(
    libwintc-shext-zip
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHELLEXT_LIBRARY_DIRS}
    PRIVATE ${ZIP_LIBRARY_DIRS}
)

target_link_libraries(
    libwintc-shext-zip
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WINTC_SHELLEXT_LIBRARIES}
    PRIVATE ${ZIP_LIBRARIES}
)

#
# Installation
#
wintc_configure_and_install_packaging()

install(
    TARGETS libwintc-shext-zip
    DESTINATION ${LIB_DIR}/wintc/shext
)
