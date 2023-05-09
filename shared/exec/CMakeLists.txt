cmake_minimum_required(VERSION 3.0)

project(
    libwintc-exec
    VERSION 1.0
    DESCRIPTION "Windows Total Conversion launcher library."
)

set(PROJECT_ANYARCH    false)
set(PROJECT_FREESTATUS true)
set(PROJECT_MAINTAINER "Rory Fewell <roryf@oddmatics.uk>")

set(
    PROJECT_DEPENDENCIES
    libglib2.0-dev
    libgtk-3-dev
    libwintc-comgtk
)

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)

include(../../packaging/cmake-inc/common/CMakeLists.txt)
include(../../packaging/cmake-inc/libraries/CMakeLists.txt)
include(../../packaging/cmake-inc/packaging/CMakeLists.txt)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GDK  REQUIRED gdk-3.0)
pkg_check_modules(GIO  REQUIRED gio-2.0)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

add_library(
    libwintc-exec
    src/action.h
    src/action.c
    src/desktop.h
    src/desktop.c
    src/exec.h
    src/exec.c
    src/mime.h
    src/mime.c
)

set_target_properties(
    libwintc-exec
    PROPERTIES
    PUBLIC_HEADER public/wintc-exec.h
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
    PRIVATE ${GDK_INCLUDE_DIRS}
    PRIVATE ${GIO_INCLUDE_DIRS}
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK3_INCLUDE_DIRS}
)

target_link_directories(
    libwintc-exec
    PRIVATE ${GDK_LIBRARY_DIRS}
    PRIVATE ${GIO_LIBRARY_DIRS}
    PRIVATE ${GLIB_LIBRARY_DIRS}
    PRIVATE ${GTK3_LIBRARY_DIRS}
)

target_link_libraries(
    libwintc-exec
    PRIVATE ${GDK_LIBRARIES}
    PRIVATE ${GIO_LIBRARIES}
    PRIVATE ${GLIB_LIBRARIES}
    PRIVATE ${GTK3_LIBRARIES}
)

# Installation
#
wintc_configure_and_install_packaging()
wintc_add_pkgconfig_install()

install(
    TARGETS libwintc-exec
    LIBRARY       DESTINATION ${LIB_DIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
