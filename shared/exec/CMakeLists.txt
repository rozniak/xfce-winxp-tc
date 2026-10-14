cmake_minimum_required(VERSION 3.14)

project(
    libwintc-exec
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion launcher library."
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

wintc_resolve_library(gio-2.0      GIO          PUBLIC)
wintc_resolve_library(glib-2.0     GLIB         PUBLIC)
wintc_resolve_library(gtk+-3.0     GTK3         PUBLIC)
wintc_resolve_library(wintc-comgtk WINTC_COMGTK PRIVATE)

add_library(
    libwintc-exec
    public/action.h
    src/action.c
    public/desktop.h
    src/desktop.c
    public/errors.h
    src/errors.c
    public/exec.h
    src/exec.c
    public/mime.h
    src/mime.c
)

set_target_properties(
    libwintc-exec
    PROPERTIES
    SOVERSION 1
    VERSION ${PROJECT_VERSION}
)

target_compile_options(
    libwintc-exec
    PRIVATE ${WINTC_COMPILE_OPTIONS}
)

target_include_directories(
    libwintc-exec
    SYSTEM
    BEFORE
    PUBLIC  ${GIO_INCLUDE_DIRS}
    PUBLIC  ${GLIB_INCLUDE_DIRS}
    PUBLIC  ${GTK3_INCLUDE_DIRS}
    PRIVATE ${WINTC_COMGTK_INCLUDE_DIRS}
)

target_link_directories(
    libwintc-exec
    PUBLIC  ${GIO_LIBRARY_DIRS}
    PUBLIC  ${GLIB_LIBRARY_DIRS}
    PUBLIC  ${GTK3_LIBRARY_DIRS}
    PRIVATE ${WINTC_COMGTK_LIBRARY_DIRS}
)

target_link_libraries(
    libwintc-exec
    PUBLIC  ${GIO_LIBRARIES}
    PUBLIC  ${GLIB_LIBRARIES}
    PUBLIC  ${GTK3_LIBRARIES}
    PRIVATE ${WINTC_COMGTK_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()
wintc_add_pkgconfig_install()
wintc_install_public_headers()

install(
    TARGETS libwintc-exec
    LIBRARY DESTINATION ${LIB_DIR}
)
