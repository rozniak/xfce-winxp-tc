cmake_minimum_required(VERSION 3.14)

project(
    libwintc-setupapi
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion setup functions library."
    LANGUAGES C
)

set(PROJECT_ANYARCH false)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/libraries/CMakeLists.txt)
include(../../packaging/cmake-inc/linking/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)

wintc_resolve_library(glib-2.0     GLIB)
wintc_resolve_library(gtk+-3.0     GTK3)
wintc_resolve_library(wintc-comgtk WINTC_COMGTK)
wintc_resolve_library(wintc-exec   WINTC_EXEC)

add_library(
    libwintc-setupapi
    src/initsys.c
    public/initsys.h
)

set_target_properties(
    libwintc-setupapi
    PROPERTIES
    SOVERSION 1
    VERSION ${PROJECT_VERSION}
)

target_compile_options(
    libwintc-setupapi
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    libwintc-setupapi
    SYSTEM
    BEFORE
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
    PRIVATE ${WINTC_EXEC_INCLUDE_DIRS}
)

target_link_directories(
    libwintc-setupapi
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
    PRIVATE ${WINTC_EXEC_LIBRARY_DIRS}
)

target_link_libraries(
    libwintc-setupapi
    PRIVATE ${GLIB_LIBRARY_LIBRARIES}
    PRIVATE ${GTK3_LIBRARY_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
    PRIVATE ${WINTC_EXEC_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()
wintc_add_pkgconfig_install()
wintc_install_public_headers()

install(
    TARGETS libwintc-setupapi
    LIBRARY DESTINATION ${LIB_DIR}
)
