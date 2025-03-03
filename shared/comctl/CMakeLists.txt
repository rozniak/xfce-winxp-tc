cmake_minimum_required(VERSION 3.12)

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

wintc_resolve_library(gdk-pixbuf-2.0 GDK_PIXBUF)
wintc_resolve_library(glib-2.0       GLIB)
wintc_resolve_library(gtk+-3.0       GTK3)
wintc_resolve_library(wintc-comgtk   WINTC_COMGTK)
wintc_resolve_library(wintc-shcommon WINTC_SHCOMMON)
wintc_resolve_library(wintc-shlang   WINTC_SHLANG)

wintc_compile_resources()

add_library(
    libwintc-comctl
    src/resources.c
    src/animctl.c
    public/animctl.h
    src/cpl.c
    public/cpl.h
    src/menubind.c
    public/menubind.h
    src/style.c
    public/style.h
)

set_target_properties(
    libwintc-comctl
    PROPERTIES
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
    BEFORE
    PRIVATE ${GDK_PIXBUF_INCLUDE_DIRS}
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHCOMMON_INCLUDE_DIRS}
    PRIVATE ${WINTC_SHLANG_INCLUDE_DIRS}
)

target_link_directories(
    libwintc-comctl
    PRIVATE ${GDK_PIXBUF_LIBRARY_DIRS}
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHCOMMON_LIBRARY_DIRS}
    PRIVATE ${WINTC_SHLANG_LIBRARY_DIRS}
)

target_link_libraries(
    libwintc-comctl
    PRIVATE ${GDK_PIXBUF_LIBRARIES}
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WINTC_SHCOMMON_LIBRARIES}
    PRIVATE ${WINTC_SHLANG_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()
wintc_add_pkgconfig_install()
wintc_install_public_headers()

install(
    TARGETS libwintc-comctl
    LIBRARY DESTINATION ${LIB_DIR}
)
