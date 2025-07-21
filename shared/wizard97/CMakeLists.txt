cmake_minimum_required(VERSION 3.12)

project(
    libwintc-wizard97
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion wizard UI library."
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

wintc_compile_resources()

add_library(
    libwintc-wizard97
    src/resources.c
    src/wizpage.c
    public/wizpage.h
    src/wizwnd.c
    public/wizwnd.h
)

set_target_properties(
    libwintc-wizard97
    PROPERTIES
    SOVERSION 1
    VERSION ${PROJECT_VERSION}
)

target_compile_options(
    libwintc-wizard97
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    libwintc-wizard97
    SYSTEM
    BEFORE
    PRIVATE ${GDK_PIXBUF_INCLUDE_DIRS}
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
)

target_link_directories(
    libwintc-wizard97
    PRIVATE ${GDK_PIXBUF_LIBRARY_DIRS}
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
)

target_link_libraries(
    libwintc-wizard97
    PRIVATE ${GDK_PIXBUF_LIBRARIES}
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()
wintc_add_pkgconfig_install()
wintc_install_public_headers()

install(
    TARGETS libwintc-wizard97
    LIBRARY DESTINATION ${LIB_DIR}
)
